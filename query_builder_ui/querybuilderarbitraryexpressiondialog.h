#ifndef QUERYBUILDERARBITRARYEXPRESSIONDIALOG_H
#define QUERYBUILDERARBITRARYEXPRESSIONDIALOG_H

#include <QDialog>
#include <QTreeView>
//#include "shared_struct_qt.hpp"
#include "global/arcirk_qt.hpp"
#include "tree_model.h"
#include "gui/treeviewwidget.h"
#include "qsourcehighliter.h"
#include "codeeditorwidget.h"

using namespace source_highlite;
using namespace arcirk::tree_model;
using namespace arcirk::tree_widget;
using namespace arcirk::http;

using json = nlohmann::json;

namespace Ui {
class QueryBuilderArbitraryExpressionDialog;
}
namespace arcirk::query_builder_ui {
    class QueryBuilderArbitraryExpressionDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit QueryBuilderArbitraryExpressionDialog(const QVector<ibase_object_structure>& rows,
                                                       const QString& user_function,
                                                       QWidget *parent = nullptr,
                                                       const http_conf& conf = http_conf());
        explicit QueryBuilderArbitraryExpressionDialog(const json& rows,
                                                       const QString& user_function,
                                                       QWidget *parent = nullptr,
                                                       const http_conf& conf = http_conf());
        ~QueryBuilderArbitraryExpressionDialog();

        void accept() override;

        QString query() const{
            return query_;
        }

        void setTableModel(const json& model);


    private slots:
        void onBtnViewFunctionToggled(bool checked);
        void onBtnViewFieldsToggled(bool checked);
        void onBtnTypeToggled(bool checked);
        void onBtnAddClicked();
        void onBtnAddGroupClicked();
        void onBtnEditClicked();
        void onBtnMoveToParentClicked();

    public slots:
        void onSetData(const ibase_object_structure& table, const QVector<ibase_object_structure>& fields);

    private:
        Ui::QueryBuilderArbitraryExpressionDialog *ui;
        QSourceHighliter *highlighter;
        QString query_;
        CodeEditorWidget* textEdit;
        http_conf m_http_conf;

        TreeViewWidget* treeFields;
        TreeViewWidget* treeFunctions;

        void help_model_init();
        void init_connections();
    };
}
#endif // QUERYBUILDERARBITRARYEXPRESSIONDIALOG_H

