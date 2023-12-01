#include "messageslistwidget.h"
#include "ui_messageslistwidget.h"
#include "messageitemdelegate.h"

MessagesListWidget::MessagesListWidget(arcirk::client::client_conf& conf, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MessagesListWidget),
    conf_(conf)
{
    ui->setupUi(this);

    ui->treeView->setModel(new TreeItemModel(this));
    ui->treeView->setItemDelegate(new MessageItemDelegate(ui->treeView));

    //auto model = (TreeItemModel*)ui->treeView->model();
    //connect(model, &TreeItemModel::fetch, this, &MessagesListWidget::fetch);

    ui->treeView->setStyleSheet("background-color: #424242;");
    ui->treeView->setHeaderHidden(true);

}

MessagesListWidget::~MessagesListWidget()
{
    delete ui;
}

TreeItemModel *MessagesListWidget::model()
{
    return (TreeItemModel*)ui->treeView->model();

}

void MessagesListWidget::setTable(const json &tableModel)
{
//    model()->set_table(tableModel);
//    for (int i = 1; i < model()->columnCount(QModelIndex()); ++i) {
//        ui->treeView->hideColumn(i);
//    }
}

void MessagesListWidget::start_read_messages(const std::string &channel_token)
{
//    model()->fetchRoot("Messages", NIL_STRING_UUID, json{
//                           {"token", channel_token}
    //                       });
}

void MessagesListWidget::add_message(const std::string &first, const std::string &second, const arcirk::database::messages message)
{
//    qDebug() << __FUNCTION__;
//    auto model = (TreeItemModel*)ui->treeView->model();
//    auto index = model->find(toQtUuid(message.parent), QModelIndex());
//    if(!index.isValid()){
//        auto parent = arcirk::database::table_default_struct<arcirk::database::messages>(arcirk::database::tables::tbMessages);
//        parent.date = arcirk::start_day(arcirk::current_date());
//        parent.ref = message.parent;
//        model->add(pre::json::to_json(parent));
//        index = model->find(toQtUuid(message.parent), QModelIndex());
//    }
//    if(index.isValid()){
//        auto grp = arcirk::secure_serialization<arcirk::database::messages>(model->to_object(index));
//        //qDebug() << model->get_object(index).dump(4).c_str();
//        model->add(pre::json::to_json(message), index);
//        //const NodeInfoList& parentInfoChildren = nodeInfo->parent != 0 ? nodeInfo->parent->children: _nodes;
//    }
//    //ui->treeView->setCurrentIndex(model->index(model->rowCount(QModelIndex()) - 1, 0, QModelIndex()));
}

void MessagesListWidget::set_recipient_name(const QString &value)
{
    recipient_name_ = value;
    ui->treeView->model()->setProperty("recipient", recipient_name_);
}

QString MessagesListWidget::recipient_name() const
{
    return recipient_name_;
}

void MessagesListWidget::fetch(const QModelIndex& parent)
{
    if(!parent.isValid() && ui->treeView->model()->columnCount() > 0){
        auto model = (TreeItemModel*)ui->treeView->model();
    //    ui->treeView->setCurrentIndex(model->index(model->rowCount(QModelIndex()) - 1, 0, QModelIndex()));
        for (int i = 1; i < model->columnCount(QModelIndex()); ++i) {
            ui->treeView->hideColumn(i);
        }
    }

}
