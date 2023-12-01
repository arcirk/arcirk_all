#include "sendmessageboxwidget.h"
#include "ui_sendmessageboxwidget.h"
#include <QHBoxLayout>
#include "shared_struct.hpp"

SendMessageBoxWidget::SendMessageBoxWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SendMessageBoxWidget)
{
    ui->setupUi(this);

    textEditor = new TextEditorWidget(this);
    ui->verticalLayout->addWidget(textEditor);
    auto h = new QHBoxLayout(this);
    ui->verticalLayout->addLayout(h);
    btnSend = new QPushButton("Отправить", this);
    btnSend->setMaximumWidth(100);
    h->addWidget(btnSend, Qt::AlignRight);
    h->setAlignment(Qt::AlignRight);

    connect(btnSend, &QPushButton::clicked, this, &SendMessageBoxWidget::onActionSendMessage);

    recipient_ = QUuid::fromString(SHARED_CHANNEL_UUID);
}

SendMessageBoxWidget::~SendMessageBoxWidget()
{
    delete ui;
}

void SendMessageBoxWidget::onActionSendMessage()
{
    json param{
        {"content_type", arcirk::enum_synonym(arcirk::database::text_type::dbHtmlText)}
    };

    QString text = textEditor->text();

    if(text.isEmpty())
        return;

    emit sendMessage(recipient_, recipient_name_, text, param);
}

void SendMessageBoxWidget::onChatInit(const QString &uuid, const QString &name, int isGroup)
{
    Q_UNUSED(isGroup);
    set_recipient(QUuid::fromString(uuid), name);
}

void SendMessageBoxWidget::onSetMessage(const arcirk::server::server_response &message)
{
    if(message.result == WS_RESULT_SUCCESS){
        textEditor->setText("");
    }
}
