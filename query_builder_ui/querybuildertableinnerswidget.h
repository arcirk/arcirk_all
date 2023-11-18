#ifndef QUERYBUILDERTABLEINNERSWIDGET_H
#define QUERYBUILDERTABLEINNERSWIDGET_H

//#include "shared_struct_qt.hpp"
#include "global/arcirk_qt.hpp"
#include <QWidget>
#include <QComboBox>
#include "querybuildertableswidget.h"
//#include "tree_model.h"
#include "iface/iface.hpp"
#include "gui/treeviewwidget.h"

namespace Ui {
class QueryBuilderTableInnersWidget;
}
namespace arcirk::query_builder_ui {

    class QueryBuilderTableInnersWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit QueryBuilderTableInnersWidget(QueryBuilderTablesWidget* query_tables, QWidget *parent = nullptr, const QUuid& packade_uuid = {});
        ~QueryBuilderTableInnersWidget();

        void save_to_database();

    private slots:
        void onBtnAddClicked();


    public slots:
        void onChangeTablesList();
        void onSelectedItemComboBoxChanged(int row, int col, const QString& value, int index);
        void onRenameTable(const QString& old_name, const QString& new_name);
        void onRemoveTable(const QString& table_name);


    private:
        Ui::QueryBuilderTableInnersWidget *ui;
        QueryBuilderTablesWidget* m_query_tables;
        QStringList current_tables;
        QMap<QString, QVector<ibase_object_structure>> fields;
        QMap<QString, ibase_object_structure> tables;
        QUuid m_packade_uuid;
        TreeViewWidget* treeView;

        ITreeInnersModel* getModel();
        json set_fields(const QString& table_name);
    };

}
#endif // QUERYBUILDERTABLEINNERSWIDGET_H
