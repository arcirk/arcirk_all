#include "global/arcirk_qt.hpp"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QHBoxLayout"
#include "QVBoxLayout"
#include "query_builder.hpp"
#include "sqlite_utils.hpp"

#include <QSqlError>
#include <QSqlQuery>

#include <QStandardPaths>
#include <QDir>
#include <QFile>

#include <QMessageBox>

#include "iface/iface.hpp"

using namespace arcirk::database::builder;
using namespace arcirk::tree_model;

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::tree_model), test_ref,
    (char, ref)
)

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_queryasToolbar = new TableToolBar(ui->dockWidget);
    m_queryasToolbar->setButtonEnabled("add_group", true);
    //m_queryasToolbar->setButtonEnabled("add_item", true);
    m_treeQueryas = new TreeViewWidget(ui->dockWidget);
    m_treeQueryas->setTableToolBar(m_queryasToolbar);
    m_treeQueryas->set_inners_dialogs(true);
    m_treeQueryas->set_only_groups_in_root(true);

    auto model = new ITree<query_builder_main>(this);
    model->set_column_aliases(QMap<QString, QString>{qMakePair("name", "Наименование")});
    model->set_columns_order(QList<QString>{"name", "ref", "parent", "is_group"});
    model->set_user_role_data("name", tree::NotNullRole, true);
    m_treeQueryas->setModel(model);
    foreach (auto itr, model->columnNames()) {
        if(itr != "name"){
            m_treeQueryas->hideColumn(model->column_index(itr));
        }
    }
    //

    ui->dockVerticalLayout->addWidget(m_queryasToolbar);
    ui->dockVerticalLayout->addWidget(m_treeQueryas);

    ui->dockWidget->setWindowTitle("Запросы");

    auto splitter = new QSplitter(Qt::Vertical, this);
    m_codeEdit = new CodeEditorWidget(splitter);
    m_codeEdit->setEnabled(false);
    splitter->addWidget(m_codeEdit);
    m_treeQuery = new TreeViewWidget(splitter);
    m_treeQuery->setEnabled(false);
    splitter->addWidget(m_treeQuery);

    ui->verticalLayout->addWidget(splitter);

    auto highlighter = new QSourceHighliter(m_codeEdit->document());
    highlighter->setCurrentLanguage(QSourceHighliter::CodeSQL);

    m_statusBarText = new QLabel(this);
    ui->statusbar->addWidget(m_statusBarText);

    auto dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if(!QDir(dir).exists())
        QDir().mkdir(dir);
    auto fileName = dir + QDir::separator() + "main.sqlite";
    m_connection = QSqlDatabase::addDatabase("QSQLITE", "main.sqlite");
    m_connection.setDatabaseName(fileName);
    if (!m_connection.open()) {
        qDebug() << m_connection.lastError().text();
    }else{
        initDatabase();
    }


    m_statusBarText->setText("Готово");

    connect(ui->mnuOpenDatabase, &QAction::triggered, this, &MainWindow::onMnuOpenDatabaseTriggered);
    connect(ui->mnuClose, &QAction::triggered, this, &MainWindow::mnMnuClose);

//    //test
//    const data_ref struct_d(generate_uuid().toStdString(), "test_synonim", "test_table");
//    test_ref f = test_ref();
//    f.ref = *root_tree_conf::ref_to_buffer(struct_d);
//    json ref_j = pre::json::to_json(f);
//    qDebug() << ref_j.dump().c_str();

//    auto c = ref_j["ref"].get<char>();
//    const char * c_ = &c;
//    data_ref df = root_tree_conf::ref_from_buffer(c_);

}

MainWindow::~MainWindow()
{
    if(m_connection.isOpen()){
        m_connection.close();
    }
    delete ui;
}

void MainWindow::initDatabase()
{
    if(!m_connection.isOpen())
        return;



    std::vector<query_builder> m_query;
    m_query.push_back(std::move(query_builder().create_table("qbQueryas", from_structure<query_builder_main>(query_builder_main()))));
    auto m_key = sql_foreign_key();
    auto e = json::array();
    e.push_back(enum_synonym(on_delete_cascade));
    const auto bt = string_to_byte_array(e.dump());
    m_key.events = ByteArray(bt.size());
    std::copy(bt.begin(), bt.end(), m_key.events.begin());
    m_key.field = "query_ref";
    m_key.refereces_table = "qbQueryas";
    m_key.refereces_field = "ref";
    m_query.push_back(std::move(query_builder().create_table("qbData",
                                                             from_structure<query_builder_querias>(query_builder_querias(),"_id", std::map<std::string, sql_foreign_key>{std::pair<std::string, sql_foreign_key>("query_ref", m_key)}))));

    m_query.push_back(query_builder().create_index(sql_index(std::string("qbData") + "_REF_", "qbData", sql_order_type::dbASC, sql_index_compare::compareBINARY, true, std::vector<std::string>{"ref"})));

    QString result = "PRAGMA foreign_keys = 0;";

    foreach (auto itr, m_query) {
        result.append("\n");
        result.append(itr.prepare().c_str());
    }
    result.append("\n");
    result.append("PRAGMA foreign_keys = 1;");
    if(!executeQueryPackade(result.toUtf8(), m_connection)){
        m_treeQueryas->setEnabled(false);
        QMessageBox::critical(this, "Ошибка", m_connection.lastError().text());
    }


}

void MainWindow::onMnuOpenDatabaseTriggered()
{

}

void MainWindow::mnMnuClose()
{
    QApplication::quit();
}

