#ifndef MESSAGESLISTWIDGET_H
#define MESSAGESLISTWIDGET_H

#include <QWidget>
#include "tree_model.h"
#include "shared_struct.hpp"

using namespace arcirk::tree_model;

using json = nlohmann::json;

namespace Ui {
class MessagesListWidget;
}

class MessagesListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MessagesListWidget(arcirk::client::client_conf& conf, QWidget *parent = nullptr);
    ~MessagesListWidget();

    TreeItemModel *model();
    void setTable(const json& tableModel);

    void start_read_messages(const std::string &channel_token);

    void add_message(const std::string& first, const std::string& second, const arcirk::database::messages message);

    void set_recipient_name(const QString& value);
    QString recipient_name() const;
private:
    Ui::MessagesListWidget *ui;
    arcirk::client::client_conf& conf_;
    QString recipient_name_;

public slots:
    void fetch(const QModelIndex& parent);
};

#endif // MESSAGESLISTWIDGET_H
