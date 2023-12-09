#ifndef DIALOGREMOTEUSERS_H
#define DIALOGREMOTEUSERS_H

#include <QDialog>
#include "gui/treeviewwidget.h"

using namespace arcirk::tree_model;
using namespace arcirk::tree_widget;

namespace Ui {
class DialogRemoteUsers;
}

class DialogRemoteUsers : public QDialog
{
    Q_OBJECT

public:
    explicit DialogRemoteUsers(const json& jsonModel, QWidget *parent = nullptr);
    ~DialogRemoteUsers();

    void accept() override;
    int seletedId();
    bool management();

private slots:
    void on_btnSetOnlineOnly_toggled(bool checked);
    void on_chkMnanager_toggled(bool checked);

private:
    Ui::DialogRemoteUsers *ui;
    TreeViewWidget* treeView;
    //TreeItemModel* model;
    int  m_result;
    bool m_management;
    QMap<QString, QString> m_colAliases;

    void createColumnAliases();

    void updateIcons();


};

#endif // DIALOGREMOTEUSERS_H
