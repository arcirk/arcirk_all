#ifndef QUERYBUILDERDATABASESWIDGET_H
#define QUERYBUILDERDATABASESWIDGET_H

#include <QWidget>
#include "global/arcirk_qt.hpp"
#include "iface/iface.hpp"
#include "gui/treeviewwidget.h"
#include "gui/tabletoolbar.h"
#include <QSqlDatabase>
#include "query_parser.h"

using namespace arcirk::tree_model;
using namespace arcirk::tree_widget;
using namespace arcirk::query_builder_ui;
using namespace arcirk::database::builder;

//typedef std::map<std::string, tree_model::ibase_object_structure> details_t;
//typedef std::map<std::string, details_t> object_t;
//typedef std::pair<std::string, tree_model::ibase_object_structure> details_pair;
//typedef std::pair<std::string, details_t> object_pair;

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

        QList<QPair<QVariant, QVariant>> databasesList() const;
        json database_structure(const QUuid& ref) const;
        object_t database_structure_t(const QUuid& ref) const;

    private:
        Ui::QueryBuilderDatabasesWidget *ui;
        TableToolBar* m_databasesToolbar;
        TreeViewWidget * m_treeDatabases;
        QSqlDatabase& m_connection;
        QMap<QUuid, object_t> m_database_t;

        void updateIcons(const QModelIndex& parent);
        void setData();

    private slots:
        void onToolbarDatabasesClicked(const QString& bottonName);
        void onTreeItemClicked(const QModelIndex &current);
        void onAddTreeItem(const QModelIndex& index, const json& data);
        void onEditTreeItem(const QModelIndex& index, const json& data);
        void onDeleteTreeItem(const json& data);

    signals:
        void databaseListChanged();
        void startDatabaseScanned();
        void endDatabaseScanned();
    };
}
#endif // QUERYBUILDERDATABASESWIDGET_H
