#include "querybuildertableinnerswidget.h"
#include "ui_querybuildertableinnerswidget.h"
#include <QUuid>
//#include "tree/treeitemdelegate.h"
#include <QToolButton>
#include "gui/treeitemdelegate.h"

using namespace arcirk::query_builder_ui;
using namespace arcirk::tree_widget;

QueryBuilderTableInnersWidget::QueryBuilderTableInnersWidget(QueryBuilderTablesWidget* query_tables, QWidget *parent, const QUuid& packade_uuid) :
    QWidget(parent),
    ui(new Ui::QueryBuilderTableInnersWidget)
{
    ui->setupUi(this);

    //qDebug() << __FUNCTION__ << "start";

    treeView = new TreeViewWidget(this);
    ui->verticalLayout->addWidget(treeView);

    m_query_tables = query_tables;
    m_packade_uuid = packade_uuid;

    auto const vec = QVector<QString>({"line_num",
                                       "left_table",
                                       "left_all",
                                       "right_table",
                                       "right_all",
                                       "user_opt",
                                       "text",
                                       });
    auto model = new ITreeInnersModel(vec, this);
    model->set_server_object(arcirk::server::server_objects::QueryBuilderJnners);
    model->set_hierarchical_list(false);
    model->set_enable_rows_icons(false);
    model->set_read_only(false);
    model->set_user_role_data("line_num", tree::ReadOnlyRole, true);
    model->set_user_role_data("line_num", tree::RowCountRole, true);
    model->set_user_role_data("text", tree::UserRoleExt, 5);
    model->set_column_aliases(QMap<QString, QString>({qMakePair("line_num", "N")
                                                      , qMakePair("left_table", "Таблица 1")
                                                      , qMakePair("left_all", "Все")
                                                      , qMakePair("right_table", "Таблица 2")
                                                      , qMakePair("right_all", "Все")
                                                      , qMakePair("user_opt", "Прозвольное")
                                                      , qMakePair("text", "Условие связи")}));
    model->set_user_role_data("left_all", tree::WidgetRole, widgetCheckBoxRole);
    model->set_user_role_data("right_all", tree::WidgetRole, widgetCheckBoxRole);
    model->set_user_role_data("user_opt", tree::WidgetRole, widgetCheckBoxRole);
    model->set_user_role_data("text", tree::WidgetRole, widgetCompareRole);
    model->set_user_role_data("left_table", tree::WidgetRole, widgetComboBoxRole);
    model->set_user_role_data("right_table", tree::WidgetRole, widgetComboBoxRole);

    //model->use_memory_database("qbTableInners");
    model->set_connection(root_tree_conf::sqlIteMemoryConnection, "", "qbTableInners");
    model->set_user_sql_where(json{{"package_ref", packade_uuid.toString(QUuid::WithoutBraces).toStdString()}});
    //model->read_sql_table();

//    auto delegate = new TreeItemDelegate(true, ui->treeView);
//    ui->treeView->setItemDelegate(delegate);
//    ui->treeView->setEditTriggers(QAbstractItemView::AllEditTriggers);
//    ui->treeView->setIndentation(0);

    treeView->setModel(model);
    for(auto i = 0; i < model->columnCount(); i++){
        if(vec.indexOf(model->column_name(i)) == -1)
            treeView->hideColumn(i);
    }
    auto header = treeView->header();
    header->resizeSection(0, 20);
    header->resizeSection(2, 16);
    header->resizeSection(4, 16);
    header->resizeSection(5, 16);
//    header->setSectionResizeMode(0, QHeaderView::Stretch);
//    header->setSectionResizeMode(1, QHeaderView::Fixed);
    header->setSectionResizeMode(0, QHeaderView::Fixed);
    header->setSectionResizeMode(2, QHeaderView::Fixed);
    header->setSectionResizeMode(4, QHeaderView::Fixed);
    header->setSectionResizeMode(5, QHeaderView::Fixed);
    //header->setStretchLastSection(false);
    //qDebug() << __FUNCTION__ << "end";

    connect(treeView->delegate(), &TreeItemDelegate::selectedItemComboBoxChanged, this, &QueryBuilderTableInnersWidget::onSelectedItemComboBoxChanged);
    connect(ui->btnAdd, &QToolButton::clicked, this, &QueryBuilderTableInnersWidget::onBtnAddClicked);

}

QueryBuilderTableInnersWidget::~QueryBuilderTableInnersWidget()
{
    delete ui;
}

void QueryBuilderTableInnersWidget::save_to_database()
{
    //auto model = treeView->get_model();
    //save_to_database();
}

void QueryBuilderTableInnersWidget::onBtnAddClicked()
{
    auto model = (ITreeInnersModel*)treeView->get_model();
    auto empty = model->empty_row();
    empty.ref = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    empty.parent = NIL_STRING_UUID;
    empty.package_ref = m_packade_uuid.toString(QUuid::WithoutBraces).toStdString();
    model->add_struct(empty);

}

void QueryBuilderTableInnersWidget::onChangeTablesList()
{
    auto modelTables = m_query_tables->getModel();
    //auto tables_ = modelTables->array(QModelIndex(), true);
    auto tables_ = modelTables->to_array(QModelIndex(), true);
    fields.clear();
    tables.clear();

    auto rootItem = ibase_object_structure();
    rootItem.ref = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    rootItem.alias = "Все поля";
    rootItem.name = "Все поля";
    rootItem.query = "Все поля";
    rootItem.full_name = "Все поля";
    rootItem.parent = NIL_STRING_UUID;
    rootItem.is_group = 1;
    rootItem.package_ref = m_packade_uuid.toString(QUuid::WithoutBraces).toStdString();

    current_tables = QStringList{""};
    int count = -1;
    for (auto itr = tables_.begin(); itr != tables_.end(); ++itr) {
        auto table = secure_serialization<ibase_object_structure>(*itr);
        count++;
        if(table.is_group == 0)
            continue;
        else{
            tables_[count]["parent"] = rootItem.ref;
            tables_[count]["query"] = tables_[count]["name"];
            tables_[count]["package_ref"] = m_packade_uuid.toString(QUuid::WithoutBraces).toStdString();
        }
        tables.insert(table.alias.c_str(), table);
        auto index = modelTables->find(QUuid::fromString(table.ref.c_str()), QModelIndex());
        if(index.isValid())
            fields.insert(table.ref.c_str(), modelTables->array(index));
        current_tables.append(table.alias.c_str());
    }

//    foreach (auto table, tables_) {
//        if(table.is_group == 0)
//            continue;
//        tables.insert(table.alias.c_str(), table);
//        auto index = modelTables->find(QUuid::fromString(table.ref.c_str()), QModelIndex());
//        if(index.isValid())
//            fields.insert(table.ref.c_str(), modelTables->array(index));
//        current_tables.append(table.alias.c_str());
//    }

    auto model = (ITreeInnersModel*)treeView->get_model();
    model->set_user_role_data("left_table", tree::UserRoleExt, QVariant(current_tables));
    model->set_user_role_data("right_table", tree::UserRoleExt, QVariant(current_tables));


    tables_.insert(tables_.begin(), pre::json::to_json(rootItem));
    model->setProperty("full_tree", tables_.dump().c_str());
    //model->reset_user_role_data();
}

void QueryBuilderTableInnersWidget::onSelectedItemComboBoxChanged(int row, int col, const QString &value, int index)
{
    //qDebug() << __FUNCTION__;

    auto model = getModel();
    int left = model->column_index("left_table");
    int right = model->column_index("right_table");

    if(col != left && col != right)
        return;

    //int col_ = col == left ? right : left;

    QStringList lst = current_tables;

    if(!value.isEmpty()){
        auto i = current_tables.indexOf(value);
        if(i !=-1){
            lst.remove(i);
        }

    }

    //model->reset_user_role_data(model->index(row, col_), UserRoleExt, QVariant(lst));

    int i_text = model->column_index("text");
    Q_ASSERT(i_text!=-1);

    //Передаем поля
    //tree::user_role role = col == left ? CompareWidgetLeftRole : CompareWidgetRightRole;
    auto details = set_fields(value);
    //model->reset_user_role_data(model->index(row, i_text), role, QVariant(details.dump().c_str()));
}

void QueryBuilderTableInnersWidget::onRenameTable(const QString &old_name, const QString &new_name)
{
    if(old_name == new_name)
        return;

    onChangeTablesList();
    //if(current_tables.indexOf(old_name) != -1){
    //    current_tables[current_tables.indexOf(old_name)] = new_name;
        auto model = getModel();
        int left = model->column_index("left_table");
        int right = model->column_index("right_table");
        int i_text = model->column_index("text");
        //передаем всю модель
        auto tables_model = m_query_tables->getModel();
        auto full = tables_model->to_table_model(QModelIndex(), true);


        auto details = set_fields(new_name);
        for (int i = 0; i < model->rowCount() ; ++i) {
            //model->reset_user_role_data(model->index(i, i_text), UserRoleDef, QVariant(full.dump().c_str()));
            //ToDo: т.к. теперь необходимо передавать всю модель далее код не имеет смысла, нужно менять
            bool is_ = model->index(i, left).data().toString() == old_name;
            if(is_){
                model->setData(model->index(i, left), new_name);
                auto l_details = model->index(i, left).data(UserRoleExt).toStringList();
                if(!l_details.isEmpty()){
                    if(l_details.indexOf(old_name) != -1){
                        l_details[l_details.indexOf(old_name)] = new_name;
//                        model->reset_user_role_data(model->index(i, left), UserRoleExt, QVariant(l_details));
//                        model->reset_user_role_data(model->index(i, i_text), CompareWidgetLeftRole, QVariant(details.dump().c_str()));
                    }
                }
            }
            is_ = model->index(i, right).data().toString() == old_name;
            if(is_){
                model->setData(model->index(i, right), new_name);
                auto l_details = model->index(i, right).data(UserRoleExt).toStringList();
                if(!l_details.isEmpty()){
                    if(l_details.indexOf(old_name) != -1){
                        l_details[l_details.indexOf(old_name)] = new_name;
//                        model->reset_user_role_data(model->index(i, right), UserRoleExt, QVariant(l_details));
//                        model->reset_user_role_data(model->index(i, i_text), CompareWidgetRighttRole, QVariant(details.dump().c_str()));
                    }
                }
            }
            QString query = model->index(i, i_text).data().toString();
            if(!query.isEmpty()){
                if(query.indexOf(old_name + ".") != -1){
                    query = query.replace(old_name, new_name);
                    model->setData(model->index(i, i_text), query);
                }
            }


        }


   // }

}

void QueryBuilderTableInnersWidget::onRemoveTable(const QString &table_name)
{

}

ITreeInnersModel *QueryBuilderTableInnersWidget::getModel()
{
        return (ITreeInnersModel*)treeView->get_model();
}

json QueryBuilderTableInnersWidget::set_fields(const QString &table_name)
{
    if(table_name.isEmpty())
        return {};
    json result = json::object();
    result["table"] = table_name.toStdString();
    json lst = json::array();
    auto itr = tables.find(table_name);
    if(itr != tables.end()){
        auto f = fields.find(itr.value().ref.c_str());
        if(f!=fields.end()){
            foreach (auto it, f.value()) {
                lst += it.alias;
            }
        }
    }
    result["details"] = lst;
    return result;
}

