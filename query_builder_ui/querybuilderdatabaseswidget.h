#ifndef QUERYBUILDERDATABASESWIDGET_H
#define QUERYBUILDERDATABASESWIDGET_H

#include <QWidget>
#include "iface/iface.hpp"
#include "gui/treeviewwidget.h"
#include "gui/tabletoolbar.h"
#include <QSqlDatabase>

using namespace arcirk::tree_model;
using namespace arcirk::tree_widget;
using namespace arcirk::query_builder_ui;

namespace Ui {
class QueryBuilderDatabasesWidget;
}

namespace arcirk::query_builder_ui {
    class QueryBuilderDatabasesWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit QueryBuilderDatabasesWidget(QSqlDatabase& connection, QWidget *parent = nullptr);
        ~QueryBuilderDatabasesWidget();

    private:
        Ui::QueryBuilderDatabasesWidget *ui;
        TableToolBar* m_databasesToolbar;
        TreeViewWidget * m_treeDatabases;
        QSqlDatabase& m_connection;

        void updateIcons(const QModelIndex& parent);
        void setData();

    private slots:
        void onToolbarDatabasesClicked(const QString& bottonName);
        void onTreeItemClicked(const QModelIndex &current);
        void onAddTreeItem(const QModelIndex& index, const json& data);
        void onEditTreeItem(const QModelIndex& index, const json& data);
        void onDeleteTreeItem(const json& data);


    };
}
#endif // QUERYBUILDERDATABASESWIDGET_H
