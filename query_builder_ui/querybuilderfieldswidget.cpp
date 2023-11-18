#include "querybuilderfieldswidget.h"
#include "ui_querybuilderfieldswidget.h"
#include <QToolButton>
#include "querybuilderarbitraryexpressiondialog.h"
#include <QUuid>
#include <QTreeView>
#include <QMessageBox>
#include <QToolButton>
#include "iface/iface.hpp"

using namespace arcirk::query_builder_ui;


QueryBuilderFieldsWidget::QueryBuilderFieldsWidget(QWidget *parent, const QUuid& packade_uuid) :
    QWidget(parent),
    ui(new Ui::QueryBuilderFieldsWidget)
{
    ui->setupUi(this);

    m_packade_uuid = packade_uuid;

    treeView = new TreeViewWidget(this, "QueryBuilderFieldsWidget");
    ui->verticalLayout->addWidget(treeView);
    treeView->set_drag_drop_behavior(drag_drop_behavior::behaviorEmit);
    treeView->set_drag_group(true);
    auto model = ibase_objects_init(treeView, this, "query", false);
    model->set_enable_drag(true);
    model->set_enable_drop(true);
    //hide_column(treeView, "data_type", true);
    treeView->setColumnHidden(model->column_index("data_type"), true);

    connect(ui->btnAdd, &QToolButton::clicked, this, &QueryBuilderFieldsWidget::onBtnAddClicked);
    connect(ui->btnDelete, &QToolButton::clicked, this, &QueryBuilderFieldsWidget::onBtnDeleteClicked);
    connect(ui->btnEdit, &QToolButton::clicked, this, &QueryBuilderFieldsWidget::onBtnEditClicked);
    connect(treeView, &QTreeView::doubleClicked, this, &QueryBuilderFieldsWidget::onTreeViewDoubleClicked);
    connect(treeView, &TreeViewWidget::doDropEventJson, this, &QueryBuilderFieldsWidget::onDropEventJson);
    connect(this, &QueryBuilderFieldsWidget::updateIcons, this, &QueryBuilderFieldsWidget::onUpdateIcons);
    connect(ui->btnLeftSelectItem, &QToolButton::clicked, this, &QueryBuilderFieldsWidget::onBtnLeftSelectItemClicked);
    connect(ui->btnLeftSelectItems, &QToolButton::clicked, this, &QueryBuilderFieldsWidget::onBtnLeftSelectItemsClicked);
    connect(ui->btnRemoveItem, &QToolButton::clicked, this, &QueryBuilderFieldsWidget::onBtnRemoveItemClicked);
    connect(ui->btnRemoveItems, &QToolButton::clicked, this, &QueryBuilderFieldsWidget::onBtnRemoveItemsClicked);

    setProperty("typeName", "QueryBuilderFieldsWidget");

    //model->use_memory_database("qbFields");
    model->set_connection(root_tree_conf::sqlIteMemoryConnection, "", "qbFields");
    model->set_user_sql_where(json{{"package_ref", m_packade_uuid.toString(QUuid::WithoutBraces).toStdString()}});
    //model->read_sql_table();
    //model->allow_edit_sql_data(true);

}

QueryBuilderFieldsWidget::~QueryBuilderFieldsWidget()
{
    delete ui;
}

QVector<ibase_object_structure> QueryBuilderFieldsWidget::array()
{
    auto model = (ITreeIbaseModel*)treeView->get_model();
    return model->array(QModelIndex());
    //return get_model<ITreeIbaseModel>(treeView)->array(QModelIndex());
}

void QueryBuilderFieldsWidget::setBuilderArbitraryExpressionResult(const QString &query, const QString& ref)
{
    if(!query.trimmed().isEmpty()){
        auto row = ibase_object_structure();
        row.ref = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
        row.parent = NIL_STRING_UUID;
        row.user_query = true;
        row.query = query.toStdString();
        row.package_ref = m_packade_uuid.toString(QUuid::WithoutBraces).toStdString();
        auto model = (ITreeIbaseModel*)treeView->get_model();
        if(!ref.isEmpty()){
            auto index = model->find(QUuid::fromString(ref));
            if(index.isValid()){
                row = model->object(index);
                row.user_query = true;
                row.query = query.toStdString();
                model->set_struct(row, index);
            }
        }else{
            row.user_query = true;
            row.query = query.toStdString();
            model->add_struct(row);
        }
        emit updateIcons();
        emit changeListFields(model->array(QModelIndex()));
    }
}

void QueryBuilderFieldsWidget::onBtnAddClicked()
{
    emit openUserQueryDialog(true, "", this, "");
}


void QueryBuilderFieldsWidget::onBtnDeleteClicked()
{
    auto index = treeView->currentIndex();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
        return;
    }

    auto model = (ITreeIbaseModel*)treeView->model();
    auto object = model->object(index);
    if(QMessageBox::question(this, "Удаление поля", "Удалить выбранное поле?") == QMessageBox::Yes){
        model->remove(index);
        emit remove(QUuid::fromString(object.ref.c_str()));
    }
}

void QueryBuilderFieldsWidget::onBtnEditClicked()
{
    auto index = treeView->currentIndex();
    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
        return;
    }
    emit treeView->doubleClicked(index);
}

void QueryBuilderFieldsWidget::onSetData(const ibase_object_structure& table, const QVector<ibase_object_structure>& fields)
{

    QVector<ibase_object_structure> vec;
    foreach (auto itr, fields) {
        Q_ASSERT(itr.is_group == 0);
        auto item = itr;
        item.ref = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
        item.parent = NIL_STRING_UUID;
        vec.push_back(item);
    }

    auto model = (ITreeIbaseModel*)treeView->get_model();
    model->add_items(vec);
    emit changeListFields(model->array(QModelIndex()));
}

void QueryBuilderFieldsWidget::onChangeTablesList(const QVector<ibase_object_structure> &tables)
{
    auto model = (ITreeIbaseModel*)treeView->get_model();
    QStringList lst;
    foreach (auto itr, tables) {
        if(itr.is_group == 1)
            lst << itr.base_ref.c_str();
    }

    auto rows = model->array(QModelIndex());
    QList<QUuid> m_delete;

    foreach (auto itr, rows) {
        if(lst.indexOf(itr.base_parent.c_str())== -1){
            //qDebug() << "Удалить поле" << itr.query.c_str();
            m_delete << QUuid::fromString(itr.ref.c_str());
        }
    }

    foreach (auto itr, m_delete) {
        auto index = model->find(itr, QModelIndex());
        if(index.isValid())
            model->remove(index);
    }

    auto arr = model->array(QModelIndex());
    emit changeListFields(arr);

    QStringList m_fld{};
    foreach (auto itr, arr) {
        m_fld << itr.ref.c_str();
    }
    emit changeFields(m_fld);
}

void QueryBuilderFieldsWidget::onFieldChanged(const ibase_object_structure &field)
{
    auto model = (ITreeIbaseModel*)treeView->get_model();
    int col = model->column_index("base_ref");
    auto indexes = model->find_all(col, field.base_ref.c_str(), QModelIndex());
    foreach (auto index, indexes) {
        if(index.isValid()){
            auto i = model->object(index);
            ibase_object_structure f(field);
            f.ref = i.ref;
            model->set_struct(f, index);
            emit fieldChanged(f);
        }
    }

}

void QueryBuilderFieldsWidget::onDragResult(const QUuid &uuid, const QString& sender)
{
    if(sender == "QueryBuilderTablesWidget"){
        auto model = (ITreeIbaseModel*)treeView->get_model();
        auto index = model->find(uuid);
        if(index.isValid()){
            model->remove(index);
            emit remove(uuid);
        }
    }
}

void QueryBuilderFieldsWidget::onTreeViewDoubleClicked(const QModelIndex &index)
{
    if(!index.isValid())
        return;

    auto model = (ITreeIbaseModel*)treeView->get_model();
    auto item = model->object(treeView->get_index(index));
    emit openUserQueryDialog(true, item.query.c_str(), this, item.ref.c_str());
}

void QueryBuilderFieldsWidget::onUpdateIcons()
{
    auto model = (ITreeIbaseModel*)treeView->get_model();
    for (int i = 0; i < model->rowCount(); ++i) {
        auto index = model->index(i, 0);
        auto object = model->object(index);
        if(object.full_name != object.query)
            model->set_row_image(index, QIcon(":/img/user_fun.png"));
    }

}

void QueryBuilderFieldsWidget::onDropEventJson(const json object, const QString& sender)
{
    auto item = secure_serialization<ibase_object_structure>(object);
    if( sender == "QueryBuilderTablesWidget"){
        if(item.is_group != 1 && item.object_type != "view"){
            emit doSetData(QUuid::fromString(item.ref.c_str()));
        }
    }
}


void QueryBuilderFieldsWidget::onBtnLeftSelectItemClicked()
{
    emit selectItem();
}


void QueryBuilderFieldsWidget::onBtnLeftSelectItemsClicked()
{
    emit selectItems();
}


void QueryBuilderFieldsWidget::onBtnRemoveItemClicked()
{
    auto model = (ITreeIbaseModel*)treeView->get_model();
    auto index = treeView->current_index();
    if(index.isValid())
        model->remove(index);
}


void QueryBuilderFieldsWidget::onBtnRemoveItemsClicked()
{
    auto model = (ITreeIbaseModel*)treeView->get_model();
    model->clear();
}


void QueryBuilderFieldsWidget::save_to_database()
{
//    auto model = get_model<ITreeIbaseModel>(treeView);
//    model->save_to_database();
}
