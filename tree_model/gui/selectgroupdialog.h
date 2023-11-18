#ifndef SELECTGROUPDIALOG_H
#define SELECTGROUPDIALOG_H

#ifndef IS_OS_ANDROID

#include <QDialog>
#include "tree_model.h"
#include "gui/treeviewwidget.h"

namespace Ui {
class SelectGroupDialog;
}

class SelectGroupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SelectGroupDialog(arcirk::tree_model::TreeItemModel * model, QWidget *parent = nullptr);
    ~SelectGroupDialog();

    void accept() override;

    json result();

private:
    Ui::SelectGroupDialog *ui;
    json m_result;
    arcirk::tree_widget::TreeViewWidget * treeView;
};

#endif
#endif // SELECTGROUPDIALOG_H
