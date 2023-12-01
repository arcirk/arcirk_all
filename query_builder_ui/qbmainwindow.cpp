#include "global/arcirk_qt.hpp"
#include "qbmainwindow.h"
#include "ui_qbmainwindow.h"
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
#include <QToolButton>

#include <QMovie>

#include "iface/iface.hpp"
#include "query_parser.h"

#include <memory>
#include <QTabWidget>
#include "qsourcehighliter.h"
#include "gui/pairmodel.h"

#include "querybuilderpackagedialog.h"

using namespace source_highlite;
using namespace arcirk::database::builder;
using namespace arcirk::tree_model;

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::tree_model), test_ref,
    (char, ref)
)

QBMainWindow::QBMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::QBMainWindow)
{
    ui->setupUi(this);

    auto dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if(!QDir(dir).exists())
        QDir().mkdir(dir);
    auto fileName = dir + QDir::separator() + "main.sqlite";
    m_connection = QSqlDatabase::addDatabase("QSQLITE", "main.sqlite");
    m_connection.setDatabaseName(fileName);
    if (!m_connection.open()) {
        qDebug() << m_connection.lastError().text();
    }

    m_queryasToolbar = new TableToolBar(ui->dockWidget);
    m_queryasToolbar->setButtonEnabled("add_group", true);
    m_queryasToolbar->setButtonVisible("move_up_item", false);
    m_queryasToolbar->setButtonVisible("move_down_item", false);

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

    ui->layoutQuerias->addWidget(m_queryasToolbar);
    ui->layoutQuerias->addWidget(m_treeQueryas);

    m_treeParam = new TreeViewWidget(this);
    ui->verticalLayoutParam->addWidget(m_treeParam);

    m_databases = new QueryBuilderDatabasesWidget(m_connection, this);
    ui->layoutDatabases->addWidget(m_databases);

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

    m_statusBarIco = new QLabel(this);
    auto mv = new QMovie(":/img/animation_loaded.gif");
    mv->setScaledSize(QSize(16,16));
    m_statusBarIco->setMovie(mv);
    m_statusBarIco->setMaximumHeight(ui->statusbar->height());
    m_statusBarIco->setVisible(false);
    m_statusBarText = new QLabel(this);

    ui->statusbar->addWidget(m_statusBarIco);
    ui->statusbar->addWidget(m_statusBarText);

    initDatabase();
    openDatabase();

    m_statusBarText->setText("Готово");

    connect(ui->mnuOpenDatabase, &QAction::triggered, this, &QBMainWindow::onMnuOpenDatabaseTriggered);
    connect(ui->mnuClose, &QAction::triggered, this, &QBMainWindow::mnMnuClose);
    connect(m_treeQueryas, &TreeViewWidget::treeItemClicked, this, &QBMainWindow::onTreeQueryasRowSelected);
    connect(m_treeQueryas, &TreeViewWidget::addTreeItem, this, &QBMainWindow::onAddTreeItem);
    connect(m_treeQueryas, &TreeViewWidget::editTreeItem, this, &QBMainWindow::onEditTreeItem);
    connect(m_treeQueryas, &TreeViewWidget::deleteTreeItem, this, &QBMainWindow::onDeleteTreeItem);
    connect(ui->btnOpenQueryBuilder, &QToolButton::clicked, this, &QBMainWindow::onOpenQueryBuilder);

    connect(ui->tabWidget, &QTabWidget::tabBarClicked, this, &QBMainWindow::onTabSelected);
    connect(m_codeEdit, &CodeEditorWidget::focusOut, this, &QBMainWindow::onEditorFocusOut);
    connect(m_codeEdit, &CodeEditorWidget::readData, this, &QBMainWindow::onCodeEditReadData);

    connect(m_databases, &QueryBuilderDatabasesWidget::databaseListChanged, this, &QBMainWindow::onDatabaseListChanged);
    connect(m_databases, &QueryBuilderDatabasesWidget::startDatabaseScanned, this, &QBMainWindow::onStartDatabaseScanned);
    connect(m_databases, &QueryBuilderDatabasesWidget::endDatabaseScanned, this, &QBMainWindow::onEndDatabaseScanned);
    connect(ui->cmbDatabases, &QComboBox::currentIndexChanged, this, &QBMainWindow::onDatabasesSelectItem);

    updateDatabasesList();

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

QBMainWindow::~QBMainWindow()
{
    if(m_connection.isOpen()){
        m_connection.close();
    }
    delete ui;
}

void QBMainWindow::initDatabase()
{
    if(!m_connection.isOpen())
        return;

    auto tables = database::get_database_tables(m_connection);
    QList<QString> m_tables{
        "qbQueryas",
        "qbData",
        "qbDatabases",
    };

    bool is_ex = true;

    foreach (auto itr, m_tables) {
        if(tables.indexOf(itr) == -1){
            is_ex = false;
            break;
        }

    }

    if(is_ex)
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

    m_query.push_back(std::move(query_builder().create_table("qbDatabases", from_structure<ibase_object_structure>(ibase_object_structure()))));

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

void QBMainWindow::openDatabase()
{

    if(!m_connection.isOpen())
        return;

    auto query = query_builder();
    auto table = query.select().from("qbQueryas").to_table(m_connection);
    auto model = m_treeQueryas->get_model();
    model->set_table(table);


}

void QBMainWindow::updateDatabasesList()
{
    auto list_model = new PairModel(this);
;
    list_model->setContent(m_databases->databasesList());
    ui->cmbDatabases->setModel(list_model);

    auto db_ref = m_codeEdit->databaseRef();
    int current = 0;

    for (int i = 0; i < list_model->rowCount(); ++i) {
        auto val = list_model->data(list_model->index(i,0), Qt::UserRole).toString();
        if(val == db_ref){
            current = i;
            break;
        }
    }

    ui->cmbDatabases->setCurrentIndex(current);
}

void QBMainWindow::onMnuOpenDatabaseTriggered()
{

}

void QBMainWindow::mnMnuClose()
{
    QApplication::quit();
}

void QBMainWindow::onTreeQueryasRowSelected(const QModelIndex &index)
{
    qDebug() << __FUNCTION__ << index.row() << index.column();
    if(!index.isValid())
        return;
    auto index_ = m_treeQueryas->get_index(index);
    auto model = (ITree<query_builder_main>)m_treeQueryas->get_model();
    if(model.is_group(index_)){
        m_codeEdit->setParent(QUuid());
        m_codeEdit->setPlainText("");
        m_codeEdit->setEnabled(false);
        ui->centralwidget->setEnabled(false);
    }else{
        if(!m_codeEdit->ref().isNull()){
            m_codeEdit->save(m_connection);
        }
        m_codeEdit->setPlainText("");
        m_codeEdit->setEnabled(true);
        m_codeEdit->setParent(model.ref(index_));
        m_codeEdit->read(m_connection, "");
        ui->centralwidget->setEnabled(true);
    }
}

void QBMainWindow::onAddTreeItem(const QModelIndex &index, const json &data)
{
    qDebug() << __FUNCTION__;

    if(!m_connection.isOpen())
        return;

    auto query = query_builder();
    query.use(data);
    QSqlQuery rc(query.insert("qbQueryas", true).prepare().c_str(), m_connection);
    rc.exec();

}
void QBMainWindow::onEditTreeItem(const QModelIndex &index, const json &data)
{
    qDebug() << __FUNCTION__;
    if(!m_connection.isOpen())
        return;

    auto query = query_builder();
    query.use(data);
    QSqlQuery rc(query.update("qbQueryas", true).where(json{{"ref", data["ref"]}}, true).prepare().c_str(), m_connection);
    rc.exec();

}
void QBMainWindow::onDeleteTreeItem(const json &data)
{
    qDebug() << __FUNCTION__;

    if(!m_connection.isOpen())
        return;

    auto query = query_builder();
    QSqlQuery rc(query.remove().from("qbQueryas").where(json{{"ref", data["ref"]}}, true).prepare().c_str(), m_connection);
    rc.exec();
}

void QBMainWindow::onOpenQueryBuilder()
{
    auto parser = std::make_shared<query_parser>(m_databases->database_structure_t(QUuid::fromString(m_codeEdit->databaseRef())));
    auto text = m_codeEdit->textCursor().selectedText();
    if(text.isEmpty()){
        text = m_codeEdit->toPlainText();
    }

    QString queryStr(text);
    queryStr = queryStr.replace(QRegularExpression("(\\/\\*(.|\\n)*?\\*\\/|^--.*\\n|\\t|\\n)", QRegularExpression::CaseInsensitiveOption|QRegularExpression::MultilineOption), " ");
    queryStr = queryStr.trimmed();//.toLower();

    auto data = m_databases->database_structure(QUuid::fromString(m_codeEdit->databaseRef()));   

    auto result = parser->parse(queryStr.toStdString());

    if(!result){
        QMessageBox::critical(this, "Ошибка парсера", parser->last_error().c_str());
        return;
    }

    if(data.empty()){
        QMessageBox::critical(this, "Ошибка", "База данных не подключена!");
        return;
    }

    auto dlg = QueryBuilderPackageDialog(data, this);
    if(!dlg.setData(parser->to_json())){
        QMessageBox::critical(this, "Ошибка парсера", parser->last_error().c_str());
        return;
    }else
        dlg.readData();
    if(dlg.exec()){

    }
}

void QBMainWindow::onTabSelected(int index)
{
    if(index == 0){
        ui->dockWidget->setWindowTitle("Запросы");
    }else if(index == 1){
        ui->dockWidget->setWindowTitle("Базы данных");
    }
}

void QBMainWindow::onEditorFocusOut()
{
    m_codeEdit->save(m_connection);
}

void QBMainWindow::onDatabaseListChanged()
{
    updateDatabasesList();
}

void QBMainWindow::onDatabasesSelectItem(int index)
{
    auto cb_model = (PairModel*)ui->cmbDatabases->model();
    auto d_ref = cb_model->data(cb_model->index(index, 0), Qt::UserRole).toString();
    m_codeEdit->setDatabaseRef(d_ref, m_connection);

}

void QBMainWindow::onCodeEditReadData(const query_builder_querias &data)
{
    if(!data.database_ref.empty()){
        auto model = (PairModel*)ui->cmbDatabases->model();
        auto index = model->findValue(QString(data.database_ref.c_str()));
        if(index.isValid()){
            ui->cmbDatabases->setCurrentIndex(index.row());
        }
    }
}

void QBMainWindow::onStartDatabaseScanned()
{
    m_statusBarIco->setVisible(true);
    m_statusBarIco->movie()->start();
    m_statusBarText->setText("Чтение базы данных ...");
}

void QBMainWindow::onEndDatabaseScanned()
{
    m_statusBarIco->setVisible(false);
    m_statusBarIco->movie()->stop();
    m_statusBarText->setText("Готово");
}
