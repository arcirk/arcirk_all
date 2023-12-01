#include "dialogmessageslist.h"
#include "ui_dialogmessageslist.h"
#include <QSplitter>
#include <QTreeView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include "userslistwidget.h"
#include "sendmessageboxwidget.h"
#include <QToolBar>
#include <QToolButton>
#include <QLabel>
#include "messageslistwidget.h"
#include "query_builder.hpp"
#include "websocketclient.h"

DialogMessagesList::DialogMessagesList(arcirk::client::client_conf& conf, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogMessagesList),
    conf_(conf)
{
    ui->setupUi(this);

    auto tools = new QToolBar(this);
    tools->setIconSize(QSize{16,16});
    tools->setMovable(true);
    tools->setFloatable(true);
    //tools->setStyleSheet("QToolBar {border-bottom: 1px solid gray; border-top: 1px solid gray;}");
    btnStateDocPane = new QAction("Подразделения", this);
    btnStateDocPane->setIcon(QIcon(":/img/gropes.png"));
    btnStateDocPane->setCheckable(true);
    tools->addSeparator();
    btnSharedChat = new QAction("Общий чат", this);
    //btnStateDocPane->setIcon(QIcon(":/img/gropes.png"));
    //btnSharedChat->setCheckable(true);
    connect(btnStateDocPane, &QAction::toggled, this, &DialogMessagesList::onBtnToggled);
    connect(btnSharedChat, &QAction::triggered, this, &DialogMessagesList::onShowSharedChat);
    tools->addAction(btnStateDocPane);
    tools->addAction(btnSharedChat);
    ui->gridLayout->addWidget(tools);

    auto splitter = new QSplitter(this);
    auto splitter1 = new QSplitter(this);
    splitter1->setOrientation(Qt::Orientation::Vertical);
    dockPane = new QDockWidget(this);
    dockPane->setFloating(false);
    dockPane->setFeatures(QDockWidget::DockWidgetClosable);
    dockPane->setAllowedAreas(Qt::LeftDockWidgetArea);
    connect(dockPane, &QDockWidget::visibilityChanged, this, &DialogMessagesList::onDockPaneHide);
    dockPane->setWindowTitle("Подразделения");
    usr_list = new UsersListWidget(this);
    dockPane->setWidget(usr_list);
    auto msg_box = new QWidget(this);
    m_tab = new QTabWidget(this);
    m_tab->setTabsClosable(true);
    m_tab->addTab(new MessagesListWidget(conf_, this), "Общий чат");
    m_tab->widget(0)->setProperty("uuid", QString(SHARED_CHANNEL_UUID));
    m_tab->widget(0)->setProperty("is_group", 1);
    auto send_box = new SendMessageBoxWidget(this);
    caption = new QLabel(this);

    auto v_box = new QVBoxLayout(this);
    splitter1->addWidget(caption);
    splitter1->addWidget(m_tab);
    splitter1->addWidget(send_box);
    v_box->addWidget(splitter1);
    msg_box->setLayout(v_box);
    splitter->addWidget(dockPane);
    splitter->addWidget(msg_box);
    ui->gridLayout->addWidget(splitter);

    setWindowTitle("Чат");

    connect(usr_list, &UsersListWidget::itemSelect, this, &DialogMessagesList::onSelectUser);
    connect(m_tab, &QTabWidget::tabCloseRequested, this, &DialogMessagesList::onTabCloseRequested);
    connect(m_tab, &QTabWidget::tabBarClicked, this, &DialogMessagesList::tabBarClicked);
    connect(send_box, &SendMessageBoxWidget::sendMessage, this, &DialogMessagesList::doSendMessage);
    connect(this, &DialogMessagesList::chatActivate, send_box, &SendMessageBoxWidget::onChatInit);
    connect(this, &DialogMessagesList::setMessage, send_box, &SendMessageBoxWidget::onSetMessage);
    connect(this, &DialogMessagesList::setMessage, this, &DialogMessagesList::onSetMessage);


}

DialogMessagesList::~DialogMessagesList()
{
    delete ui;
}

//void DialogMessagesList::set_session_id(const QUuid &value)
//{
//    m_session = value;
//}

//void DialogMessagesList::set_channel(std::string &value)
//{
//    current_channel = value;

//}

MessagesListWidget *DialogMessagesList::messagesListWidget(const QString &uuid)
{
    for (int i = 0; i < m_tab->count(); ++i) {
        auto w = m_tab->widget(i);
        QString uuid_ = w->property("uuid").toString();
        if(uuid == uuid_)
            return (MessagesListWidget*)w;
    }

    return nullptr;
}

void DialogMessagesList::onDockPaneHide(bool state)
{
    btnStateDocPane->setChecked(state);
}

void DialogMessagesList::onBtnToggled(bool state)
{
    qDebug() << __FUNCTION__;

    if(state == dockPane->isVisible())
        return;

    dockPane->setVisible(state);

//    if(dockPane->isVisible())
//        dockPane->setVisible(false);
//    else
    //        dockPane->setVisible(true);
}

void DialogMessagesList::onShowSharedChat()
{
    m_tab->setCurrentIndex(0);
}

void DialogMessagesList::tabBarClicked(int index)
{
    qDebug() << __FUNCTION__;

    auto widget = m_tab->widget(index);
    Q_ASSERT(widget!=0);
    auto uuid = widget->property("uuid");
    auto name = widget->property("name");
    Q_ASSERT(uuid.isValid());
    int is_group = widget->property("is_group").toInt();

    emit chatActivate(uuid.toString(), name.toString(), is_group);

}

void DialogMessagesList::onSetMessage(const arcirk::server::server_response &message)
{
    qDebug() << __FUNCTION__;
    auto current_tab = (MessagesListWidget*)m_tab->currentWidget();
    auto uuid = current_tab->property("uuid").toString();
    if(uuid.toStdString() == message.receiver_uuid || uuid.toStdString() == message.sender_uuid){
        auto m = QByteArray::fromBase64(message.message.data());
        //qDebug() << m;
        auto msg = arcirk::secure_serialization<arcirk::database::messages>(QByteArray::fromBase64(message.message.data()).toStdString());
        current_tab->add_message(message.sender_uuid, message.receiver_uuid, msg);
    }
}

void DialogMessagesList::onSelectUser(const json &object)
{
    qDebug() << __FUNCTION__;
    qDebug() << object.dump(4).c_str();

    auto name = object.value("first", "");
    if(!name.empty())
        caption->setText(name.c_str());

    auto uuid = object.value("uuid", "");

    MessagesListWidget * messages = nullptr;
    for (int i = 0; i < m_tab->count(); ++i) {
        auto tab = m_tab->widget(i);
        auto uuid_ = tab->property("uuid").toString();

        if(uuid == uuid_.toStdString()){
            messages = (MessagesListWidget*)tab;
            break;
        }
    }

    if(!messages){
        messages = new MessagesListWidget(conf_, this);
        m_tab->addTab(messages, name.c_str());
        int index = m_tab->count() - 1;
        m_tab->widget(index)->setProperty("uuid", QString::fromStdString(uuid));
        m_tab->widget(index)->setProperty("is_group", object.value("is_group", 0));
        m_tab->widget(index)->setProperty("name", name.c_str());
        m_tab->setCurrentWidget(messages);
    }else{
        m_tab->setCurrentWidget(messages);
    }

    messages->set_recipient_name(name.c_str());
    set_channel(messages->model(), uuid.c_str(), name.c_str(), object.value("is_group", 0));
}

void DialogMessagesList::onTabCloseRequested(int index)
{
    qDebug() << __FUNCTION__ << index;

    if(index > 0){
        m_tab->removeTab(index);
    }
}

void DialogMessagesList::onGetMessages(const arcirk::server::server_response &message)
{
    qDebug() << __FUNCTION__;

////    if(message.result == WS_RESULT_ERROR)
////        return;

//    auto param = json::parse(QByteArray::fromBase64(message.param.data()).toStdString());
//    auto sender = param.value("sender", "");
//    auto recipient = param.value("recipient", "");
////    auto table = json::parse(QByteArray::fromBase64(message.result.data()).toStdString());

//    auto shared_widget = messagesListWidget(recipient.c_str());
//    Q_ASSERT(shared_widget!=0);
////    shared_widget->setTable(table);

//    using namespace arcirk::database;
//    using namespace arcirk::database::builder;

//    auto model = shared_widget->model();
//    auto s_date = sql_compare_value("date", start_date, sql_type_of_comparison::More).to_object();
//    auto s_token = sql_compare_value("token", tokens , sql_type_of_comparison::On_List).to_object();

}

void DialogMessagesList::onUserMessage(const arcirk::server::server_response &message)
{
    qDebug() << __FUNCTION__;
    if(message.result == WS_RESULT_SUCCESS){
        emit setMessage(message);
    }
}

void DialogMessagesList::doSendMessage(const QUuid &recipient, const QString &recipient_name, const QString &message, const json &param)
{
    qDebug() << __FUNCTION__;
    emit sendMessage(recipient, message, param);
}

void DialogMessagesList::onConnectionSuccess(const QUuid &sess, const QUuid &uuid)
{
    using namespace arcirk::database::builder;

    qDebug() << __FUNCTION__;

    m_currentSession = sess;
    m_currentUserUuid = uuid;

    auto shared_token = WebSocketClient::get_sha1(SHARED_CHANNEL_UUID);
    auto widget = messagesListWidget(SHARED_CHANNEL_UUID);
    if(widget!=0){
//        widget->model()->fetchRoot("Messages", "", json{{"token", shared_token.toStdString()}});
//        widget->model()->use_hierarchy("_id");
    }

//    usr_list->init(conf_);
}

void DialogMessagesList::onClientLeave(const arcirk::client::session_info &info)
{
    qDebug() << __FUNCTION__;
    //MessagesListWidget * messages = nullptr;
    for (int i = 0; i < m_tab->count(); ++i) {
        auto tab = m_tab->widget(i);
        auto uuid_ = tab->property("uuid").toString();

        if(info.user_uuid == uuid_.toStdString()){
            //messages = (MessagesListWidget*)tab;
            m_tab->setTabIcon(i, QIcon(":/img/offline.svg"));
            break;
        }
    }
}

void DialogMessagesList::onClientJoin(const arcirk::client::session_info &info)
{
    qDebug() << __FUNCTION__;
    //MessagesListWidget * messages = nullptr;
    for (int i = 0; i < m_tab->count(); ++i) {
        auto tab = m_tab->widget(i);
        auto uuid_ = tab->property("uuid").toString();

        if(info.user_uuid == uuid_.toStdString()){
            //messages = (MessagesListWidget*)tab;
            m_tab->setTabIcon(i, QIcon(":/img/online.svg"));
            break;
        }
    }
}

void DialogMessagesList::onUpdateOnlineUsers(TreeItemModel *model)
{
    for (int i = 0; i < model->rowCount(QModelIndex()); ++i) {
        auto object = arcirk::secure_serialization<client::session_info>(model->to_object(model->index(i,0, QModelIndex())));
        //usr_list
    }
}

void DialogMessagesList::set_channel(TreeItemModel* model, const QString& recipient, const QString& recipient_name, int isGroup){

    qDebug() << __FUNCTION__;

    if(isGroup == 0){
        json query_param = {
            {"first", m_currentUserUuid.toString(QUuid::WithoutBraces).toStdString()},
            {"second", recipient.toStdString()}
        };

        QString token = conf_.hash.c_str();
        QUrl ws(conf_.server_host.data());
        auto resp = WebSocketClient::http_query(ws, token, arcirk::enum_synonym(arcirk::server::server_commands::GetChannelToken), query_param);

        if(resp == WS_RESULT_ERROR)
            return;

        std::string channel_token = resp.get<std::string>();
        if(channel_token.empty())
            return;

//        model->fetchRoot("Messages", "", json{{"token", channel_token}});
//        model->use_hierarchy("_id");

    }else{
        auto shared_token = WebSocketClient::get_sha1(recipient.toUtf8());
//        model->fetchRoot("Messages", "", json{{"token", shared_token.toStdString()}});
//        model->use_hierarchy("_id");
    }

    emit chatActivate(recipient, recipient_name, isGroup);
}
