#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include "gui/treeviewwidget.h"
#include "codeeditorwidget.h"
#include "gui/tabletoolbar.h"
#include "QLabel"
#include <QSqlDatabase>
#include <QAction>
#include "querybuilderdatabaseswidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


using namespace arcirk::tree_model;
using namespace arcirk::tree_widget;
using namespace arcirk::query_builder_ui;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onMnuOpenDatabaseTriggered();
    void mnMnuClose();
    void onTreeQueryasRowSelected(const QModelIndex& index);
    void onAddTreeItem(const QModelIndex& index, const json& data);
    void onEditTreeItem(const QModelIndex& index, const json& data);
    void onDeleteTreeItem(const json& data);
    void onOpenQueryBuilder();
    void onTabSelected(int index);
    void onEditorFocusOut();
    void onDatabaseListChanged();
    void onDatabasesSelectItem(int index);
    void onCodeEditReadData(const query_builder_querias& data);
    void onStartDatabaseScanned();
    void onEndDatabaseScanned();

private:
    Ui::MainWindow *ui;

    TreeViewWidget * m_treeQueryas;
    TreeViewWidget * m_treeQuery;
    TreeViewWidget * m_treeParam;
    CodeEditorWidget * m_codeEdit;
    TableToolBar* m_queryasToolbar;
    QueryBuilderDatabasesWidget* m_databases;

    QLabel * m_statusBarText;
    QLabel * m_statusBarIco;

    QSqlDatabase m_connection;
    QMap<QString, TreeItemModel*> m_data_models;


    void initDatabase();
    void openDatabase();
    void updateDatabasesList();
};

#endif // MAINWINDOW_H
