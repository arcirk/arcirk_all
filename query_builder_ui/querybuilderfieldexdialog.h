#ifndef QUERYBUILDERFIELDEXDIALOG_H
#define QUERYBUILDERFIELDEXDIALOG_H


#include <QDialog>
#include "gui/treeviewwidget.h"
#include <QMenu>
#include <QAction>
#include "iface/iface.hpp"
#include "query_builder.hpp"

using namespace arcirk::database::builder;
using namespace arcirk::tree_model;
using namespace arcirk::tree_widget;

namespace Ui {
class QueryBuilderFieldExDialog;
}

namespace arcirk::query_builder_ui {
class QueryBuilderFieldExDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QueryBuilderFieldExDialog(QWidget *parent = nullptr, const QUuid& packade_uuid = {});
    ~QueryBuilderFieldExDialog();

    void set_database_structure(ITree<ibase_object_structure>* structurte);

    ibase_object_structure getField();
    void setData(const ibase_object_structure& value);

private:
    Ui::QueryBuilderFieldExDialog *ui;
    TreeViewWidget* treeView;
    QMenu* mnu_index_type;
    QAction * m_compare;
    QAction * m_ext;
    QAction * m_check;
    ITree<ibase_object_structure>* m_structurte;
    QStringList m_tables;
    QUuid m_packade_uuid;
    ibase_object_structure m_field;


    std::string get_query(const sql_foreign_key& value);

private slots:
    void onMenuClicked();   
    void onBtnEditClicked();
    void onBtnDeleteClicked();

signals:

};
}
#endif // QUERYBUILDERFIELDEXDIALOG_H
