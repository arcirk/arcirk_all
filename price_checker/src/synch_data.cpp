#include "synch_data.hpp"
#include "database_struct.hpp"
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include "query_builder.hpp"
#include <QStandardPaths>
#include <QDir>
#include "verify_database.hpp"

namespace arcirk::synchronize{

SynchDocumentsBase::SynchDocumentsBase(arcirk::Settings *sett, QObject *parent )
{
    this->setParent(parent);
    m_settings = sett;
    open_database();

}

void SynchDocumentsBase::synchronize()
{
    emit startSynchronize(connectionName());
    nlohmann::json send_srv{};
    bool result = synchronizeBaseDocuments(send_srv);
    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__;
    emit endSynchronize(connectionName(), result, send_srv);
}

void SynchDocumentsBase::setComparisonOfDocuments(const nlohmann::json &resp)
{
    srv_resp = resp;
}

bool SynchDocumentsBase::synchronizeBaseDocuments(nlohmann::json &sendToSrvDocuments)
{
    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__;

    try {

        using namespace arcirk::database;

        if (!sql.open()) {
            qCritical() << sql.lastError().text();
            return false;
        }

        if(srv_resp.empty())
            return false;

        auto objects = srv_resp.value("objects", nlohmann::json{});
        auto comparison_table = srv_resp.value("comparison_table", nlohmann::json{});
        std::map<std::string, std::pair<int,int>> m_ext_table;
        std::vector<std::string> m_vec_new_documents;
        if (comparison_table.is_array() && !comparison_table.empty()) {
            for (auto itr = comparison_table.begin(); itr != comparison_table.end(); ++itr) {
                nlohmann::json r = *itr;
                int ver1 = r["ver1"];
                int ver2 = r["ver2"];
                std::string ref = r["ref"];
                m_ext_table.emplace(ref, std::pair<int,int>(ver1, ver2));
                if(ver1 < ver2){
                    m_vec_new_documents.push_back(ref);
                }
            }
        }

        if(objects.is_array()){
            //sqlDatabase.transaction();
            for (auto const & itr : objects) {
                if(itr.is_object()){
                    auto items = itr.items();
                    for (auto const & itr : items) {
                        //qDebug() << QString::fromStdString(itr.key());
                        auto std_attr = pre::json::from_json<documents>(itr.value()["object"]["StandardAttributes"]);
                        auto query = builder::query_builder();
                        query.use(pre::json::to_json(std_attr));
                        QSqlQuery q(sql);
                        auto d_itr = m_ext_table.find(std_attr.ref);
                        QString query_str;
                        if(d_itr != m_ext_table.end()){
                            if(d_itr->second.second != -1){
                                query_str = QString::fromStdString(query.remove().from(arcirk::enum_synonym(tbDocuments)).where(nlohmann::json{
                                                                                                                                    {"ref", std_attr.ref}
                                                                                                                                }, true).prepare());
                                q.exec(query_str);
                                if(q.lastError().isValid())
                                    qCritical() << q.lastError().text();
                            }

                        }

                        query_str = QString::fromStdString(query.insert(arcirk::enum_synonym(tbDocuments), true).prepare());
                        //qDebug() << qPrintable(query_str);
                        q.exec(query_str);
                        if(q.lastError().isValid())
                            qCritical() << q.lastError().text();

                        auto m_ts = itr.value()["object"]["TabularSections"];
                        if(m_ts.is_array()){
                            for (auto const & tbl : m_ts) {
                                auto m_rows = tbl["strings"];
                                auto name_ts = tbl["name"];
                                auto table = name_ts.get<tables>();
                                if(m_rows.is_array()){
                                    for (auto const & tbl : m_rows) {
                                        query.clear();
                                        if(tbl.is_object()){
                                            query.use(tbl);
                                            std::string ref_fieled;
                                            if(table == tbDocumentsTables)
                                                ref_fieled = "parent";
                                            else if (table == tbDocumentsMarkedTables)
                                                ref_fieled = "document_ref";

                                            query_str = QString::fromStdString(query.remove().from(arcirk::enum_synonym(table)).where(nlohmann::json{
                                                                                                                                          {ref_fieled, std_attr.ref}
                                                                                                                                      }, true).prepare());


                                            q.exec(query_str);
                                            if(q.lastError().isValid())
                                                qCritical() << q.lastError().text();
                                            query_str = QString::fromStdString(query.insert(arcirk::enum_synonym(table), true).prepare());
                                            q.exec(query_str);
                                            if(q.lastError().isValid())
                                                qCritical() << q.lastError().text();
                                        }
                                    }
                                }
                            }
                        }

                    }
                }
            }
            //sqlDatabase.commit();
        }else if(objects.is_object()){
            auto items = objects.items();
            for (auto const & itr : items) {
                qDebug() << QString::fromStdString(itr.key());
            }
        }

        //Выгружаем на севрвер локальные документы с версией выше чем на сервере
        //nlohmann::json result{};
        if(m_vec_new_documents.size() > 0){
            foreach (auto const& itr, m_vec_new_documents) {
                sendToSrvDocuments += data_synchronization_get_object(
                    arcirk::enum_synonym(tables::tbDocuments), itr, sql
                    );
            }
        }

        //Зачищаем базу от помеченных на удаление объектов
        int count = 0;
        QString query(builder::query_builder().row_count().from(arcirk::enum_synonym(tables::tbDocuments)).where(nlohmann::json{
                                                                                                                     {"deletion_mark", 1}
                                                                                                                 }, true).prepare().data());
        QSqlQuery rs(query, sql);
        rs.exec();
        while (rs.next()) {
            count++;
        }

        if(count > 0){

            sql.transaction();
            rs.clear();
            rs.exec("delete from DocumentsMarkedTables where DocumentsMarkedTables.document_ref in (select Documents.ref from Documents where Documents.deletion_mark = '1');");
            rs.exec("delete from DocumentsTables where DocumentsTables.parent in (select Documents.ref from Documents where Documents.deletion_mark = '1');");
            rs.exec("delete from Documents where Documents.deletion_mark = '1';");
            sql.commit();

        }

        sql.close();
        //sqlDatabase.removeDatabase("SyncCon");

    } catch (...) {
        return false;
    }

    return true;
}

SynchInitialDataEntry::SynchInitialDataEntry(Settings *sett)
{
    m_settings = sett;
    open_database();
}

void SynchInitialDataEntry::synchronize()
{

}

void SynchronizeBase::open_database()
{
    sql = QSqlDatabase::addDatabase("QSQLITE", connectionName());
#ifndef Q_OS_WINDOWS
    sql.setDatabaseName("pricechecker.sqlite");
#else
    auto path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(path);
    if(!dir.exists())
        dir.mkpath(path);

    auto fileName= path + "/pricechecker.sqlite";
    sql.setDatabaseName(fileName);
#endif
}

}
