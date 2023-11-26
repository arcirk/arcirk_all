#include "querybuilderpackagedialog.h"
#include "ui_querybuilderpackagedialog.h"
#include <QToolButton>
#include "dialogquerybuilder.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>
//#include "query_builder.hpp"
#include <QPushButton>
#include <QFile>
#include <QFileDialog>
#include "sqlite_utils.hpp"

//using namespace arcirk::tree_model;
using namespace arcirk::query_builder_ui;
using namespace arcirk::database::builder;

QueryBuilderPackageDialog::QueryBuilderPackageDialog(WebSocketClient* client, QWidget *parent, const QString& query_text) :
    QDialog(parent),
    ui(new Ui::QueryBuilderPackageDialog)
{
    ui->setupUi(this);

    treeView = new TreeViewWidget(this);
    treeView->enable_sort(false);
    ui->verticalLayout->addWidget(treeView);

    m_client = client;

    connect(ui->btnAdd, &QToolButton::clicked, this, &QueryBuilderPackageDialog::onBtnAddClicked);
    connect(ui->btnDelete, &QToolButton::clicked, this, &QueryBuilderPackageDialog::onBtnDeleteClicked);
    connect(ui->btnEdit, &QToolButton::clicked, this, &QueryBuilderPackageDialog::onBtnEditClicked);
    connect(ui->btnQuery, &QPushButton::clicked, this, &QueryBuilderPackageDialog::onBtnQueryClicked);
    connect(ui->btnOpenDataBase, &QPushButton::clicked, this, &QueryBuilderPackageDialog::onOpenDataBaseClicked);
    connect(ui->btnSaveDatabase, &QPushButton::clicked, this, &QueryBuilderPackageDialog::onSaveDataBaseClicked);

    m_connection = QSqlDatabase::addDatabase("QSQLITE", "private.sqlite");
    m_connection.setDatabaseName(":memory:");
    if (!m_connection.open()) {
        qDebug() << m_connection.lastError().text();
    }

    initDatabase();

    auto order = QList<QString>({"line_num", "name"});
    auto model = new ITreeSQlPacketModel(order, this);
    model->enable_message_question();

    model->set_connection(root_tree_conf::sqlIteMemoryConnection, QString(), "qbPackets");
    model->enable_database_changed();

    model->set_hierarchical_list(false);
    model->set_user_role_data("line_num", tree::RowCountRole, true);
    model->set_enable_rows_icons(false);
    auto aliases = QMap<QString, QString>({qMakePair("line_num", "№"), qMakePair("name", "Наименование")});
    model->set_column_aliases(aliases);

    treeView->setModel(model);
    for (int i = 0; i < model->columnCount(); ++i) {
        if(order.indexOf(model->column_name(i)) == -1)
            treeView->hideColumn(i);
    }
    auto header = treeView->header();
    header->resizeSection(0, 20);
    header->setSectionResizeMode(0, QHeaderView::Fixed);

    if(!query_text.isEmpty()){
        parse(query_text);
    }
}

QueryBuilderPackageDialog::~QueryBuilderPackageDialog()
{
    if(m_connection.isOpen()){
        m_connection.close();
        m_connection.removeDatabase("private.sqlite");
    }
    delete ui;
}

void QueryBuilderPackageDialog::onBtnAddClicked()
{
    if(!m_connection.isOpen()){
        qCritical() << "База данных не открыта!";
        return;
    }
    auto packet = query_builder_packet();
    packet.name = "Запрос 1";// + QString::number(ui->treeView->model()->rowCount() + 1).toStdString();
    packet.ref = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    packet.parent = NIL_STRING_UUID;
    auto model = (ITreeSQlPacketModel*)treeView->get_model();
    model->add_struct(packet, QModelIndex(), true);
    auto index = model->find(to_qt_uuid(packet.ref));
    if(index.isValid()){
        treeView->set_current_index(index);
    }
    auto dlg = DialogQueryBuilder(m_client, packet, this);
    if(dlg.exec() == QDialog::Accepted){
        reset();
    }
}

void QueryBuilderPackageDialog::onBtnDeleteClicked()
{
    auto model =(ITreeSQlPacketModel*)treeView->get_model();
    auto index = treeView->current_index();
    if(index.isValid())
        model->remove_row(index.row(), true); // model->remove_sql_data(index);
}

void QueryBuilderPackageDialog::onBtnEditClicked()
{
    if(!m_connection.isOpen()){
        qCritical() << "База данных не открыта!";
        return;
    }

    auto model = (ITreeSQlPacketModel*)treeView->get_model();
    auto index = treeView->current_index();

    if(!index.isValid()){
        QMessageBox::critical(this, "Ошибка", "Не выбран элемент!");
        return;
    }

    auto object = model->object(index);

    auto dlg = DialogQueryBuilder(m_client, object, this);
    if(dlg.exec() == QDialog::Accepted){
        reset();
    }
}

void QueryBuilderPackageDialog::initDatabase()
{
    if(!m_connection.isOpen())
        return;

    std::vector<builder::query_builder> m_query;

    auto query = builder::query_builder();

    auto packets = query_builder_packet();
    packets.ref = NIL_STRING_UUID;
    packets.parent = NIL_STRING_UUID;
    auto items_ = from_structure<query_builder_packet>(packets);
    query = builder::query_builder().create_table("qbPackets", items_);
    m_query.push_back(std::move(query));

    auto item = ibase_object_structure();
    item.ref = NIL_STRING_UUID;
    item.parent = NIL_STRING_UUID;
    item.package_ref = NIL_STRING_UUID;

    QList<std::string> m_tables{
        "qbTables",
        "qbFields",
        "qbGroupBy",
        "qbGroupByFun",
        "qbWhere",
        "qbIndexes",
        "qbTotalGroups",
        "qbAdditionalTable",
        "qbAdditionalIndex",
        "qbTotalFun"
    };

    auto e = json::array();
    e.push_back(enum_synonym(on_delete_cascade));
    const auto bt = string_to_byte_array(e.dump());

    foreach (auto itr, m_tables) {
        auto m_key = sql_foreign_key();
        m_key.events = ByteArray(bt.size());
        std::copy(bt.begin(), bt.end(), m_key.events.begin());
        m_key.field = "package_ref";
        m_key.refereces_table = "qbPackets";
        m_key.refereces_field = "ref";
        auto items = from_structure<ibase_object_structure>(
            item, "_id", std::map<std::string, sql_foreign_key>{
            std::pair<std::string, sql_foreign_key>("package_ref", m_key)});

//        auto items = from_structure<ibase_object_structure>(
//            item, "_id", std::map<std::string, sql_foreign_key>{
//            std::pair<std::string, sql_foreign_key>("package_ref", sql_foreign_key("package_ref"
//                                                                    , "qbPackets"
//                                                                    , bt
//                                                                    , "ref"
//                                                                    , ""
//                                                                    , ""
//                                                                   , ""
//                                                                   , "")
//                                                    )});
        query = builder::query_builder().create_table(itr, items);
        m_query.push_back(std::move(query));
        query = builder::query_builder().create_index(sql_index(itr + "_" + "REF_", itr, sql_order_type::dbASC, sql_index_compare::compareBINARY, true, std::vector<std::string>{"ref"}));
        m_query.push_back(std::move(query));
    }

    auto item_ = query_inners_item();
    item_.ref = NIL_STRING_UUID;
    item_.parent = NIL_STRING_UUID;
    item_.package_ref = NIL_STRING_UUID;
    auto m_key = sql_foreign_key();
    m_key.events = ByteArray(bt.size());
    std::copy(bt.begin(), bt.end(), m_key.events.begin());
    m_key.field = "package_ref";
    m_key.refereces_table = "qbPackets";
    m_key.refereces_field = "ref";
    auto items = from_structure<query_inners_item>(
        item_, "_id", std::map<std::string, sql_foreign_key>{
                                               std::pair<std::string, sql_foreign_key>("package_ref", m_key
                                                                                       )});
    query = builder::query_builder().create_table("qbTableInners", items);
    m_query.push_back(std::move(query));
    query = builder::query_builder().create_index(sql_index(std::string("qbTableInners") + "_" + "REF_", "qbTableInners", sql_order_type::dbASC, sql_index_compare::compareBINARY, true, std::vector<std::string>{"ref"}));
    m_query.push_back(std::move(query));


    QString result = "PRAGMA foreign_keys = 0;";

    foreach (auto itr, m_query) {
        result.append("\n");
        result.append(itr.prepare().c_str());
    }
    result.append("\n");
    result.append("PRAGMA foreign_keys = 1;");

    //qDebug() << qPrintable(result);

    executeQueryPackade(result.toUtf8(), m_connection);
//    if(rc.lastError().isValid())
    //        qCritical() << rc.lastError().text();
}

void QueryBuilderPackageDialog::reset()
{
    auto model = (ITreeSQlPacketModel*)treeView->get_model();
    model->reset();
}

void QueryBuilderPackageDialog::generate_query()
{
    m_result = "";

    auto m_packades = builder::query_builder::array<query_builder_packet>(m_connection, "select * from qbPackets");
    if(m_packades.size() == 0)
        return;

    QList<builder::query_builder> m_querias;

    foreach (auto itr, m_packades) {

        if(itr.type == sql_global_query_type::sqlTypeSelect){
            m_result = query_text(itr.ref);
        }else if(itr.type == sql_global_query_type::sqlTypeCreateTempTable){

        }else if(itr.type == sql_global_query_type::sqlTypeDeleteTempTable){

        }else if(itr.type == sql_global_query_type::sqlTypeCreateTable){

        }else if(itr.type == sql_global_query_type::sqlTypeDeleteTable){

        }
    }

}

QString QueryBuilderPackageDialog::query_text(const std::string &ref)
{
    QString result;
    auto query = builder::query_builder().select().from("qbPackets").where(json{{"ref", ref}}, true);
    auto packade = builder::query_builder::object<query_builder_packet>(m_connection, query.prepare().c_str());
    if(packade.ref.empty())
        return "";

    if(packade.type == 0){
        query = builder::query_builder().select().from("qbFields").where(json{{"package_ref", ref}}, true);
        auto fields = builder::query_builder::array<ibase_object_structure>(m_connection, query.prepare().c_str());
        std::vector<sql_value> vals{};
        QSet<std::string> m_tables;
        foreach (auto itr, fields) {
//            if(itr.user_query == 0){
//                vals.push_back(sql_value(itr.name, itr.alias, ""));
//            }else{
            vals.push_back(sql_value(itr.query, itr.alias, "", (sql_value_type)0));
            //}
            m_tables.insert(itr.parent_name);
        }

        if(m_tables.size() == 0)
            return "SELECT * FROM ";

        auto query_res = builder::query_builder();
        if(m_tables.size() == 1)
            result = query_res.select(vals).from(m_tables.values()[0]).prepare().c_str();
        else
            result = query_res.select(vals).from(m_tables.values()[0]).prepare().c_str();
    }

    return result;
}

void QueryBuilderPackageDialog::parse(const QString &query_text)
{
    QString queryStr(query_text);
    queryStr = queryStr.replace(QRegularExpression("(\\/\\*(.|\\n)*?\\*\\/|^--.*\\n|\\t|\\n)", QRegularExpression::CaseInsensitiveOption|QRegularExpression::MultilineOption), " ");
    queryStr = queryStr.trimmed().toLower();
    //int max_sumbols = queryStr.length();
    QList<QString> m_letters = queryStr.split(" ");
    //int count = 0;
    //QList<QString> vec{};
    QString command = "";
    //bool start = false;
    QList<builder::query_builder> m_querias;
    for (int i = 0; i < m_letters.size(); ++i) {
        auto letter = m_letters[i].trimmed();
        if(letter == "select"){
            command = "select";
            m_querias.append(builder::query_builder());
            continue;
        }

        //if(letter.left(1) == )

    }

}

int QueryBuilderPackageDialog::parse_values(int position, const QList<QString> querias, const QString end, builder::query_builder& query)
{
    int count = 0;
    sql_value value;
    do {
        count++;
        auto letter = querias[position + count];
        if(letter == end)
            return count;

    } while ((count+position) < querias.size());

    return 0;
}

QString arcirk::query_builder_ui::QueryBuilderPackageDialog::generate_text_create_table(const query_builder_packet& package)
{
    auto query = builder::query_builder();
    auto text = query.select().from("qbAdditionalTable").where(json{{"package_ref", package.ref}}, true).prepare();
    auto m_fields = builder::query_builder::array<ibase_object_structure>(m_connection, text.c_str());
    std::string table_name = package.name;

    std::vector<sql_field> fields;
    for (auto itr = m_fields.begin(); itr != m_fields.end(); ++itr) {
        auto field = sql_field();
        //field.autoincrement
    }
    return "";
}

void QueryBuilderPackageDialog::accept()
{

    generate_query();

    return QDialog::accept();
}

QString QueryBuilderPackageDialog::result() const
{
    return m_result;
}

void QueryBuilderPackageDialog::onBtnQueryClicked()
{

}

void QueryBuilderPackageDialog::onOpenDataBaseClicked()
{
    auto result = QFileDialog::getOpenFileName(this, "Открыть файл", "", "Пакет запросов (*.querypack)");
    if(!result.isEmpty()){
        if(!m_connection.isOpen()){
            m_connection.close();
            m_connection.open();
        }
        auto res = arcirk::database::sqliteDBMemFile(m_connection, result, false);
        if(res){
            auto model = treeView->get_model();
            model->reset();
        }
    }
}

void QueryBuilderPackageDialog::onSaveDataBaseClicked()
{
    if(!m_connection.isOpen()){
        QMessageBox::critical(this, "Ошибка", "База данных не открыта!");
        return;
    }

    auto result = QFileDialog::getSaveFileName(this, "Открыть файл", "", "Пакет запросов (*.querypack)");
    if(!result.isEmpty()){
        auto res = arcirk::database::sqliteDBMemFile(m_connection, result, true);
        if(!res){

        }
    }
}

