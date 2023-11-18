#ifndef DATABASESTRUCTUREWIDGET_H
#define DATABASESTRUCTUREWIDGET_H

#include <QWidget>

#include "global/arcirk_qt.hpp"
#include "websocketclient.h"
#include <QUuid>
#include "tree_model.h"
#include "gui/treeviewwidget.h"
#include "iface/iface.hpp"

using namespace arcirk::database;
using namespace arcirk::tree_model;
using namespace arcirk::tree_widget;

namespace Ui {
class DatabaseStructureWidget;
}
namespace arcirk::query_builder_ui {
    class DatabaseStructureWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit DatabaseStructureWidget(WebSocketClient* client, QWidget *parent = nullptr, const QUuid& packade_uuid = {});
        ~DatabaseStructureWidget();

        json objectStructure(const QUuid& uuid) const;

        ITree<ibase_object_structure>* model(){
            return (ITree<ibase_object_structure>*)treeView->get_model();
        }

    private:
        Ui::DatabaseStructureWidget *ui;
        WebSocketClient* m_client;
        TreeViewWidget* treeView;
        QUuid m_packade_uuid;

        void load_structure();

    signals:
        void selectRow(const ibase_object_structure& table, const QVector<ibase_object_structure>& fields);
        void selectRows(const QVector<ibase_object_structure>& fields);
        void selectItem(const ibase_object_structure& table, const QVector<ibase_object_structure>& fields, const ibase_object_structure& to_field);
        void dropNode(const QUuid& uuid);
        void removeRightItems();
        void removeRightItem();

    private slots:
        void onTreeViewOnDoubleClick(const QModelIndex& index);
        void onBtnTypeToggled(bool checked);

        void onBtnSetItemClicked();
        void onBtnSetItemsClicked();
        void onBtnRemoveItemClicked();
        void onBtnRemoveItemsClicked();

    public slots:
        void doObjectStructure(const QUuid& uuid);
        void onDropEvent(const json object, const QString& sender);

    };
}
#endif // DATABASESTRUCTUREWIDGET_H
