#ifndef QUERYBUILDERPACKAGEDIALOG_H
#define QUERYBUILDERPACKAGEDIALOG_H

#include <QDialog>
#include "query_builder.hpp"
//#include "shared_struct_qt.hpp"
//#include "global/arcirk_qt.hpp""
//#include "tree_model.h"
#include "websocketclient.h"
#include <QSqlDatabase>
#include "gui/treeviewwidget.h"
//#include "sql/query_builder.hpp"


using namespace arcirk::tree_model;
using namespace arcirk::tree_widget;
//using namespace arcirk::database;

namespace Ui {
class QueryBuilderPackageDialog;
}
namespace arcirk::query_builder_ui {
    class QueryBuilderPackageDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit QueryBuilderPackageDialog(WebSocketClient* client, QWidget *parent = nullptr, const QString& query_text = "");
        ~QueryBuilderPackageDialog();

        void accept() override;
        QString result() const;

    private slots:
        void onBtnAddClicked();
        void onBtnDeleteClicked();
        void onBtnEditClicked();
        void initDatabase();
        void onBtnQueryClicked();
        void onOpenDataBaseClicked();
        void onSaveDataBaseClicked();

    public slots:
        void reset();

    private:
        Ui::QueryBuilderPackageDialog *ui;
        WebSocketClient* m_client;
        QSqlDatabase m_connection;
        QString m_result;
        TreeViewWidget* treeView;
        QFile m_database_file;

        void generate_query();
        QString query_text(const std::string& ref);
        void parse(const QString& query_text);
        int parse_values(int position, const QList<QString> querias, const QString end, arcirk::database::builder::query_builder& query);

        QString generate_text_create_table(const query_builder_packet& package);

    };
}
#endif // QUERYBUILDERPACKAGEDIALOG_H
