#ifndef QUERYBUILDESQLITESUPPORTITEMDIALOG_H
#define QUERYBUILDESQLITESUPPORTITEMDIALOG_H

#include <QDialog>
//#include "shared_struct_qt.hpp"
#include "global/arcirk_qt.hpp"

using namespace arcirk::database;
namespace Ui {
class QueryBuildeSqliteSupportItemDialog;
}

namespace arcirk::query_builder_ui  {
    class QueryBuildeSqliteSupportItemDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit QueryBuildeSqliteSupportItemDialog(sqlite_functions_info& info, const QString& parent_name, QWidget *parent = nullptr);
        ~QueryBuildeSqliteSupportItemDialog();

        void accept() override;

    private slots:
        void onBtnToUpperClicked();

    private:
        Ui::QueryBuildeSqliteSupportItemDialog *ui;
        sqlite_functions_info& info_;
    };
}


#endif // QUERYBUILDESQLITESUPPORTITEMDIALOG_H
