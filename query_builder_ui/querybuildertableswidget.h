#ifndef QUERYBUILDERTABLESWIDGET_H
#define QUERYBUILDERTABLESWIDGET_H

#include <QWidget>
//#include "shared_struct_qt.hpp"
#include "global/arcirk_qt.hpp"
#include <QUuid>
#include <QVector>
#include "databasestructurewidget.h"
//#include "tree_model.h"
#include "iface/iface.hpp"

using namespace arcirk::tree_model;
using json = nlohmann::json;
using namespace arcirk::database;

namespace Ui {
class QueryBuilderTablesWidget;
}
namespace arcirk::query_builder_ui {
    class QueryBuilderTablesWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit QueryBuilderTablesWidget(DatabaseStructureWidget* structure, QWidget *parent = nullptr, const QUuid& packade_uuid = {});
        ~QueryBuilderTablesWidget();

        ITreeIbaseModel *getModel();

        std::string to_base64();

        void save_to_database();

        int count(){return treeView->get_model()->rowCount();};

    private:
        Ui::QueryBuilderTablesWidget *ui;
        DatabaseStructureWidget * m_structure;
        TreeViewWidget* treeView;
        QUuid m_packade_uuid;

        //void insert_object(const database_structure& table, const QVector<database_structure>& fields);
        void set_data(const ibase_object_structure& table, const QVector<ibase_object_structure>& fields, bool erase = false);

        template<class T>
        ibase_object_structure ibase_object(const T& parent);

        void rename_table_details(const QModelIndex& parent);

    public slots:
        void onSetData(const ibase_object_structure& table, const QVector<ibase_object_structure>& fields);
        void onSetDataEx(const ibase_object_structure& table, const QVector<ibase_object_structure>& fields, const ibase_object_structure& to_field);
        void onStructureDropNode(const QUuid& uuid);
        void doFiledsSetData(const QUuid& uuid);
        void onSetArray(const QVector<ibase_object_structure>& fields);
        void onRemoveRightItems();
        void onRemoveRightItem();
        void onRightSetItem();
        void onRightSetItems();

    signals:
        void objectStructure(const QUuid &uuid);
        void changeTableList(int count);
        void changeTables(const QVector<ibase_object_structure>& tables);
        void renameTable(const QString& old_name, const QString& new_name);
        void removeTable(const QString& table_name);
        void selectField(const ibase_object_structure& table, const QVector<ibase_object_structure>& fields);
        void fieldChanged(const ibase_object_structure& field);
        void dropObject(const QUuid& uuid, const QString& sender);

    private slots:
        void onBtnEditClicked();
        void onBtnNewTempTableClicked();
        void onBtnTableReplaceClicked();
        void onBtnDeleteClicked();


        void onTreeViewDoubleClicked(const QModelIndex &index);
        void onBtnHideColumnClicked(bool checked);
        void onDropEventJson(const json object, const QString& sender);

    };
}
#endif // QUERYBUILDERTABLESWIDGET_H
