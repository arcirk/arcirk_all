#include "querybuildergroupbyfunwidget.h"
#include "ui_querybuildergroupbyfunwidget.h"
//#include "tree/treeitemdelegate.h"

using namespace arcirk::tree_model;
using namespace arcirk::query_builder_ui;

QueryBuilderGroupByFunWidget::QueryBuilderGroupByFunWidget(QWidget *parent, const QUuid& packade_uuid) :
    QWidget(parent),
    ui(new Ui::QueryBuilderGroupByFunWidget)
{
    ui->setupUi(this);

    m_packade_uuid = packade_uuid;

    treeGroupFields = new TreeViewWidget(this, "QueryBuilderGroupByFunWidget");
    treeGroupFields->setSelectionMode(QAbstractItemView::SingleSelection);
    treeGroupFields->set_drag_drop_behavior(drag_drop_behavior::behaviorEmit);

    ui->splitter->addWidget(treeGroupFields);
    treeAggregateFields = new TreeViewWidget(this, "QueryBuilderGroupByFunWidget");
    treeAggregateFields->setSelectionMode(QAbstractItemView::SingleSelection);
    treeAggregateFields->set_drag_drop_behavior(drag_drop_behavior::behaviorEmit);
    ui->splitter->addWidget(treeAggregateFields);

    auto model = new ITreeIbaseModel(QVector<QString>({"query"}), this);
    model->set_hierarchical_list(false);
    model->set_column_aliases(QMap<QString, QString>({qMakePair("query", "Поле группировки")}));
    model->set_rows_icon(item_icons_enum::Item, QIcon(":/img/column.png"));
    //model->drag_data_column("ref");
    model->set_enable_drag(true);
    model->set_enable_drop(true);

//    auto delegate_ = new TreeItemDelegate(true, treeAggregateFields);
//    treeGroupFields->setItemDelegate(delegate_);
//    treeGroupFields->setEditTriggers(QAbstractItemView::AllEditTriggers);
//    treeGroupFields->setIndentation(0);
    treeGroupFields->setModel(model);
    for (int i = 1; i < model->columnCount(); ++i) {
        treeGroupFields->hideColumn(i);
    }

    auto modelFun = new ITreeIbaseModel(QVector<QString>({"query", "groupe_as"}), this);
    modelFun->set_hierarchical_list(false);
    modelFun->set_column_aliases(QMap<QString, QString>({qMakePair("query", "Суммовое поле"), qMakePair("groupe_as", "Функция")}));
    modelFun->set_rows_icon(item_icons_enum::Item, QIcon(":/img/column.png"));
    //modelFun->set_drag_data_column("ref");
    modelFun->set_enable_drag(true);
    modelFun->set_enable_drop(true);
    modelFun->set_read_only(false);
    modelFun->set_user_role_data("query", tree::ReadOnlyRole, true);
    modelFun->set_user_role_data("groupe_as", tree::WidgetRole, widgetComboBoxRole);
    modelFun->set_user_role_data("groupe_as", tree::UserRoleExt, QVariant(QStringList({"Сумма", "Количество различных", "Количество", "Максимум", "Минимум", "Среднее"})));

    auto delegate = new TreeItemDelegate(true, treeAggregateFields);
    treeAggregateFields->setItemDelegate(delegate);
    treeAggregateFields->setEditTriggers(QAbstractItemView::AllEditTriggers);
    treeAggregateFields->setIndentation(0);

    treeAggregateFields->setModel(modelFun);
    for (int i = 2; i < modelFun->columnCount(); ++i) {
        treeAggregateFields->hideColumn(i);
    }

    connect(treeGroupFields, &TreeViewWidget::doDropEventJson, this, &QueryBuilderGroupByFunWidget::onTreeGropsDropEvent);
    connect(treeAggregateFields, &TreeViewWidget::doDropEventJson, this, &QueryBuilderGroupByFunWidget::onTreeGropsAgregateEvent);

    //model->use_memory_database("qbGroupBy");
    model->set_connection(root_tree_conf::sqlIteMemoryConnection, "", "qbGroupBy");
    model->set_user_sql_where(json{{"package_ref", m_packade_uuid.toString(QUuid::WithoutBraces).toStdString()}});
//    model->read_sql_table();
//    model->allow_edit_sql_data(true);
    //modelFun->use_memory_database("qbGroupByFun");
    model->set_connection(root_tree_conf::sqlIteMemoryConnection, "", "qbGroupByFun");
    modelFun->set_user_sql_where(json{{"package_ref", m_packade_uuid.toString(QUuid::WithoutBraces).toStdString()}});
//    modelFun->read_sql_table();
//    modelFun->allow_edit_sql_data(true);
}

QueryBuilderGroupByFunWidget::~QueryBuilderGroupByFunWidget()
{
    delete ui;
}

void QueryBuilderGroupByFunWidget::insert_group(const json &object, const QModelIndex &parent)
{
    json obj(object);
    obj["parent"] = NIL_STRING_UUID;
    treeGroupFields->insert(obj, parent);
}

void QueryBuilderGroupByFunWidget::insert_agregate(const json &object, const QModelIndex &parent)
{
    json obj(object);
    obj["parent"] = NIL_STRING_UUID;
    treeAggregateFields->insert(obj, parent);
}

void QueryBuilderGroupByFunWidget::remove(const QString &uuid)
{
    treeGroupFields->remove(uuid);
    treeAggregateFields->remove(uuid);
}

json QueryBuilderGroupByFunWidget::groups_array()
{
    auto model = treeGroupFields->get_model();
    return model->to_array(QModelIndex());
}

json QueryBuilderGroupByFunWidget::agregate_array()
{
    auto model = treeAggregateFields->get_model();
    return model->to_array(QModelIndex());
}

void QueryBuilderGroupByFunWidget::save_to_database()
{
//    get_model<ITreeIbaseModel>(treeGroupFields)->save_to_database();
//    get_model<ITreeIbaseModel>(treeAggregateFields)->save_to_database();
}

void QueryBuilderGroupByFunWidget::verify_fields(ITreeIbaseModel *model, const QList<QString> fields)
{
    QList<QUuid> m_delete;
    auto arr = model->array(QModelIndex());
    for (auto itr = arr.begin(); itr != arr.end(); ++itr) {
        if(fields.indexOf(itr->ref.c_str()) == -1){
            m_delete << QUuid::fromString(itr->ref.c_str());
        }
    }

    foreach (auto itr, m_delete) {
        auto index = model->find(itr);
        if(index.isValid())
            model->remove(index);
    }
}

void QueryBuilderGroupByFunWidget::onSetData(const ibase_object_structure &field)
{
    auto model = (ITreeIbaseModel*)treeGroupFields->get_model();
    model->add_struct(field);
}

void QueryBuilderGroupByFunWidget::onChangeFieldsList(const QList<QString> fields)
{
    verify_fields((ITreeIbaseModel*)treeGroupFields->get_model(), fields);
    verify_fields((ITreeIbaseModel*)treeAggregateFields->get_model(), fields);
}

void QueryBuilderGroupByFunWidget::onFieldChanged(const ibase_object_structure &field)
{
    auto model = treeAggregateFields->get_model();
    int col = model->column_index("base_ref");
    auto index = model->find(col, field.base_ref.c_str(), QModelIndex());
    if(index.isValid()){
        model->set_object(index, pre::json::to_json(field));
    }
    model = treeGroupFields->get_model();
    index = model->find(col, field.base_ref.c_str(), QModelIndex());
    if(index.isValid()){
        model->set_object(index, pre::json::to_json(field));
    }

}

void QueryBuilderGroupByFunWidget::onTreeGropsDropEvent(const json &data, const QString& sender)
{
    auto model = treeAggregateFields->get_model();
    auto index = model->find(QUuid::fromString(data.value("ref", "").c_str()), QModelIndex());
    if(index.isValid())
        model->remove(index);
    emit doTreeGropsDropEvent(data);
}

void QueryBuilderGroupByFunWidget::onTreeGropsAgregateEvent(const json &data, const QString& sender)
{
    auto model = treeGroupFields->get_model();
    auto index = model->find(QUuid::fromString(data.value("ref", "").c_str()), QModelIndex());
    if(index.isValid())
        model->remove(index);
    emit doTreeAgregateDropEvent(data);
}

void QueryBuilderGroupByFunWidget::onRemove(const QUuid &ref)
{
    auto model = treeAggregateFields->get_model();
    auto index = model->find(ref);
    if(index.isValid()){
        model->remove(index);
    }
    model = treeGroupFields->get_model();
    index = model->find(ref);
    if(index.isValid()){
        model->remove(index);
    }
}
