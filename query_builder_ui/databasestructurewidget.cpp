#include "databasestructurewidget.h"
#include "ui_databasestructurewidget.h"
#include <QToolButton>


using namespace arcirk::query_builder_ui;
using namespace arcirk::tree_model;

DatabaseStructureWidget::DatabaseStructureWidget(WebSocketClient* client, QWidget *parent, const QUuid& packade_uuid) :
    QWidget(parent),
    ui(new Ui::DatabaseStructureWidget)
{
    ui->setupUi(this);

    treeView = new TreeViewWidget(this, "DatabaseStructureWidget");
    ui->horizontalLayoutTree->addWidget(treeView);
    m_client = client;
    m_packade_uuid = packade_uuid;

    load_structure();
    connect(treeView, &TreeViewWidget::doDropEventJson, this,  &DatabaseStructureWidget::onDropEvent);
    connect(ui->btnType, &QToolButton::toggled, this, &DatabaseStructureWidget::onBtnTypeToggled);
    connect(ui->btnSetItem, &QToolButton::clicked, this, &DatabaseStructureWidget::onBtnSetItemClicked);
    connect(ui->btnSetItems, &QToolButton::clicked, this, &DatabaseStructureWidget::onBtnSetItemsClicked);
    connect(ui->btnRemoveItem, &QToolButton::clicked, this, &DatabaseStructureWidget::onBtnRemoveItemClicked);
    connect(ui->btnRemoveItems, &QToolButton::clicked, this, &DatabaseStructureWidget::onBtnRemoveItemsClicked);

    setProperty("typeName", "DatabaseStructureWidget");
}

DatabaseStructureWidget::~DatabaseStructureWidget()
{
    delete ui;
}

json DatabaseStructureWidget::objectStructure(const QUuid &uuid) const
{
//    auto model = (tree_model::TreeItemModel*)treeView->model();
//    auto index =  model->find(json{{"ref", uuid.toString(QUuid::WithoutBraces).toStdString()}}, QModelIndex());
    auto model = treeView->get_model();
    auto index = model->find(uuid);
    auto result = json::object();
    if(index.isValid()){
        result["object"] = model->to_object(index);
        result["detalis"] = model->to_array(index);
    }
    return result;
}

void DatabaseStructureWidget::load_structure()
{
    if(!m_client->isConnected())
        return;

    auto dev = m_client->exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::GetDatabaseStructure), json{});

    if(dev != WS_RESULT_ERROR && dev.is_object()){
        auto model = ibase_objects_init(treeView, this);
        auto rows = json::array();
        if(!dev.value("rows", json::array()).empty()){
            for (auto it = dev["rows"].begin(); it != dev["rows"].end(); ++it) {
                auto obj = *it;
                obj["package_ref"] = m_packade_uuid.toString(QUuid::WithoutBraces).toStdString();
                rows += obj;
            }
        }
        dev["rows"] = rows;
        model->set_table(dev);
        auto tablesRoot = model->find(model->column_index("object_type"), "tablesRoot",  QModelIndex());
        if(tablesRoot.isValid()){
            model->set_row_image(tablesRoot, QIcon(":/img/tables.png"));
        }
        auto tviewsRoot = model->find(model->column_index("object_type"), "viewsRoot", QModelIndex());
        if(tviewsRoot.isValid()){
            model->set_row_image(tviewsRoot, QIcon(":/img/views.png"));
            for (int i = 0 ; i < model->rowCount(tviewsRoot); ++i) {
                model->set_row_image(model->index(i,0, tviewsRoot), QIcon(":/img/view.png"));
            }
        }

        model->set_enable_drag(true);
        model->set_enable_drop(true);
        treeView->set_drag_group(true);
        treeView->set_drag_drop_behavior(drag_drop_behavior::behaviorEmit);
        treeView->hideColumn(model->column_index("data_type"));
    }


    connect(treeView, &QTreeView::doubleClicked, this, &DatabaseStructureWidget::onTreeViewOnDoubleClick);

}

void DatabaseStructureWidget::onTreeViewOnDoubleClick(const QModelIndex &index)
{

    auto model = (ITreeIbaseModel*)treeView->get_model();
    auto object = model->object(treeView->get_index(index));
    auto index_ = treeView->get_index(index);

    if(object.object_type == "tablesRoot" || object.object_type == "viewsRoot" || object.object_type == "field")
        return;

    object.parent = NIL_STRING_UUID;

    auto rows = model->to_array(index_);
    QVector<ibase_object_structure> vec;
    if(rows.is_array()){
        for (auto itr = rows.begin(); itr != rows.end(); ++itr) {
            vec.push_back(arcirk::secure_serialization<ibase_object_structure>(*itr));
        }
    }

    emit selectRow(object, vec);
}

void DatabaseStructureWidget::doObjectStructure(const QUuid &uuid)
{
    auto model = (ITreeIbaseModel*)treeView->get_model();
    auto index = model->find(uuid);
    if(!index.isValid())
        return;
    auto object = model->object(index);
    if(object.object_type == "table" || object.object_type == "view"){
        object.parent = NIL_STRING_UUID;
        auto rows = model->array(index);
        emit selectRow(object, rows);
    }else{
        if(object.object_type == "tablesRoot" || object.object_type == "viewsRoot"){
            auto rows = model->array(index, true);
            emit selectRows(rows);
        }else{
            auto parent = model->object(index.parent());
            auto rows = model->array(index.parent());
            emit selectRow(object, rows);
        }
    }
}

void DatabaseStructureWidget::onDropEvent(const json object, const QString& sender)
{
    //auto model = get_model<ITreeIbaseModel>(treeView);
    auto item = secure_serialization<ibase_object_structure>(object);
    if(item.is_group == 1){
        emit dropNode(QUuid::fromString(item.ref));
    }
}



void DatabaseStructureWidget::onBtnTypeToggled(bool checked)
{
    auto model = (ITreeIbaseModel*)treeView->get_model();
    int i = model->column_index("data_type");
    treeView->setColumnHidden(i, !checked);
}


void DatabaseStructureWidget::onBtnSetItemClicked()
{
    auto model = (ITreeIbaseModel*)treeView->get_model();
    auto index = treeView->current_index();
    if(!index.isValid())
        return;
    auto object = model->object(index);
    if(object.object_type == "tablesRoot" || object.object_type == "viewsRoot")
        return;
    if(object.is_group == 1){
        auto items = model->array(index);
        emit selectRow(object, items);
    }else{
        auto parent = model->object(index.parent());
        emit selectItem(parent,  model->array(index.parent()), object);
    }
}

void DatabaseStructureWidget::onBtnSetItemsClicked()
{
    auto model = (ITreeIbaseModel*)treeView->get_model();
    auto index = treeView->current_index();
    if(!index.isValid())
        return;
    auto object = model->object(index);
    if(object.object_type == "tablesRoot"){
        auto m_arr = model->array(index, true);
        emit selectRows(m_arr);
    }else if(object.object_type == "viewsRoot"){
        auto m_arr = model->array(index, true);
        emit selectRows(m_arr);
    }else{
        auto object = model->object(index);
        if(object.is_group == 1){
            auto m_arr = model->array(index, true);
            m_arr.insert(0, object);
            emit selectRows(m_arr);
        }else{
            auto object = model->object(index.parent());
            auto m_arr = model->array(index.parent(), true);
            m_arr.insert(0, object);
            emit selectRows(m_arr);
        }
    }
}

void DatabaseStructureWidget::onBtnRemoveItemClicked()
{
    emit removeRightItem();
}

void DatabaseStructureWidget::onBtnRemoveItemsClicked()
{
    emit removeRightItems();
}

