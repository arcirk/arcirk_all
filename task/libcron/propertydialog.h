#ifndef PROPERTYDIALOG_H
#define PROPERTYDIALOG_H

#include <QDialog>
#include "gui/treeviewwidget.h"

namespace Ui {
class PropertyDialog;
}

using namespace arcirk::tree_widget;
using namespace arcirk::tree_model;

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::plugins), tasks_param,
    (std::string, key)
    (ByteArray, value)
    (int, is_group)
    (std::string, ref)
    (std::string, parent)
)

namespace arcirk::plugins {
    class PropertyDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit PropertyDialog(const json& table, QWidget *parent = nullptr);
        ~PropertyDialog();

        json result() const;

    private:
        Ui::PropertyDialog *ui;
        TreeViewWidget* m_tree;
        TableToolBar* m_toolbar;

    };
}
#endif // PROPERTYDIALOG_H
