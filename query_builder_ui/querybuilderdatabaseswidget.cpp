#include "querybuilderdatabaseswidget.h"
#include "ui_querybuilderdatabaseswidget.h"
#include <QToolButton>
#include <QMessageBox>
#include "query_builder.hpp"
#include <QSqlQuery>
#include "sqlite_utils.hpp"


using namespace arcirk::query_builder_ui;
using namespace arcirk::database::builder;


QueryBuilderDatabasesWidget::QueryBuilderDatabasesWidget(QSqlDatabase& connection, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QueryBuilderDatabasesWidget),
    m_connection(connection)
{
    ui->setupUi(this);

    m_database_t = QMap<QUuid, object_t>();

    m_databasesToolbar = new TableToolBar(this);
    m_databasesToolbar->setButtonEnabled("add_group", true);
    m_databasesToolbar->setButtonVisible("move_up_item", false);
    m_databasesToolbar->setButtonVisible("move_down_item", false);
    m_databasesToolbar->setButtonVisible("add_item", false);
    m_databasesToolbar->setButtonVisible("move_to_item", false);
    m_databasesToolbar->setSeparatorVisible(0, false);
    m_databasesToolbar->addBotton("connect", QIcon(":/img/database_connect.png"));
    m_databasesToolbar->addBotton("disconnect", QIcon(":/img/database_disconnect.png"));

    auto btn = m_databasesToolbar->botton("add_group");
    if(btn){
        btn->setIcon(QIcon(":/img/database_new.png"));
    }
    btn = m_databasesToolbar->botton("connect");
    if(btn)
        btn->setEnabled(false);
    btn = m_databasesToolbar->botton("disconnect");
    if(btn)
        btn->setEnabled(false);

    m_treeDatabases = new TreeViewWidget(this);
    m_treeDatabases->setTableToolBar(m_databasesToolbar);
    m_treeDatabases->set_inners_dialogs(true);
    m_treeDatabases->set_only_groups_in_root(true);
    m_treeDatabases->add_group_in_root_only(true);

    auto model = new ITree<ibase_object_structure>(this);
    model->set_column_aliases(QMap<QString, QString>{
                                                     qMakePair("name", "Наименование"),
                                                     qMakePair("query", "Путь к файлу")
                                    }
                              );
    model->set_columns_order(QList<QString>{"name", "ref", "parent", "is_group"});
    model->set_user_role_data("name", tree::NotNullRole, true);
    model->set_user_role_data("query", tree::WidgetInnerRole, tree::tree_editor_inneer_role::widgetFilePath);
    model->set_user_role_data("query", tree::WidgetRole, tree::item_editor_widget_roles::widgetTextEditRole);
    model->set_user_role_data("query", tree::UserRoleExt, "Файлы базы данных (*.sqlite, *.sqlite3);;Все файлы (*.*)");
    foreach (auto itr, model->columnNames()) {
        if(itr != "name" && itr != "query"){
            model->set_user_role_data(itr, tree::user_role::UseRole, tree::attribute_use::forItem);
        }
    }

    model->set_rows_icon(tree::item_icons_enum::ItemGroup, QIcon(":/img/database_offline1.png"));
    model->set_rows_icon(tree::item_icons_enum::Item, QIcon(":/img/table.png"));

    m_treeDatabases->setModel(model);
    foreach (auto itr, model->columnNames()) {
        if(itr != "name"){
            m_treeDatabases->hideColumn(model->column_index(itr));
        }
    }

    ui->verticalLayout->addWidget(m_databasesToolbar);
    ui->verticalLayout->addWidget(m_treeDatabases);

    connect(m_treeDatabases, &TreeViewWidget::toolBarItemClicked, this, &QueryBuilderDatabasesWidget::onToolbarDatabasesClicked);
    connect(m_treeDatabases, &TreeViewWidget::treeItemClicked, this, &QueryBuilderDatabasesWidget::onTreeItemClicked);
    connect(m_treeDatabases, &TreeViewWidget::addTreeItem, this, &QueryBuilderDatabasesWidget::onAddTreeItem);
    connect(m_treeDatabases, &TreeViewWidget::editTreeItem, this, &QueryBuilderDatabasesWidget::onEditTreeItem);
    connect(m_treeDatabases, &TreeViewWidget::deleteTreeItem, this, &QueryBuilderDatabasesWidget::onDeleteTreeItem);

    setData();
}

QueryBuilderDatabasesWidget::~QueryBuilderDatabasesWidget()
{
    delete ui;
}

QList<QPair<QVariant, QVariant>> QueryBuilderDatabasesWidget::databasesList() const
{
    auto model = (ITree<ibase_object_structure>*)m_treeDatabases->get_model();
    auto arr = model->array(QModelIndex());
    QList<QPair<QVariant, QVariant>> lst{};
    for (auto itr = arr.begin(); itr != arr.end(); ++itr) {
        lst.append(qMakePair(itr->name.c_str(), itr->ref.c_str()));
    }
    return lst;
}

json QueryBuilderDatabasesWidget::database_structure(const QUuid &ref) const
{
    auto model = (ITree<ibase_object_structure>*)m_treeDatabases->get_model();
    auto index = model->find(ref);
    if(index.isValid()){
        if(model->rowCount(index) > 0){
            auto object = model->object(index);
            if(object.object_type == "Database"){
                return model->to_table_model(index, true);
            }
        }
    }
    return {};
}

object_t QueryBuilderDatabasesWidget::database_structure_t(const QUuid &ref) const
{
    if(m_database_t.find(ref) != m_database_t.end())
        return m_database_t[ref];
    else
        return {};
}

void QueryBuilderDatabasesWidget::updateIcons(const QModelIndex &parent)
{
    auto model = m_treeDatabases->get_model();
    for (int i = 0; i < model->rowCount(parent); ++i) {
        auto index = model->index(i, 0, parent);
        auto object = model->to_object(index);
        auto object_type = object["object_type"].get<std::string>() ;
        if(object_type == "tablesRoot")
            model->set_row_image(index, QIcon(":/img/tables.png"));
        else if(object_type == "viewsRoot")
            model->set_row_image(index, QIcon(":/img/views.png"));
        else if(object_type == "table")
            model->set_row_image(index, QIcon(":/img/table.png"));
        else if(object_type == "field")
            model->set_row_image(index, QIcon("://img/column.png"));
        else if(object_type == "view")
            model->set_row_image(index, QIcon(":/img/view.png"));

        updateIcons(index);
    }
}

void QueryBuilderDatabasesWidget::setData()
{
    if(!m_connection.isOpen())
        return;
    auto query = query_builder();
    auto table = query.select().from("qbDatabases").to_table(m_connection);
    auto model = m_treeDatabases->get_model();
    model->set_table(table);
    model->reset();
}

void QueryBuilderDatabasesWidget::onToolbarDatabasesClicked(const QString &bottonName)
{
    qDebug() << __FUNCTION__ << bottonName;
    if(bottonName == "connect"){
        auto model = (ITree<ibase_object_structure>*)m_treeDatabases->get_model();
        if(model){

            auto index = m_treeDatabases->current_index();
            if(!index.isValid())
                return;
            auto object = model->object(index);
            if(object.object_type != "Database")
                return;

            model->remove_childs(index);
            auto db = QSqlDatabase::addDatabase("QSQLITE", object.name.c_str());
            //auto db = QSqlDatabase::database(object.name.c_str());
            db.setDatabaseName(object.query.c_str());
            if(!db.isOpen())
                db.open();
            if(db.isOpen()){
                emit startDatabaseScanned();

                m_database_t.remove(QUuid::fromString(object.ref.c_str()));
                object_t vec;
                auto table = database::get_database_structure(db, vec, object.ref);

                if(!table.empty()){
                    model->set_table(table, index);
                    m_database_t.insert(QUuid::fromString(object.ref.c_str()), vec);
                }
                m_databasesToolbar->botton("connect")->setEnabled(false);
                m_databasesToolbar->botton("disconnect")->setEnabled(true);

                model->set_row_image(index, QIcon("://img/serverOnline.png"));
                updateIcons(index);
                model->reset();
                emit endDatabaseScanned();
            }

        }
    }else if(bottonName == "disconnect"){
        auto model = (ITree<ibase_object_structure>*)m_treeDatabases->get_model();
        if(model){
            auto index = m_treeDatabases->current_index();
            if(!index.isValid())
                return;
            auto object = model->object(index);
            if(object.object_type != "Database")
                return;

            auto db = QSqlDatabase::database(object.name.c_str());
            if(db.isOpen()){
                db.close();
            }
            QSqlDatabase::removeDatabase(object.name.c_str());
            model->remove_childs(index);
        }
    }
}

void QueryBuilderDatabasesWidget::onTreeItemClicked(const QModelIndex &current)
{
    auto index = m_treeDatabases->get_index(current);

    if(index.isValid()){
        auto model = (ITree<ibase_object_structure>*)m_treeDatabases->get_model();
        if(model){
            auto object = model->object(index);
            if(object.object_type == "Database"){
                auto count = model->rowCount(index);
                auto btn = m_databasesToolbar->botton("connect");
                if(btn)
                    btn->setEnabled(count == 0);
                btn = m_databasesToolbar->botton("disconnect");
                if(btn)
                    btn->setEnabled(count > 0);
            }else{
                auto btn = m_databasesToolbar->botton("connect");
                if(btn)
                    btn->setEnabled(false);
                btn = m_databasesToolbar->botton("disconnect");
                if(btn)
                    btn->setEnabled(false);
            }
        }
    }else{
        auto btn = m_databasesToolbar->botton("connect");
        if(btn)
            btn->setEnabled(false);
        btn = m_databasesToolbar->botton("disconnect");
        if(btn)
            btn->setEnabled(false);
    }
}

void QueryBuilderDatabasesWidget::onAddTreeItem(const QModelIndex &index, const json &data)
{
    auto object = secure_serialization<ibase_object_structure>(data);
    if(object.is_group == 1){
        object.object_type = "Database";
        auto model = (ITree<ibase_object_structure>*)m_treeDatabases->get_model();
        model->set_struct(object, index);
    }
    if(!m_connection.isOpen())
        return;
    auto query = query_builder();
    QSqlQuery rc(query.use(pre::json::to_json(object)).insert("qbDatabases").prepare().c_str(), m_connection);
    rc.exec();
    emit databaseListChanged();
}

void QueryBuilderDatabasesWidget::onEditTreeItem(const QModelIndex &index, const json &data)
{
    if(!m_connection.isOpen())
        return;
    auto query = query_builder();
    QSqlQuery rc(query.use(data).update("qbDatabases").where(json{{"ref", data["ref"]}}).prepare().c_str(), m_connection);
    rc.exec();
    if(rc.lastError().isValid())
        qCritical() << rc.lastError().text();
    emit databaseListChanged();
}

void QueryBuilderDatabasesWidget::onDeleteTreeItem(const json &data)
{
    if(!m_connection.isOpen())
        return;
    auto query = query_builder();
    QSqlQuery rc(query.remove().from("qbDatabases").where(json{{"ref", data["ref"]}}).prepare().c_str(), m_connection);
    rc.exec();
    emit databaseListChanged();
}

