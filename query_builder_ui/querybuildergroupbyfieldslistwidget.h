#ifndef QUERYBUILDERGROUPBYFIELDSLISTWIDGET_H
#define QUERYBUILDERGROUPBYFIELDSLISTWIDGET_H

//#include "shared_struct_qt.hpp"
#include "global/arcirk_qt.hpp"
#include <QWidget>
#include "tree_model.h"
#include "gui/treeviewwidget.h"
#include "iface/iface.hpp"

using namespace arcirk::tree_model;
using namespace arcirk::tree_widget;

namespace Ui {
class QueryBuilderGroupByFieldsListWidget;
}
namespace arcirk::query_builder_ui {
    class QueryBuilderGroupByFieldsListWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit QueryBuilderGroupByFieldsListWidget(QWidget *parent = nullptr, const QUuid& packade_uuid = {});
        ~QueryBuilderGroupByFieldsListWidget();

        void insert(const json& object, const QModelIndex& parent);
        void remove(const QString& uuid);

    public slots:
        void onSetData(const QVector<ibase_object_structure>& fields);
        void onSetFilter(const QStringList& lst);
        void onFieldChanged(const ibase_object_structure& field);
        void onRemove(const QUuid& ref);

    private slots:
        void onTreeViewDoubleClicked(const QModelIndex &index);
        void onTreeDropEvent(const json& data, const QString& sender);
        void onBtnTypeToggled(bool checked);

    private:
        Ui::QueryBuilderGroupByFieldsListWidget *ui;
        TreeViewWidget* treeView;
        QUuid m_packade_uuid;

        ITreeIbaseModel* getModel();
        QModelIndex get_index(const QModelIndex& proxy_index);
    signals:
        void selectData(const ibase_object_structure& field);
        void doTreeDropEvent(const json& data);
        void fieldChanged(const ibase_object_structure& field);
        void remove_(const QUuid& ref);
    };
}
#endif // QUERYBUILDERGROUPBYFIELDSLISTWIDGET_H
