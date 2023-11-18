#ifndef QUERYEDITORADDGROUPDIALOG_H
#define QUERYEDITORADDGROUPDIALOG_H

#include <QDialog>
//#include "shared_struct_qt.hpp"
#include "global/arcirk_qt.hpp"

using namespace arcirk::database;

namespace Ui {
class QueryEditorAddGroupDialog;
}

namespace arcirk::query_builder_ui {

    class QueryEditorAddGroupDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit QueryEditorAddGroupDialog(sqlite_functions_info& info, const QString& parent_name, QWidget *parent = nullptr);
        ~QueryEditorAddGroupDialog();

        void accept() override;

    private:
        Ui::QueryEditorAddGroupDialog *ui;
        sqlite_functions_info& info_;
    };
}
#endif // QUERYEDITORADDGROUPDIALOG_H
