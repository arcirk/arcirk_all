#ifndef USERSLISTWIDGET_H
#define USERSLISTWIDGET_H

#include <QWidget>
//#include "shared_struct.hpp"
//#include "tree_model.h"
#include "global/arcirk_qt.hpp"
#include "gui/treeviewwidget.h"

//using json = nlohmann::json;

using namespace arcirk::tree_model;
using namespace arcirk::tree_widget;

namespace Ui {
class UsersListWidget;
}

class UsersListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit UsersListWidget(QWidget *parent = nullptr);
    ~UsersListWidget();

    void init(const arcirk::http::http_conf& conf);

private:
    Ui::UsersListWidget *ui;
    TreeViewWidget* treeView;

signals:
    void itemSelect(const json& object);
private slots:
    void on_treeView_clicked(const QModelIndex &index);
    void on_treeView_doubleClicked(const QModelIndex &index);
};

#endif // USERSLISTWIDGET_H
