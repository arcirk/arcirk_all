#ifndef PLUGINPROPERTYDIALOG_H
#define PLUGINPROPERTYDIALOG_H

#include <QDialog>
//#include <QJsonObject>
#include "gui/treeviewwidget.h"

namespace Ui {
class PluginPropertyDialog;
}

using namespace arcirk::tree_widget;
using namespace arcirk::tree_model;


namespace arcirk::plugins {
    class PluginPropertyDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit PluginPropertyDialog(const json& table, QWidget *parent = nullptr);
        ~PluginPropertyDialog();

        json result() const;

    private:
        Ui::PluginPropertyDialog *ui;
        TreeViewWidget* m_tree;
        TableToolBar* m_toolbar;

    };
}
#endif // PLUGINPROPERTYDIALOG_H
