#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include "gui/treeviewwidget.h"
#include "codeeditorwidget.h"
#include "qsourcehighliter.h"
#include "gui/tabletoolbar.h"
#include "QLabel"
#include <QSqlDatabase>
#include <QAction>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

using namespace source_highlite;
using namespace arcirk::tree_model;
using namespace arcirk::tree_widget;
using namespace arcirk::query_builder_ui;

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::query_builder_ui), query_builder_main,
    (int, _id)
    (std::string, name)
    (std::string, ref)
    (std::string, path)
    (std::string, parent)
    (std::string, data_type)
    (int, is_group)
)

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::query_builder_ui), query_builder_querias,
    (int, _id)
    (std::string, ref)
    (std::string, parent)
    (ByteArray, data)
    (std::string, query_ref)
    (int, is_group)
)

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onMnuOpenDatabaseTriggered();
    void mnMnuClose();
    //void onDialogRowChangedData(const json data, bool is_new = false);

private:
    Ui::MainWindow *ui;

    TreeViewWidget * m_treeQueryas;
    TreeViewWidget * m_treeQuery;
    CodeEditorWidget * m_codeEdit;
    TableToolBar* m_queryasToolbar;
    QLabel * m_statusBarText;

    QSqlDatabase m_connection;

    void initDatabase();
};

#endif // MAINWINDOW_H
