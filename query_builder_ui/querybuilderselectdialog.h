#ifndef QUERYBUILDERSELECTDIALOG_H
#define QUERYBUILDERSELECTDIALOG_H

#include <QDialog>
//#include "shared_struct_qt.hpp"
#include "global/arcirk_qt.hpp"
#include "tree_model.h"

using namespace arcirk::tree_model;

namespace Ui {
    class QueryBuilderSelectDialog;
    }

    namespace arcirk::query_builder_ui {
    class QueryBuilderSelectDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit QueryBuilderSelectDialog(TreeItemModel* model, bool allowSelGroup, QWidget *parent = nullptr);
        ~QueryBuilderSelectDialog();

        void accept() override;

        json selectedObject() const;

    private:
        Ui::QueryBuilderSelectDialog *ui;
        bool sel_group;
        json object;
    public slots:
        void onTreeViewSelect(const QModelIndex& index);
    };
}
#endif // QUERYBUILDERSELECTDIALOG_H
