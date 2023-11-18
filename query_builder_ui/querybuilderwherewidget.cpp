#include "querybuilderwherewidget.h"
#include "ui_querybuilderwherewidget.h"
#include <QTreeView>
#include <QToolButton>
#include "iface/iface.hpp"

using namespace arcirk::query_builder_ui;
using namespace arcirk::tree_model;

QueryBuilderWhereWidget::QueryBuilderWhereWidget(QWidget *parent, const QUuid& packade_uuid) :
    QWidget(parent),
    ui(new Ui::QueryBuilderWhereWidget)
{
    ui->setupUi(this);
    m_packade_uuid = packade_uuid;

    treeFields = new TreeViewWidget(this);
    treeFields->setSelectionMode(QAbstractItemView::SingleSelection);
    treeFields->set_drag_drop_behavior(drag_drop_behavior::behaviorEmit);
    treeFields->set_drag_group(false);
//    auto model = new ITreeIbaseModel(QVector<QString>({"query"}), this);
//    //model->set_hierarchical_list(false);
//    model->set_column_aliases(QMap<QString, QString>({qMakePair("query", "Поля"), qMakePair("data_type", "Тип")}));
//    model->set_hierarchy_list_icon(item_icons_enum::Item, QIcon(":/img/column.png"));
//    model->set_drag_data_column("ref");
//    model->enable_drag(true);
//    treeFields->setModel(model);
//    treeFields->setIndentation(2);
//    for (int i = 0; i < model->columnCount(); ++i) {
//        if(model->column_name(i) != "query")
//            treeFields->hideColumn(i);
//    }
    auto model = ibase_objects_init(treeFields, this, "alias");
    model->set_enable_drag(true);
    model->set_enable_drop(true);
    setProperty("typeName", "QueryBuilderTablesWidget");

   // hide_column(treeFields, "data_type", true);
    treeFields->set_drag_drop_behavior(drag_drop_behavior::behaviorEmit);
    treeFields->set_drag_group(true);

    //model->use_memory_database("qbTables");

    model->set_user_sql_where(json{{"package_ref", m_packade_uuid.toString(QUuid::WithoutBraces).toStdString()}});
    model->set_connection(root_tree_conf::sqlIteMemoryConnection, "", "qbTables");
    //model->read_sql_table();

    treeWhere = new TreeViewWidget(this);

    ui->LayoutTreeFields->addWidget(treeFields);
    ui->LayoutTreeWhere->addWidget(treeWhere);

    set_splitter_size(ui->splitter, parent, 30);

    auto modelFun = new ITreeInnersModel(QVector<QString>({"line_num", "user_query", "query"}), this);
    modelFun->set_hierarchical_list(false);
    modelFun->set_column_aliases(QMap<QString, QString>({qMakePair("line_num", "Номер"), qMakePair("user_query", "Простой"), qMakePair("query", "Условие")}));
    modelFun->set_enable_drop(true);
    modelFun->set_read_only(false);
    modelFun->set_user_role_data("line_num", tree::user_role::ReadOnlyRole,  true);
    modelFun->set_user_role_data("line_num", tree::user_role::RowCountRole, true);
    modelFun->set_enable_rows_icons(false);
    modelFun->set_user_role_data("query", tree::user_role::WidgetRole, tree::item_editor_widget_roles::widgetCompareRole);
    modelFun->set_user_role_data("user_query", tree::user_role::WidgetRole, tree::item_editor_widget_roles::widgetCheckBoxRole);


//    auto delegate = new TreeItemDelegate(true, treeWhere);
//    treeWhere->setItemDelegate(delegate);
//    treeWhere->setEditTriggers(QAbstractItemView::AllEditTriggers);
//    treeWhere->setIndentation(0);

    treeWhere->setModel(modelFun);
    for (int i = 3; i < modelFun->columnCount(); ++i) {
        treeWhere->hideColumn(i);
    }

    auto header = treeWhere->header();
    header->resizeSection(0, 20);
    header->resizeSection(1, 16);
    header->setSectionResizeMode(0, QHeaderView::Fixed);
    header->setSectionResizeMode(1, QHeaderView::Fixed);

    connect(treeFields, &TreeViewWidget::doubleClicked, this, &QueryBuilderWhereWidget::onTreeViewDoubleClicked);
    connect(treeFields, &TreeViewWidget::doDropEventJson, this, &QueryBuilderWhereWidget::onTreeDropEvent);
    connect(ui->btnType, &QToolButton::toggled, this, &QueryBuilderWhereWidget::onBtnTypeToggled);
}

QueryBuilderWhereWidget::~QueryBuilderWhereWidget()
{
    delete ui;
}

void QueryBuilderWhereWidget::onTablesChanged(const QVector<ibase_object_structure> &rows)
{
    auto model = (ITreeIbaseModel*)treeFields->get_model();
    model->clear();
    model->add_items(rows);
    //add_items<ITreeIbaseModel,ibase_object_structure>(rows, model);
}

//void QueryBuilderWhereWidget::insert(const json &object, const QModelIndex &parent)
//{
//    treeFields->insert(object, parent);
//}

//void QueryBuilderWhereWidget::remove(const QString &uuid)
//{
//    treeFields->remove(uuid);
//}


//void QueryBuilderWhereWidget::onSetData(const QVector<ibase_object_structure>& fields)
//{
//    auto model = get_model<ITreeIbaseModel>(treeFields);
//    model->clear();
//    add_items<ITreeIbaseModel,ibase_object_structure>(fields, model);

//}

void QueryBuilderWhereWidget::onTreeViewDoubleClicked(const QModelIndex &index)
{
    auto model = (ITreeIbaseModel*)treeFields->get_model();
    auto object = model->object(treeFields->get_index(index));
    emit selectData(object);
}

void QueryBuilderWhereWidget::onTreeDropEvent(const json &data, const QString& sender)
{
    emit doTreeDropEvent(data);
}

//void QueryBuilderWhereWidget::onFieldChanged(const ibase_object_structure &field)
//{
//    auto model = get_model<ITreeIbaseModel>(treeFields);
//    auto index = model->find(QUuid::fromString(field.ref.c_str()), QModelIndex());
//    if(index.isValid()){
//        model->set_struct(index, field);
//        emit fieldChanged(field);
//    }
//}

//void QueryBuilderWhereWidget::onRemove(const QUuid &ref)
//{
//    auto model = get_model<ITreeIbaseModel>(treeFields);
//    auto index = model->find(ref);
//    if(index.isValid()){
//        model->remove(index);
//        emit remove_(ref);
//    }
//}


void QueryBuilderWhereWidget::onBtnTypeToggled(bool checked)
{
    auto model = (ITreeIbaseModel*)treeFields->get_model();
    int i = model->column_index("data_type");
    if(i != -1)
        treeFields->setColumnHidden(i, !checked);
}

