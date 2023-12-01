#include "userslistwidget.h"
#include "ui_userslistwidget.h"
#include <QSplitter>
//#include "tree/treeitemmodel.h"
//#include "query_builder/ui/query_builder.hpp"
#include <QVector>

using namespace arcirk::tree_model;

UsersListWidget::UsersListWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UsersListWidget)
{
    ui->setupUi(this);

    treeView = new TreeViewWidget(this);
}

UsersListWidget::~UsersListWidget()
{
    delete ui;
}

void UsersListWidget::init(const arcirk::http::http_conf &conf)
{
    //using namespace arcirk::database::builder;

    QMap<QString, QString>  m_aliases;
    m_aliases.insert("first", "Наменование");
    auto model = new TreeItemModel(this);
    //model->use_http(true);
    auto order = QVector<QString>{
        "first"
    };
    model->set_fetch_expand(true);
    model->set_column_aliases(m_aliases);
    model->set_columns_order(order);
    model->set_connection(root_tree_conf::httpConnection, conf);
    //model->set_http_conf(QUrl(conf.server_host.c_str()), conf.hash.c_str(), "RegisteredUsersView");



//    sql_values vals{sql_value("*", "", ""),
//                sql_value("CAST(0 AS INTEGER)", "is_online", ""), //в сети не всети
//                sql_value("CAST(0 AS INTEGER)", "unread", "")}; //количество не прочитанных сообщений
//    model->set_sql_columns(vals);
//    model->fetchRoot("RegisteredUsersView");

    //model->use_hierarchy("first");
    //model->fetchMore(QModelIndex());

   treeView->setModel(model);
    foreach (auto const& itr, model->columnNames()) {
        auto index = model->column_index(itr);
        if(itr != "first" )
            treeView->hideColumn(index);
    }
}


void UsersListWidget::on_treeView_clicked(const QModelIndex &index)
{
//    if(!index.isValid())
//        return;

//    auto model = (TreeItemModel*)ui->treeView->model();
//    auto object = model->get_object(index);

//    emit itemSelect(object);
}


void UsersListWidget::on_treeView_doubleClicked(const QModelIndex &index)
{
    qDebug() << __FUNCTION__;

    if(!index.isValid())
        return;

    auto model = treeView->get_model();
    auto object = model->to_object(treeView->get_index(index));
    //qDebug() << object.dump(4).c_str();
    emit itemSelect(object);
}

