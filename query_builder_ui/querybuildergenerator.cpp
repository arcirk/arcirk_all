#include "querybuildergenerator.h"
#include "global/arcirk_qt.hpp"

using namespace arcirk::query_builder_ui;
using namespace arcirk::tree_model;

QueryBuilderGenerator::QueryBuilderGenerator()
{

}

bool QueryBuilderGenerator::from_database(const QString &connection_name, const std::string &query_ref)
{

    m_db = QSqlDatabase::database(connection_name);
    Q_ASSERT(m_db.isOpen());

    auto rc = m_db.exec(query_builder().select().from("qbPackets").where(json{{"ref", query_ref}}, true).prepare().c_str());
    if(rc.lastError().isValid()){
        qCritical() << rc.lastError().text();
        return false;
    }

    if(!rc.next()){
       return false;
    }

    auto query_item = to_std_struct<query_builder_packet>(rc);
    auto query = std::make_shared<arcirk::database::builder::query_builder>();

    if(query_item.type == 0){ //select

    }else if(query_item.type == 1){ //создание временной таблицы

    }else if(query_item.type == 2){ //уничтожение временной таблицы

    }else if(query_item.type == 3){ //создание таблицы

    }else if(query_item.type == 4){ //уничтожение таблицы

    }

    return true;
}

std::vector<ibase_object_structure> QueryBuilderGenerator::sql_rows(const std::string &packet_id)
{
    return {};
}

template<typename T>
T QueryBuilderGenerator::to_std_struct(QSqlQuery &rc)
{
    auto m = pre::json::to_json(T());

    for (auto itr = m.items().begin(); itr != m.items().end(); ++itr) {
        auto val = rc.value(itr.key().c_str());
    }

    return secure_serialization<T>(m);
}
