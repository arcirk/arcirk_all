#ifndef DIALOGMESSAGESLIST_H
#define DIALOGMESSAGESLIST_H

#include <QDialog>
#include <QDockWidget>
#include <QAction>
#include "shared_struct.hpp"
#include "userslistwidget.h"
#include <QLabel>
#include <QTabWidget>
#include <QUuid>
#include "messageslistwidget.h"

using json = nlohmann::json;

namespace Ui {
class DialogMessagesList;
}

class DialogMessagesList : public QDialog
{
    Q_OBJECT

public:
    explicit DialogMessagesList(arcirk::client::client_conf& conf, QWidget *parent = nullptr);
    ~DialogMessagesList();

    //void set_session_id(const QUuid& value);

    //void set_channel(std::string& value);

private:
    Ui::DialogMessagesList *ui;

    QAction * btnStateDocPane;
    QAction * btnSharedChat;
    QDockWidget * dockPane;
    UsersListWidget * usr_list;
    QLabel * caption;
    QTabWidget * m_tab;

    QUuid m_currentSession;
    QUuid m_currentUserUuid;

    std::string current_channel;

    arcirk::client::client_conf& conf_;

    MessagesListWidget *messagesListWidget(const QString& uuid);
    void set_channel(TreeItemModel* model, const QString& recipient, const QString& recipient_name, int isGroup);

private slots:
    void onDockPaneHide(bool state);
    void onBtnToggled(bool state);
    void onShowSharedChat();
    void tabBarClicked(int index);
    void onSetMessage(const arcirk::server::server_response &message);

public slots:
    void onSelectUser(const json& object);
    void onTabCloseRequested(int index);
    void onGetMessages(const arcirk::server::server_response  &message);
    void onUserMessage(const arcirk::server::server_response  &message);
    void doSendMessage(const QUuid& recipient, const QString &recipient_name, const QString& message, const json& param);
    void onConnectionSuccess(const QUuid& sess, const QUuid& uuid);
    void onClientLeave(const arcirk::client::session_info& info);
    void onClientJoin(const arcirk::client::session_info& info);
    void onUpdateOnlineUsers(TreeItemModel* model);
signals:
    void sendMessage(const QUuid& recipient, const QString& message, const json& param);
    void getMessages(const nlohmann::json& param);
    void chatActivate(const QString& uuid, const QString& name, int isGroup);
    void setMessage(const arcirk::server::server_response &message);

};

#endif // DIALOGMESSAGESLIST_H
