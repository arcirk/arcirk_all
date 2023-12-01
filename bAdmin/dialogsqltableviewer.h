#ifndef DIALOGSQLTABLEVIEWER_H
#define DIALOGSQLTABLEVIEWER_H

#include <QDialog>
#include "websocketclient.h"
#include "shared_struct.hpp"
#include "global/arcirk_qt.hpp"

using json = nlohmann::json;
using namespace arcirk;

namespace Ui {
class DialogSqlTableViewer;
}

class DialogSqlTableViewer : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSqlTableViewer(database::tables table, WebSocketClient* client, QWidget *parent = nullptr);
    ~DialogSqlTableViewer();

private slots:
    void on_btnTreeViewSettings_clicked();

    void on_btnMoveNext_clicked();

    void on_btnMoveLast_clicked(bool checked);

    void on_btnMovePrevious_clicked();

    void on_btnMoveFirst_clicked();

private:
    Ui::DialogSqlTableViewer *ui;
    database::tables m_table;
    WebSocketClient* m_client;
    int m_start_id;
    int m_max_rows;
    int row_count;

    void reload_model();
    void update_navigation_widget();
};

#endif // DIALOGSQLTABLEVIEWER_H
