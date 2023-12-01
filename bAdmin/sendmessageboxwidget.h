#ifndef SENDMESSAGEBOXWIDGET_H
#define SENDMESSAGEBOXWIDGET_H

#include <QWidget>
#include "texteditorwidget.h"
#include <QPushButton>
#include <QUuid>
#include "websocketclient.h"

using json = nlohmann::json;

namespace Ui {
class SendMessageBoxWidget;
}

class SendMessageBoxWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SendMessageBoxWidget(QWidget *parent = nullptr);
    ~SendMessageBoxWidget();


    void set_recipient(const QUuid& recipient, QString recipient_name){
        recipient_ = recipient;
        recipient_name_ = recipient_name;
    }

private:
    Ui::SendMessageBoxWidget *ui;
    TextEditorWidget * textEditor;
    QPushButton * btnSend;
    QUuid recipient_;
    QString recipient_name_;

signals:
    void sendMessage(const QUuid& recipient, const QString& recipient_name, const QString& message, const json& param);

private slots:
    void onActionSendMessage();
public slots:
    void onChatInit(const QString& uuid, const QString& name, int isGroup);
    void onSetMessage(const arcirk::server::server_response &message);
};

#endif // SENDMESSAGEBOXWIDGET_H
