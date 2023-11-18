#include "querybuildertableswidget.h"
#include "ui_querybuildertableswidget.h"
//#include "tree/treeitemmodel.h"
//#include "treeitemmodel.h"
#include "dialogtemptable.h"
#include <QMessageBox>
#include "dialogsettablealias.h"
//#include "treeitemdelegate.h"
#include <QToolButton>
#include <QTreeView>

using namespace arcirk::tree_model;
using namespace arcirk::query_builder_ui;

QueryBuilderTablesWidget::QueryBuilderTablesWidget(DatabaseStructureWidget* structure, QWidget *parent, const QUuid& packade_uuid) :
    QWidget(parent),
    ui(new Ui::QueryBuilderTablesWidget)
{
    ui->setupUi(this);

    m_structure = structure;
    m_packade_uuid = packade_uuid;

    treeView = new TreeViewWidget(this, "QueryBuilderTablesWidget");
    ui->verticalLayout->addWidget(treeView);

    auto model = ibase_objects_init(treeView, this, "alias");
    model->set_enable_drag(true);
    model->set_enable_drop(true);

    //hide_column(treeView, "data_type", true);
    treeView->setColumnHidden(model->column_index("data_type"), true);
    treeView->set_drag_drop_behavior(drag_drop_behavior::behaviorEmit);
    treeView->set_drag_group(true);

    connect(ui->btnHideColumn, &QToolButton::clicked, this, &QueryBuilderTablesWidget::onBtnHideColumnClicked);
    connect(ui->btnEdit, &QToolButton::clicked, this, &QueryBuilderTablesWidget::onBtnEditClicked);
    connect(ui->btnNewTempTable, &QToolButton::clicked, this, &QueryBuilderTablesWidget::onBtnNewTempTableClicked);
    connect(ui->btnTableReplace, &QToolButton::clicked, this, &QueryBuilderTablesWidget::onBtnTableReplaceClicked);
    connect(ui->btnDelete, &QToolButton::clicked, this, &QueryBuilderTablesWidget::onBtnDeleteClicked);
    connect(treeView, &QTreeView::doubleClicked, this, &QueryBuilderTablesWidget::onTreeViewDoubleClicked);
    connect(treeView, &TreeViewWidget::doDropEventJson, this, &QueryBuilderTablesWidget::onDropEventJson);

    setProperty("typeName", "QueryBuilderTablesWidget");

    //model->use_memory_database("qbTables");
    model->set_connection(root_tree_conf::sqlIteMemoryConnection, "", "qbTables");
    model->set_user_sql_where(json{{"package_ref", m_packade_uuid.toString(QUuid::WithoutBraces).toStdString()}});
    //model->read_sql_table();
    //model->allow_edit_sql_data(true);
}

QueryBuilderTablesWidget::~QueryBuilderTablesWidget()
{
    delete ui;
}

void QueryBuilderTablesWidget::set_data(const ibase_object_structure &table, const QVector<ibase_object_structure> &fields, bool erase)
{

    auto model = (ITreeIbaseModel*)treeView->get_model();

    if(erase){
        auto index = model->find(QUuid::fromString(table.ref.c_str()), QModelIndex());
        if(index.isValid()){
            model->remove(index);
        }
    }
    auto tbl = json::object();
    auto t = pre::json::to_json(ibase_object_structure());
    auto columns = json::array();

    for (auto itr = t.items().begin(); itr != t.items().end(); ++itr) {
        columns += itr.key();
    }

    auto rows = json::array();
    auto p = pre::json::to_json(table);

    std::string parent = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();

    if(!erase){
        p["ref"] = parent;
    }
    rows += p;
    foreach (auto itr, fields) {
        auto r = pre::json::to_json(itr);
        r["ref"] = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
        if(!erase)
            r["parent"] = parent;
        rows += r;
    }

    tbl["columns"] = columns;
    tbl["rows"] = rows;

//    if(model->rowCount() == 0)
        model->set_table(tbl);
//    else
//        model->insert_table(tbl);

    for (int i = 0 ; i < model->rowCount(); ++i) {
        auto index = model->index(i, model->column_index("object_type"));
        if(index.data().toString() == "view")
            model->set_row_image(index, QIcon(":/img/view.png"));
        if(index.data().toString() == "TempTable")
            model->set_row_image(index, QIcon(":/img/temp_table.png"));
        if(index.data().toString() == "TempTableDesc")
            model->set_row_image(index, QIcon(":/img/temp_table_desc.png"));
    }

    auto index = model->find(QUuid::fromString(table.ref.c_str()), QModelIndex());
    if(index.isValid()){
        treeView->expand(index);
        treeView->setCurrentIndex(index);
    }

    emit changeTableList(model->rowCount());
    emit changeTables(model->array(QModelIndex(), true));
}

void QueryBuilderTablesWidget::rename_table_details(const QModelIndex &parent)
{
    auto model = (ITreeIbaseModel*)treeView->get_model();
    auto table = model->object(parent);
    auto arr = model->array(parent);
    foreach (auto itr, arr) {
        auto row = itr;
        row.full_name = table.alias + "." + row.name;
        row.query = table.alias + "." + row.name;
        row.parent_alias = table.alias;
        row.parent_name = table.name;
        auto index = model->find(QUuid::fromString(row.ref.c_str()), parent);
        if(index.isValid()){
            model->set_struct(row, index);
            emit fieldChanged(row);
        }
    }
//    emit changeTableList(model->rowCount());
//    emit changeTables(model->array(QModelIndex()));
}

ITreeIbaseModel *QueryBuilderTablesWidget::getModel()
{
    return (ITreeIbaseModel*)treeView->get_model();
}

std::string QueryBuilderTablesWidget::to_base64()
{
    auto model = getModel();
    auto arr = model->to_array(QModelIndex(), true);
    return QByteArray(arr.dump().data()).toBase64().toStdString();
}

void QueryBuilderTablesWidget::save_to_database()
{
//    auto model = getModel();
//    model->save_to_database();
}

void QueryBuilderTablesWidget::onSetData(const ibase_object_structure &table, const QVector<ibase_object_structure> &fields)
{
    set_data(table, fields);

}

void QueryBuilderTablesWidget::onSetDataEx(const ibase_object_structure &table, const QVector<ibase_object_structure> &fields, const ibase_object_structure &to_field)
{
    auto model = getModel();
    int i = model->column_index("base_ref");
    auto index = model->find(i, QVariant(table.base_ref.c_str()));
    if(!index.isValid())
        set_data(table, fields);

    auto itemIndex = model->find(i, QVariant(to_field.base_ref.c_str()));
    if(itemIndex.isValid()){
        auto item = model->object(itemIndex);
        if(item.is_group == 0){
            auto parent = model->object(itemIndex.parent());
            QVector<ibase_object_structure> vec({item});
            emit selectField(parent, vec);
        }
    }
}

void QueryBuilderTablesWidget::onStructureDropNode(const QUuid &uuid)
{
    auto model = treeView->get_model();
    auto index = model->find(uuid);
    if(index.isValid())
        model->remove(index);
}

void QueryBuilderTablesWidget::doFiledsSetData(const QUuid &uuid)
{
    auto model = (ITreeIbaseModel*)treeView->get_model();
    auto index = model->find(uuid);
    if(index.isValid()){
        auto item = model->object(index);
        if(item.is_group == 0){
            auto parent = model->object(index.parent());
            QVector<ibase_object_structure> vec({item});
            emit selectField(parent, vec);
        }
    }

}

void QueryBuilderTablesWidget::onSetArray(const QVector<ibase_object_structure> &fields)
{
    QMap<std::string, ibase_object_structure> m_groups{};
    QVector<ibase_object_structure> results;

    foreach (auto itr, fields) {
        if(itr.is_group == 0)
            continue;
        auto obj = itr;
        obj.ref = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
        obj.parent = NIL_STRING_UUID;
        m_groups.insert(itr.ref, obj);
        results.push_back(obj);
    }

    foreach (auto itr, fields) {
        if(itr.is_group == 1)
            continue;
        auto obj = itr;
        obj.ref = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
        auto it = m_groups.find(itr.parent);
        if(it != m_groups.end()){
            obj.parent = it.value().ref;
        }else
            obj.parent = NIL_STRING_UUID;

        results.push_back(obj);
    }
    auto model = (ITreeIbaseModel*)treeView->get_model();
    model->add_items(results);

}

void QueryBuilderTablesWidget::onRemoveRightItems()
{
    auto model = (ITreeIbaseModel*)treeView->get_model();
    auto index = treeView->current_index();
    if(!index.isValid())
        model->clear();
    else{
        auto object = model->object(index);
        if(object.is_group == 1 || object.object_type == "view" )
            model->remove(index);
        else{
            if(object.is_group == 0 ){
                model->remove(index.parent());
            }
        }
    }
}

void QueryBuilderTablesWidget::onRemoveRightItem()
{
    auto model = (ITreeIbaseModel*)treeView->get_model();
    auto index = treeView->current_index();
    if(!index.isValid())
        return;
    else{
        auto object = model->object(index);
        if(object.is_group == 1 || object.object_type == "view" )
            model->remove(index);
    }
}

void QueryBuilderTablesWidget::onRightSetItem()
{
    auto model = (ITreeIbaseModel*)treeView->get_model();
    auto index = treeView->current_index();
    if(!index.isValid())
        return;
    else{
        auto object = model->object(index);
        if(object.is_group != 1){
            auto parent = model->object(index.parent());
            emit selectField(parent, QVector<ibase_object_structure>{object});
        }

    }
}

void QueryBuilderTablesWidget::onRightSetItems()
{
    auto model = (ITreeIbaseModel*)treeView->get_model();
    auto index = treeView->current_index();
    if(!index.isValid())
        return;
    else{
        auto object = model->object(index);
        if(object.is_group == 1){
            emit selectField(object, model->array(index));
        }

    }
}

void QueryBuilderTablesWidget::onBtnEditClicked()
{
    auto model = (ITreeIbaseModel*)treeView->get_model();
    auto index = treeView->current_index();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
        return;
    }

    auto object = model->object(index);
    QString old_name = object.alias.c_str();
    QString new_name;
    if(object.object_type == "view"){
        if(object.is_group){
            auto details = model->array(index);
            auto dlg = DialogTempTable(object, details, this);
            if(dlg.exec() == QDialog::Accepted){
                set_data(object, details, true);
                new_name = object.alias.c_str();
            }
        }
    }else if(object.object_type == "table"){
        auto dlg = DialogSetTableAlias(object.name, object.alias, this);
        if(dlg.exec() == QDialog::Accepted){
            model->set_struct(object, index);
            new_name = object.alias.c_str();
            rename_table_details(index);
        }
    }else if(object.object_type == "TempTableDesc"){
        if(object.is_group){
            auto details = model->array(index);
            auto dlg = DialogTempTable(object, details, this);
            if(dlg.exec() == QDialog::Accepted){
                set_data(object, details, true);
                new_name = object.alias.c_str();

            }
        }
    }

    if(model->rowCount() > 1)
        emit renameTable(old_name, new_name);
}


void QueryBuilderTablesWidget::onBtnNewTempTableClicked()
{
    auto table = ibase_object_structure();
    table.object_type = "TempTableDesc";
    table.ref = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    table.parent = NIL_STRING_UUID;
    table.name = "TempTableDesc";
    table.alias = "TempTableDesc";
    table.is_group = 1;
    table.package_ref = m_packade_uuid.toString(QUuid::WithoutBraces).toStdString();
    QVector<ibase_object_structure> details{};
    auto dlg = DialogTempTable(table, details, this);
    if(dlg.exec() == QDialog::Accepted){
        set_data(table, details);
    }
}


void QueryBuilderTablesWidget::onBtnTableReplaceClicked()
{
//    auto model = (TreeItemModel*)ui->treeView->model();
//    model->test_internalPointer(ui->treeView->currentIndex());

//    auto tbl = json::object();
//    auto t = pre::json::to_json(query_table_item());
//    auto columns = json::array();

//    for (auto itr = t.items().begin(); itr != t.items().end(); ++itr) {
//        columns += itr.key();
//    }

//    auto rows = json::array();

//    auto table = query_table_item();
//    table.object_type = "TempTableDesc";
//    table.ref = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
//    table.parent = NIL_STRING_UUID;
//    table.name = "TempTableDesc";
//    table.alias = "TempTableDesc";
//    table.is_group = 1;
//    QVector<query_table_item> details{};
//    auto dlg = DialogTempTable(table, details, this);
//    if(dlg.exec() == QDialog::Accepted){
//        insert_object(table, details);
//    }
}


void QueryBuilderTablesWidget::onBtnDeleteClicked()
{
    auto index = treeView->current_index();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
        return;
    }

    auto model = (ITreeIbaseModel*)treeView->get_model();
    auto object = model->object(index);
    if(object.is_group == 1){
        if(QMessageBox::question(this, "Удаление таблицы", "Удалить выбранную таблицу?") == QMessageBox::Yes)
            model->remove(index);
    }

    emit changeTableList(model->rowCount());
    emit changeTables(model->array(QModelIndex(), true));
}


void QueryBuilderTablesWidget::onTreeViewDoubleClicked(const QModelIndex &index)
{
    auto index_ = treeView->get_index(index);
    if(!index_.isValid())
        return;
    auto model = getModel();
    auto item = model->object(index_);
    if(item.is_group == 1)
        return;
    auto parent = model->object(index_.parent());
    QVector<ibase_object_structure> vec({item});
    emit selectField(parent, vec);

}


void QueryBuilderTablesWidget::onBtnHideColumnClicked(bool checked)
{
    int index = getModel()->column_index("data_type");
    treeView->setColumnHidden(index , !checked);
}

void QueryBuilderTablesWidget::onDropEventJson(const json object, const QString& sender)
{

    auto item = secure_serialization<ibase_object_structure>(object);
    if( sender == "DatabaseStructureWidget"){
        if(item.is_group == 0){
            if(item.object_type == "view")
                emit objectStructure(QUuid::fromString(item.ref));
            else{
                auto model = getModel();
                auto i = model->column_index("base_ref");
                auto index = model->find(i, QVariant(item.ref.c_str()));
                if(index.isValid()){
                    auto perent = model->object(index.parent());
                    emit selectField(perent, QVector<ibase_object_structure>{model->object(index)});
                }else{
                    emit objectStructure(QUuid::fromString(item.parent));
                }
            }
        }else if(item.is_group == 1){
            emit objectStructure(QUuid::fromString(item.ref));
        }
    }else if(sender == "QueryBuilderFieldsWidget"){
        emit dropObject(QUuid::fromString(item.ref.c_str()), this->property("typeName").toString());
    }

}

