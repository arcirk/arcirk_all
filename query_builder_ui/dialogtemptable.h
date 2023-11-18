#ifndef DIALOGTEMPTABLE_H
#define DIALOGTEMPTABLE_H

#include <QDialog>
//#include "shared_struct_qt.hpp"
#include "global/arcirk_qt.hpp"
#include <QValidator>
//#include "querybuildertableswidget.h"
//#include "tree_model.h"
#include "gui/treeviewwidget.h"

using namespace arcirk::tree_model;
using namespace arcirk::tree_widget;
using json = nlohmann::json;
//using namespace arcirk::database;

namespace Ui {
class DialogTempTable;
}

namespace arcirk::query_builder_ui {
    class DialogTempTable : public QDialog
    {
        Q_OBJECT

    public:
        explicit DialogTempTable(ibase_object_structure& root, QVector<ibase_object_structure>& details, QWidget *parent = nullptr);
        ~DialogTempTable();

        void accept() override;

    private slots:
        void onBtnAddClicked();
        void onTxtNameTextChanged(const QString &arg1);
        void onTxtAliasTextChanged(const QString &arg1);
        void onBtnAddCopyClicked();
        void onBtnDeleteClicked();

    private:
        Ui::DialogTempTable *ui;
        ibase_object_structure& root_;
        QVector<ibase_object_structure>& details_;
        TreeViewWidget* treeView;


    };
}
#endif // DIALOGTEMPTABLE_H
