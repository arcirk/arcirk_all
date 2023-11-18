#ifndef QUERYBUILDERWHEREWIDGET_H
#define QUERYBUILDERWHEREWIDGET_H

//#include "shared_struct_qt.hpp"
#include "global/arcirk_qt.hpp"
#include <QWidget>
#include "tree_model.h"
#include "gui/treeviewwidget.h"

using namespace arcirk::tree_model;
using namespace arcirk::tree_widget;

namespace Ui {
class QueryBuilderWhereWidget;
}
namespace arcirk::query_builder_ui {
    class QueryBuilderWhereWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit QueryBuilderWhereWidget(QWidget *parent = nullptr, const QUuid& packade_uuid = {});
        ~QueryBuilderWhereWidget();

//        void insert(const json& object, const QModelIndex& parent);
//        void remove(const QString& uuid);

    private:
        Ui::QueryBuilderWhereWidget *ui;
        TreeViewWidget* treeFields;
        TreeViewWidget* treeWhere;
        QUuid m_packade_uuid;

    public slots:
//        void onSetData(const QVector<ibase_object_structure>& fields);
//        void onFieldChanged(const ibase_object_structure& field);
//        void onRemove(const QUuid& ref);
        void onTablesChanged(const QVector<ibase_object_structure>& rows);

    private slots:
        void onTreeViewDoubleClicked(const QModelIndex &index);
        void onTreeDropEvent(const json& data, const QString& sender);
        void onBtnTypeToggled(bool checked);

    signals:
        void selectData(const ibase_object_structure& field);
        void doTreeDropEvent(const json& data);
        void fieldChanged(const ibase_object_structure& field);
        void remove_(const QUuid& ref);
    };
}
#endif // QUERYBUILDERWHEREWIDGET_H
