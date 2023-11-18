#ifndef QUERYBUILDERFIELDSWIDGET_H
#define QUERYBUILDERFIELDSWIDGET_H

#include <QWidget>
#include "global/arcirk_qt.hpp"
//#include "tree_model.h"
#include "gui/treeviewwidget.h"

namespace Ui {
class QueryBuilderFieldsWidget;
}
using namespace arcirk::tree_model;
using namespace arcirk::tree_widget;
using namespace arcirk::http;

namespace arcirk::query_builder_ui {
    class QueryBuilderFieldsWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit QueryBuilderFieldsWidget(QWidget *parent = nullptr, const QUuid& packade_uuid = {});
        ~QueryBuilderFieldsWidget();

        void set_http_conf(const http_conf& value){
            m_http_conf = value;};

        QVector<ibase_object_structure> array();

        void setBuilderArbitraryExpressionResult(const QString& query, const QString& ref);

        void save_to_database();

    private slots:
        void onBtnAddClicked();
        void onBtnDeleteClicked();
        void onBtnEditClicked();
        void onTreeViewDoubleClicked(const QModelIndex &index);
        void onUpdateIcons();
        void onDropEventJson(const json object, const QString& sender);

        void onBtnLeftSelectItemClicked();
        void onBtnLeftSelectItemsClicked();
        void onBtnRemoveItemClicked();
        void onBtnRemoveItemsClicked();

    public slots:
        void onSetData(const ibase_object_structure& table, const QVector<ibase_object_structure>& fields);
        void onChangeTablesList(const QVector<ibase_object_structure>& tables);
        void onFieldChanged(const ibase_object_structure& field);
        void onDragResult(const QUuid& uuid, const QString& sender);

    private:
        Ui::QueryBuilderFieldsWidget *ui;
        http_conf m_http_conf;
        TreeViewWidget* treeView;
        QUuid m_packade_uuid;

    signals:
        void updateIcons();
        void changeListFields(const QVector<ibase_object_structure>& fields);
        void changeFields(const QList<QString> parents);
        void fieldChanged(const ibase_object_structure& field);
        void openUserQueryDialog(bool selected_fields, const QString &current_function, QWidget* parent, const QString& ref );
        void remove(const QUuid& ref);
        void doSetData(const QUuid& uuid);
        void selectItem();
        void selectItems();
    };
}


#endif // QUERYBUILDERFIELDSWIDGET_H
