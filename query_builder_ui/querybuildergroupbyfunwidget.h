#ifndef QUERYBUILDERGROUPBYFUNWIDGET_H
#define QUERYBUILDERGROUPBYFUNWIDGET_H

#include <QWidget>
//#include "shared_struct_qt.hpp"
#include "global/arcirk_qt.hpp"
#include "tree_model.h"
#include "gui/treeviewwidget.h"
#include "iface/iface.hpp"

using namespace arcirk::tree_model;
using namespace arcirk::tree_widget;

namespace Ui {
class QueryBuilderGroupByFunWidget;
}
namespace arcirk::query_builder_ui {
    class QueryBuilderGroupByFunWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit QueryBuilderGroupByFunWidget(QWidget *parent = nullptr, const QUuid& packade_uuid = {});
        ~QueryBuilderGroupByFunWidget();

        void insert_group(const json& object, const QModelIndex& parent = QModelIndex());
        void insert_agregate(const json& object, const QModelIndex& parent = QModelIndex());
        void remove(const QString& uuid);

        json groups_array();
        json agregate_array();

        void save_to_database();

    private:
        Ui::QueryBuilderGroupByFunWidget *ui;
        TreeViewWidget* treeGroupFields;
        TreeViewWidget* treeAggregateFields;
        QUuid m_packade_uuid;

        void verify_fields(ITreeIbaseModel* model, const QList<QString> fields);
    public slots:
        void onSetData(const ibase_object_structure& field);
        void onChangeFieldsList(const QList<QString> fields);
        void onFieldChanged(const ibase_object_structure& field);
        void onRemove(const QUuid& ref);

    private slots:
        void onTreeGropsDropEvent(const json& data, const QString& sender);
        void onTreeGropsAgregateEvent(const json& data, const QString& sender);


    signals:
        void doTreeGropsDropEvent(const json& data);
        void doTreeAgregateDropEvent(const json& data);
    };
}
#endif // QUERYBUILDERGROUPBYFUNWIDGET_H
