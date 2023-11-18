#include "querybuildergroupbyfieldslistwidget.h"
#include "ui_querybuildergroupbyfieldslistwidget.h"
//#include "tree/treesortmodel.h"
#include <QTreeView>
#include <QToolButton>

using namespace arcirk::tree_model;
using namespace arcirk::query_builder_ui;

QueryBuilderGroupByFieldsListWidget::QueryBuilderGroupByFieldsListWidget(QWidget *parent, const QUuid& packade_uuid) :
    QWidget(parent),
    ui(new Ui::QueryBuilderGroupByFieldsListWidget)
{
    ui->setupUi(this);

    m_packade_uuid = packade_uuid;

    treeView = new TreeViewWidget(this, "QueryBuilderGroupByFieldsListWidget");
    treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    treeView->set_drag_drop_behavior(drag_drop_behavior::behaviorEmit);
    ui->hLayout->addWidget(treeView);

    auto model = new ITreeIbaseModel(QVector<QString>({"query"}), this);
    model->set_hierarchical_list(false);
    model->set_column_aliases(QMap<QString, QString>({qMakePair("query", "Поля"), qMakePair("data_type", "Тип")}));
    model->set_rows_icon(item_icons_enum::Item, QIcon(":/img/column.png"));
    //model->set_drag_data_column("ref");
    model->set_enable_drag(true);
    treeView->setModel(model);

 //   auto sort_model = new TreeSortModel(this);
//
//    sort_model->setSourceModel(model);
//    treeView->setModel(sort_model);

//    treeView->setIndentation(2);
//    for (int i = 0; i < model->columnCount(); ++i) {
//        if(model->column_name(i) != "query")
//            treeView->hideColumn(i);
//    }

    connect(treeView, &TreeViewWidget::doubleClicked, this, &QueryBuilderGroupByFieldsListWidget::onTreeViewDoubleClicked);
    connect(treeView, &TreeViewWidget::doDropEventJson, this, &QueryBuilderGroupByFieldsListWidget::onTreeDropEvent);
    connect(ui->btnType, &QToolButton::toggled, this, &QueryBuilderGroupByFieldsListWidget::onBtnTypeToggled);

    //model->use_memory_database("qbFields");
    model->set_connection(root_tree_conf::sqlIteMemoryConnection, "", "qbFields");
    model->set_user_sql_where(json{{"package_ref", packade_uuid.toString(QUuid::WithoutBraces).toStdString()}});
    //model->read_sql_table();
    //model->allow_edit_sql_data(true);
}

QueryBuilderGroupByFieldsListWidget::~QueryBuilderGroupByFieldsListWidget()
{
    delete ui;
}

void QueryBuilderGroupByFieldsListWidget::insert(const json &object, const QModelIndex &parent)
{
    treeView->insert(object, parent);
}

void QueryBuilderGroupByFieldsListWidget::remove(const QString &uuid)
{
    treeView->remove(uuid);
}

void QueryBuilderGroupByFieldsListWidget::onSetData(const QVector<ibase_object_structure>& fields)
{
    auto model = getModel();
    model->clear();
    model->add_items(fields);
    auto sort_model = (TreeSortModel*)treeView->model();
    sort_model->reset();
}

ITreeIbaseModel *QueryBuilderGroupByFieldsListWidget::getModel()
{
//    auto sort_model = (TreeSortModel*)treeView->model();
//    if(sort_model == 0)
//        return nullptr;
//    auto model = (ITreeIbaseModel*)sort_model->sourceModel();
//    if(model == 0)
//        return nullptr;
//    return model;
    return (ITreeIbaseModel*)treeView->get_model();
}

QModelIndex QueryBuilderGroupByFieldsListWidget::get_index(const QModelIndex &proxy_index)
{
//    if(!proxy_index.isValid())
//        return QModelIndex();
//    auto sort_model = (TreeSortModel*)treeView->model();
//    Q_ASSERT(sort_model != 0);
//    return sort_model->mapToSource(proxy_index);
    return treeView->current_index();
}

void QueryBuilderGroupByFieldsListWidget::onTreeViewDoubleClicked(const QModelIndex &index)
{
    auto model = getModel();
    auto object = model->object(get_index(index));
    emit selectData(object);
}

void QueryBuilderGroupByFieldsListWidget::onTreeDropEvent(const json &data, const QString& sender)
{
    emit doTreeDropEvent(data);
}

void QueryBuilderGroupByFieldsListWidget::onSetFilter(const QStringList &lst)
{
    qDebug() << __FUNCTION__ << lst;
    auto sort_model = (TreeSortModel*)treeView->model();
    sort_model->addFilter("ref", lst, type_of_comparison::Not_In_List);
}

void QueryBuilderGroupByFieldsListWidget::onFieldChanged(const ibase_object_structure &field)
{
    auto model = getModel();
    auto index = model->find(QUuid::fromString(field.ref.c_str()), QModelIndex());
    if(index.isValid()){
        model->set_struct(field, index);
        emit fieldChanged(field);
    }
}

void QueryBuilderGroupByFieldsListWidget::onRemove(const QUuid &ref)
{
    auto model = getModel();
    auto index = model->find(ref);
    if(index.isValid()){
        model->remove(index);
        emit remove_(ref);
    }
}


void QueryBuilderGroupByFieldsListWidget::onBtnTypeToggled(bool checked)
{
    auto model = getModel();
    int i = model->column_index("data_type");
    if(i != -1)
        treeView->setColumnHidden(i, !checked);
}

