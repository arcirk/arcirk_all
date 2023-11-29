#include "dialogquerybuilder.h"
#include "ui_dialogquerybuilder.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QSplitter>
#include <QToolButton>
#include <QSplitterHandle>
#include <QSizePolicy>
//#include <QPushButton>
#include "querybuilderarbitraryexpressiondialog.h"

#include <QDialogButtonBox>

using namespace arcirk::query_builder_ui;

DialogQueryBuilder::DialogQueryBuilder(WebSocketClient* client, const query_builder_packet& packade, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogQueryBuilder)
{
    ui->setupUi(this);

    m_client = client;

    init_class(m_client, packade, parent);

    m_query_fields->set_http_conf(http_conf(m_client->http_url().toString().toStdString() + "/api/info", m_client->conf().hash, "SQliteFunctionsInfo"));


}

DialogQueryBuilder::DialogQueryBuilder(const json &data, const query_builder_packet &packade, QWidget *parent):
    QDialog(parent),
    ui(new Ui::DialogQueryBuilder)
{

    ui->setupUi(this);
    m_database_structure = data;

    init_class(data, packade, parent);
}

DialogQueryBuilder::~DialogQueryBuilder()
{
    delete ui;
}

void DialogQueryBuilder::accept()
{

    m_query_tables->save_to_database();
    m_query_fields->save_to_database();
    m_group_by_function->save_to_database();
    m_table_inners->save_to_database();
    m_addionall_widget->save_to_database();
    return QDialog::accept();
}

void DialogQueryBuilder::reset_filer()
{
    json arr_fields = m_group_by_function->groups_array();
    json arr_agr = m_group_by_function->agregate_array();
    QStringList lst{};
    for (auto it = arr_fields.begin(); it != arr_fields.end(); ++it) {
        auto obj = *it;
        lst << obj.value("ref", "").c_str();
    }
    for (auto it = arr_agr.begin(); it != arr_agr.end(); ++it) {
        auto obj = *it;
        lst << obj.value("ref", "").c_str();
    }
    emit setFieldsFilter(lst);
}

QVector<ibase_object_structure> DialogQueryBuilder::for_arbitrary_table(bool selected_fields)
{
    QVector<ibase_object_structure> result;
    if(selected_fields){
        auto m_selected_fields = m_query_fields->array();
        result.resize(m_selected_fields.size());
        std::copy(m_selected_fields.begin(), m_selected_fields.end(), result.begin());
    }
    auto full_tables = m_query_tables->getModel()->array(QModelIndex(), true);
    auto root = ibase_object_structure();
    root.ref = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    root.query = "Все поля";
    root.name = "Все поля";
    root.alias = "Все поля";
    root.full_name = "Все поля";
    root.is_group = 1;
    root.parent = NIL_STRING_UUID;
    result << root;
    QMap<std::string, std::string> m_parents;
    foreach (auto itr, full_tables) {
        auto m = itr;
        if(m.is_group == 1){
            std::string new_ref = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
            m_parents.insert(m.ref, new_ref);
            m.ref = new_ref;
            m.parent = root.ref;
            m.query = m.alias;
        }else{
            if(m_parents.find(m.parent) != m_parents.end())
                m.parent = m_parents.find(m.parent).value();
            m.ref = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
        }
        result.push_back(m);
    }

    return result;
}


void DialogQueryBuilder::onChangeTableList(int count)
{
    m_tab->setTabVisible(TAB_INNERS, count > 1);
    emit changeTablesList();

}

void DialogQueryBuilder::onTabBarClicked(int index)
{
//    if(index==1){
//        auto m = m_table_inners->columns();
    //    }
}

void DialogQueryBuilder::onGroupByFieldsDropEvent(const json &object)
{
    qDebug() << __FUNCTION__;
    if(!object.empty()){
        m_group_by_function->remove(object.value("ref", "").c_str());
        reset_filer();
    }
}

void DialogQueryBuilder::onGroupByGroupsDropEvent(const json &object)
{
    qDebug() << __FUNCTION__;
    if(!object.empty()){
        m_group_by_function->insert_group(object);
        reset_filer();
    }
}

void DialogQueryBuilder::onGroupByFunctionsDropEvent(const json &object)
{
    qDebug() << __FUNCTION__;
    if(!object.empty()){
        m_group_by_function->insert_agregate(object);
        reset_filer();
    }
}

void DialogQueryBuilder::onOpenArbitraryFunctionsDialog(bool selected_fields, const QString &current_function, QWidget* parent, const QString& ref)
{
    auto tree_model = for_arbitrary_table(selected_fields);
    auto dlg = QueryBuilderArbitraryExpressionDialog(tree_model, current_function, parent, http_conf(m_client->http_url().toString().toStdString() + "/api/info", m_client->conf().hash, "SQliteFunctionsInfo"));
    if(dlg.exec() == QDialog::Accepted){
        QString typeName = parent->property("typeName").toString();
        if(typeName == "QueryBuilderFieldsWidget"){
            QueryBuilderFieldsWidget* w = qobject_cast<QueryBuilderFieldsWidget*>(parent);
            if(w){
                w->setBuilderArbitraryExpressionResult(dlg.query(), ref);
            }
        }
    }
}

void DialogQueryBuilder::onSelectedQueryType(sql_global_query_type index)
{
    switch (index) {
    case sqlTypeSelect:{
        for (int i = 0; i < m_tab->count(); ++i) {
            m_tab->setTabVisible(i, true);
        }
        m_tab->setTabVisible(TAB_INNERS, m_query_tables->count() > 1);
    }
        break;
    case sqlTypeCreateTempTable:{
        for (int i = 0; i < m_tab->count(); ++i) {
            m_tab->setTabVisible(i, true);
        }
        m_tab->setTabVisible(TAB_ORDER, false);
        m_tab->setTabVisible(TAB_TOTAL, false);
        m_tab->setTabVisible(TAB_INNERS, m_query_tables->count() > 1);
    }
        break;
    case sqlTypeDeleteTempTable:{
        for (int i = 1; i < m_tab->count(); ++i) {
            m_tab->setTabVisible(i, false);
        }
    }
        break;
    case sqlTypeCreateTable:{
        for (int i = 1; i < m_tab->count(); ++i) {
            m_tab->setTabVisible(i, false);
        }
    }
        break;
    case sqlTypeDeleteTable:{
        for (int i = 1; i < m_tab->count(); ++i) {
            m_tab->setTabVisible(i, false);
        }
    }
        break;
    default:
        break;
    }
}

void DialogQueryBuilder::onButtonBoxAccepted()
{
    accept();
}


void DialogQueryBuilder::onButtonBoxRejected()
{
    QDialog::close();
}

void DialogQueryBuilder::onButtonGenerateQuery()
{

}


template<class T>
void DialogQueryBuilder::init_class(T& dataCl, const query_builder_packet &packade, QWidget *parent)
{
    m_packade_uuid = QUuid::fromString(packade.ref.c_str());
    m_packade = packade;

    m_tab = new QTabWidget(this);
    m_structure = new DatabaseStructureWidget(dataCl, this, m_packade_uuid);
    m_query_tables = new QueryBuilderTablesWidget(m_structure, this, m_packade_uuid);
    m_query_fields = new QueryBuilderFieldsWidget( this, m_packade_uuid);
    m_table_inners = new QueryBuilderTableInnersWidget(m_query_tables, this, m_packade_uuid);
    m_group_by_fields = new QueryBuilderGroupByFieldsListWidget(this, m_packade_uuid);
    m_group_by_function = new QueryBuilderGroupByFunWidget(this, m_packade_uuid);
    m_addionall_widget = new QueryBuilderAdditionallyWidget(this, m_packade_uuid);
    m_inners_aliases_widget = new QueryBuilderInnersAliasesWidget(this);
    m_sort_widget = new QueryBuilderSortWidget(this);
    m_total_widget = new QueryBuilderTotalByHierarchy(this);
    m_where_widget = new QueryBuilderWhereWidget(this, m_packade_uuid);

    auto splitter = new QSplitter(Qt::Orientation::Horizontal, m_tab);
    auto splitter1 = new QSplitter(Qt::Orientation::Horizontal, m_tab);

    m_tab->addTab(m_addionall_widget, "Настройка");
    splitter->addWidget(m_structure);
    splitter->addWidget(m_query_tables);
    splitter->addWidget(m_query_fields);
    m_tab->addTab(splitter, "Таблицы и поля");
    m_tab->addTab(m_table_inners, "Связи");
    m_tab->setTabVisible(TAB_INNERS, false);
    splitter1->addWidget(m_group_by_fields);
    splitter1->addWidget(m_group_by_function);
    set_splitter_size(splitter1, parent, 30);

    m_tab->addTab(splitter1, "Группировка");
    m_tab->addTab(m_where_widget, "Условия");
    //m_tab->addTab(m_addionall_widget, "Дополнительно");
    m_tab->addTab(m_inners_aliases_widget, "Объединения/Псевдонимы");
    m_tab->addTab(m_sort_widget, "Порядок");
    m_tab->addTab(m_total_widget, "Итоги");

    ui->gridLayout->addWidget(m_tab);

    connect(m_tab, &QTabWidget::tabBarClicked, this, &DialogQueryBuilder::onTabBarClicked);

    connect(m_structure, &DatabaseStructureWidget::selectRow, m_query_tables, &QueryBuilderTablesWidget::onSetData);
    connect(m_structure, &DatabaseStructureWidget::selectRows, m_query_tables, &QueryBuilderTablesWidget::onSetArray);
    connect(m_structure, &DatabaseStructureWidget::selectItem, m_query_tables, &QueryBuilderTablesWidget::onSetDataEx);
    connect(m_structure, &DatabaseStructureWidget::dropNode, m_query_tables, &QueryBuilderTablesWidget::onStructureDropNode);
    connect(m_structure, &DatabaseStructureWidget::removeRightItem, m_query_tables, &QueryBuilderTablesWidget::onRemoveRightItem);
    connect(m_structure, &DatabaseStructureWidget::removeRightItems, m_query_tables, &QueryBuilderTablesWidget::onRemoveRightItems);
    connect(m_query_tables, &QueryBuilderTablesWidget::changeTableList, this, &DialogQueryBuilder::onChangeTableList);
    connect(m_query_tables, &QueryBuilderTablesWidget::objectStructure, m_structure, &DatabaseStructureWidget::doObjectStructure);


    connect(this, &DialogQueryBuilder::changeTablesList, m_table_inners, &QueryBuilderTableInnersWidget::onChangeTablesList);
    connect(m_query_tables, &QueryBuilderTablesWidget::renameTable, m_table_inners, &QueryBuilderTableInnersWidget::onRenameTable);

    connect(m_query_tables, &QueryBuilderTablesWidget::selectField, m_query_fields, &QueryBuilderFieldsWidget::onSetData);
    connect(m_query_tables, &QueryBuilderTablesWidget::dropObject, m_query_fields, &QueryBuilderFieldsWidget::onDragResult);

    connect(m_query_fields, &QueryBuilderFieldsWidget::changeListFields, m_group_by_fields, &QueryBuilderGroupByFieldsListWidget::onSetData);
    connect(m_query_tables,  &QueryBuilderTablesWidget::changeTables, m_where_widget, &QueryBuilderWhereWidget::onTablesChanged);

    connect(m_query_fields, &QueryBuilderFieldsWidget::changeFields, m_group_by_function, &QueryBuilderGroupByFunWidget::onChangeFieldsList);
    connect(m_query_fields, &QueryBuilderFieldsWidget::openUserQueryDialog, this, &DialogQueryBuilder::onOpenArbitraryFunctionsDialog);
    connect(m_query_fields, &QueryBuilderFieldsWidget::doSetData, m_query_tables, &QueryBuilderTablesWidget::doFiledsSetData);
    connect(m_query_fields, &QueryBuilderFieldsWidget::selectItem, m_query_tables, &QueryBuilderTablesWidget::onRightSetItem);
    connect(m_query_fields, &QueryBuilderFieldsWidget::selectItems, m_query_tables, &QueryBuilderTablesWidget::onRightSetItems);

    connect(m_group_by_fields, &QueryBuilderGroupByFieldsListWidget::selectData, m_group_by_function, &QueryBuilderGroupByFunWidget::onSetData);

    connect(m_group_by_fields, &QueryBuilderGroupByFieldsListWidget::doTreeDropEvent, this, &DialogQueryBuilder::onGroupByFieldsDropEvent);
    connect(m_group_by_function, &QueryBuilderGroupByFunWidget::doTreeGropsDropEvent, this, &DialogQueryBuilder::onGroupByGroupsDropEvent);
    connect(m_group_by_function, &QueryBuilderGroupByFunWidget::doTreeAgregateDropEvent, this, &DialogQueryBuilder::onGroupByFunctionsDropEvent);

    connect(this, &DialogQueryBuilder::setFieldsFilter, m_group_by_fields, &QueryBuilderGroupByFieldsListWidget::onSetFilter);

    connect(m_query_tables,  &QueryBuilderTablesWidget::changeTables, m_query_fields, &QueryBuilderFieldsWidget::onChangeTablesList);

    connect(m_query_tables, &QueryBuilderTablesWidget::fieldChanged, m_query_fields, &QueryBuilderFieldsWidget::onFieldChanged);
    connect(m_query_fields, &QueryBuilderFieldsWidget::fieldChanged, m_group_by_fields, &QueryBuilderGroupByFieldsListWidget::onFieldChanged);
    connect(m_query_fields, &QueryBuilderFieldsWidget::remove, m_group_by_fields, &QueryBuilderGroupByFieldsListWidget::onRemove);



    connect(m_group_by_fields, &QueryBuilderGroupByFieldsListWidget::fieldChanged, m_group_by_function, &QueryBuilderGroupByFunWidget::onFieldChanged);
    connect(m_group_by_fields, &QueryBuilderGroupByFieldsListWidget::remove_, m_group_by_function, &QueryBuilderGroupByFunWidget::onRemove);


    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &DialogQueryBuilder::onButtonBoxAccepted);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &DialogQueryBuilder::onButtonBoxRejected);

    connect(m_addionall_widget, &QueryBuilderAdditionallyWidget::selectedQueryType, this, &DialogQueryBuilder::onSelectedQueryType);

    connect(ui->btnQuery, &QPushButton::clicked, this, &DialogQueryBuilder::onButtonGenerateQuery);

    m_query_name = "Запрос пакета";

    if(m_query_tables->count() > 0){
        onChangeTableList(m_query_tables->count());
    }

    onSelectedQueryType((sql_global_query_type)m_packade.type);

    reset_filer();

    m_addionall_widget->set_database_structure(m_structure->model());
}
