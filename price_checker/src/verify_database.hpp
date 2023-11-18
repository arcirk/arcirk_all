#ifndef VERIFY_DATABASE_HPP
#define VERIFY_DATABASE_HPP

#include <QtCore>
#include "database_struct.hpp"
#include <QSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlField>
#include <QUuid>
#include "query_builder.hpp"

using namespace arcirk::database::builder;

namespace arcirk::database{

template<class T>
static inline QString to_ddl_table(const std::string& table_name){

    auto query = query_builder();
    auto fields = from_structure<T>(T());
    query.create_table(table_name, fields);
    return query.prepare().c_str();

}

static inline QString get_ddl(tables table){
    switch (table) {
    case tbUsers: return "";
    case tbMessages: return "";
    case tbOrganizations: return "";
    case tbSubdivisions: return "";
    case tbWarehouses: return "";
    case tbPriceTypes: return "";
    case tbWorkplaces: return "";
    case tbDevices: return "";
    case tbDocumentsTables: return to_ddl_table<document_table>(enum_synonym(tbDocumentsTables));
    case tbDocuments: return to_ddl_table<documents>(enum_synonym(tbDocuments));
    case tbNomenclature: return to_ddl_table<nomenclature>(enum_synonym(tbNomenclature));
    case tbDatabaseConfig: return to_ddl_table<database_config>(enum_synonym(tbDatabaseConfig));
    case tbDevicesType:  return "";
    case tbPendingOperations: to_ddl_table<pending_operations>(enum_synonym(tbPendingOperations));
    case tbBarcodes: return to_ddl_table<barcodes>(enum_synonym(tbBarcodes));
    case tbDocumentsMarkedTables: return to_ddl_table<document_marked_table>(enum_synonym(tbDocumentsMarkedTables));
    case tables_INVALID:{
        break;
    }
    }

    return {};
}

static inline std::string get_ddl(views table){
    switch (table) {
    case dvDevicesView: break;
    case dvDocumentsTableView: return documents_table_view_ddl();
    case views_INVALID:{
        break;
    }
    }

    return {};
}

static inline QMap<std::string, table_info_sqlite>  table_info(QSqlDatabase& sql, tables table) {
    QMap<std::string, table_info_sqlite> result{};
    QString  query = QString("PRAGMA table_info(\"%1\");").arg(QString::fromStdString(arcirk::enum_synonym(table)));
    QSqlQuery rs(query);
    while (rs.next())
    {
        auto info = table_info_sqlite();
        info.name = rs.value("name").toString().toStdString();
        info.type = rs.value("type").toString().toStdString();
        result.insert(info.name, info);
    }
    return result;
}
static inline QMap<std::string, table_info_sqlite>  table_info(QSqlDatabase& sql, views table) {
    QMap<std::string, table_info_sqlite> result{};
    QString  query = QString("PRAGMA table_info(\"%1\");").arg(QString::fromStdString(arcirk::enum_synonym(table)));
    QSqlQuery rs(query);
    while (rs.next())
    {
        auto info = table_info_sqlite();
        info.name = rs.value("name").toString().toStdString();
        info.type = rs.value("type").toString().toStdString();
        result.insert(info.name, info);
    }
    return result;
}

static inline std::string query_insert(const std::string& table_name, nlohmann::json values){
    std::string result = QString("insert into %1 (").arg(QString::fromStdString(table_name)).toStdString();
    std::string string_values;
    std::vector<std::pair<std::string, nlohmann::json>> m_list;
    auto items_ = values.items();
    for (auto itr = items_.begin(); itr != items_.end(); ++itr) {
        m_list.emplace_back(itr.key(), itr.value());
    }

    for (auto itr = m_list.cbegin(); itr != m_list.cend() ; ++itr) {
        result.append("[" + itr->first + "]");
        std::string value;
        if(itr->second.is_string())
            value = itr->second.get<std::string>();
        else if(itr->second.is_number_float()){
            value = std::to_string(itr->second.get<double>());
            auto index = value.find(",");
            if(index > 0)
                value.replace(index, 1, ".");
        }else if(itr->second.is_number_integer())
            value = std::to_string(itr->second.get<long long>());

        boost::erase_all(value, "'");

        if(value.empty())
            string_values.append("''");
        else
            string_values.append(QString("'%1'").arg(QString::fromStdString(value)).toStdString());
        if(itr != (--m_list.cend())){
            result.append(",\n");
            string_values.append(",\n");
        }
    }
    result.append(")\n");
    result.append("values(");
    result.append(string_values);
    result.append(")");

    return result;
}

static inline nlohmann::json get_json_object(QSqlQuery& rs){
    QSqlRecord row = rs.record();
    auto values = nlohmann::json::object();
    for(int i = 0; i < row.count(); ++i){
        std::string column_name = row.fieldName(i).toStdString();
        QVariant val = row.field(i).value();

        if(val.userType() == QMetaType::QString)
            values[column_name] = val.toString().toStdString();
        else if(val.userType() == QMetaType::Double)
            values[column_name] = val.toDouble();
        else if(val.userType() == QMetaType::Int)
            values[column_name] = val.toInt();
        else if(val.userType() == QMetaType::LongLong)
            values[column_name] = val.toLongLong();
        else if(val.userType() == QMetaType::ULongLong)
            values[column_name] = val.toULongLong();
        else
            values[column_name] = "";
    }
    return values;
}

static inline void rebase(QSqlDatabase& sql, tables table){

    QString table_name = QString::fromStdString(arcirk::enum_synonym(table));
    QString temp_query = QString("create temp table %1_temp as select * from %1;").arg(table_name);

    sql.transaction();
    QSqlQuery query;
    query.exec(temp_query);
    query.exec(QString("drop table %1;").arg(table_name));
    query.exec(get_ddl(table));
    sql.commit();

    sql.transaction();
    QSqlQuery rs(QString("select * from %1_temp;").arg(table_name));
    //std::vector<std::string> columns{};
    auto t_info = table_info(sql, table);

    while (rs.next())
    {
        QSqlRecord row = rs.record();
        nlohmann::json values{};

        for(int i = 0; i < row.count(); ++i)
        {
            //const column_properties & props = row.get_properties(i);
            std::string column_name = row.fieldName(i).toStdString();

            if(t_info.find(column_name) == t_info.end())
                continue;

            QVariant val = row.field(i).value();

            if(val.userType() == QMetaType::QString)
                values[column_name] = val.toString().toStdString();
            else if(val.userType() == QMetaType::Double)
                values[column_name] = val.toDouble();
            else if(val.userType() == QMetaType::Int)
                values[column_name] = val.toInt();
            else if(val.userType() == QMetaType::LongLong)
                values[column_name] = val.toLongLong();
            else if(val.userType() == QMetaType::ULongLong)
                values[column_name] = val.toULongLong();


        }

        query.exec(QString::fromStdString(query_insert(table_name.toStdString(), values)));

    }

    query.exec(QString("drop table if exists %1_temp;").arg(table_name));

    sql.commit();

}

static inline std::map<tables, int> get_release_tables_versions(){
    std::map<tables, int> result;
    result.emplace(tables::tbDatabaseConfig, 2);
    result.emplace(tables::tbNomenclature, 5);
    result.emplace(tables::tbDocuments, 4);
    result.emplace(tables::tbDevices, 3);
    result.emplace(tables::tbMessages, 3);
    result.emplace(tables::tbUsers, 3);
    result.emplace(tables::tbDevicesType, 3);
    result.emplace(tables::tbDocumentsTables, 5);
    result.emplace(tables::tbOrganizations, 3);
    result.emplace(tables::tbPriceTypes, 3);
    result.emplace(tables::tbSubdivisions, 3);
    result.emplace(tables::tbWarehouses, 3);
    result.emplace(tables::tbWorkplaces, 3);
    result.emplace(tables::tbPendingOperations, 1);
    result.emplace(tables::tbBarcodes, 4);
    result.emplace(tables::tbDocumentsMarkedTables, 1);
    return result;
}

static inline std::vector<tables> tables_name_array(){
    std::vector<tables> result = {
        //            tbUsers,
        //            tbMessages,
        //            tbOrganizations,
        //            tbSubdivisions,
        //            tbWarehouses,
        //            tbPriceTypes,
        //            tbWorkplaces,
        //            tbDevices,
        //            tbDevicesType,
        tbDocuments,
        tbDocumentsTables,
        tbNomenclature,
        tbDatabaseConfig,
        tbPendingOperations,
        tbBarcodes,
        tbDocumentsMarkedTables
    };
    return result;
}

static inline std::vector<views> views_name_array(){
    std::vector<views> result = {
        dvDocumentsTableView
    };
    return result;
}

static inline std::vector<std::string> get_database_tables(QSqlDatabase& sql){

    QSqlQuery rs("SELECT name FROM sqlite_master WHERE type='table';");
    std::vector<std::string> result;
    while (rs.next())
    {
        result.push_back(rs.value(0).toString().toStdString());
    }

    return result;
}

static inline std::vector<std::string> get_database_views(QSqlDatabase& sql){

    QSqlQuery rs("SELECT name FROM sqlite_master WHERE type='view';");
    std::vector<std::string> result;
    while (rs.next())
    {
        result.push_back(rs.value(0).toString().toStdString());
    }

    return result;
}
static inline void verify_database_views(QSqlDatabase& sql, const std::vector<std::string>& db_views, const std::string& view_name,  const std::string& t_ddl, bool is_rebase = false){

    if(std::find(db_views.begin(), db_views.end(), view_name) == db_views.end()) {
        QSqlQuery rs(t_ddl.c_str(), sql);
        rs.exec();
    }else{
        if(is_rebase){
            sql.transaction();
            QSqlQuery rs(QString("drop view %1").arg(view_name.c_str()), sql);
            rs.exec();
            rs.exec(t_ddl.c_str());
            sql.commit();
        }
    }
}
static inline void verify_database(QSqlDatabase& sql){

    qDebug() << __FUNCTION__;
    auto release_table_versions = get_release_tables_versions(); //Текущие версии релиза
    auto database_tables = get_database_tables(sql); //Массив существующих таблиц
    auto database_views = get_database_views(sql); //Массив существующих представлений
    auto tables_arr = tables_name_array(); //Массив имен таблиц
    //bool is_rebase = false;

    QSqlQuery rs;
    //Сначала проверим, существует ли таблица версий
    if(std::find(database_tables.begin(), database_tables.end(), arcirk::enum_synonym(tables::tbDatabaseConfig)) == database_tables.end()) {
        auto ddl = get_ddl(tables::tbDatabaseConfig);
        rs.exec(ddl);
    }

    //Заполним массив версий для сравнения
    std::map<tables, int> current_table_versions;
    rs.exec(QString("select * from %1;").arg(arcirk::enum_synonym(tables::tbDatabaseConfig).data()));
    while (rs.next())
    {
        nlohmann::json t_name = rs.value("first").toString().toStdString();
        auto t_ver = rs.value("version").toInt();
        current_table_versions.emplace(t_name.get<tables>(), t_ver);
    }

    bool is_rebase = false;
    //Выполним реструктуризацию
    for (auto t_name : tables_arr) {
        if(t_name == tables::tbDatabaseConfig)
            continue;
        if(std::find(database_tables.begin(), database_tables.end(), arcirk::enum_synonym(t_name)) == database_tables.end()){
            //Таблицы не существует, просто создаем новую
            sql.transaction();
            rs.exec(get_ddl(t_name));
            rs.exec(QString::fromStdString(query_insert(arcirk::enum_synonym(tables::tbDatabaseConfig), nlohmann::json{
                                                                                                            {"first", arcirk::enum_synonym(t_name)},
                                                                                                            {"version", release_table_versions[t_name]},
                                                                                                            {"ref", QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString()}
                                                                                                        })));
            sql.commit();
            is_rebase = true;
        }else{
            //Если существует, проверяем версию таблицы если не совпадает запускаем реструктуризацию
            int current_ver = 0;
            auto itr_ver = current_table_versions.find(t_name);
            if(itr_ver != current_table_versions.end())
                current_ver = itr_ver->second;
            if(release_table_versions[t_name] != current_ver){
                rebase(sql, t_name);
                sql.transaction();

                if(current_ver == 0)
                    rs.exec(QString::fromStdString(query_insert(arcirk::enum_synonym(tables::tbDatabaseConfig), nlohmann::json{
                                                                                                                    {"first", arcirk::enum_synonym(t_name)},
                                                                                                                    {"version", release_table_versions[t_name]},
                                                                                                                    {"ref",  QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString()}
                                                                                                                })));
                else
                    rs.exec(QString("update %1 set version='%2' where [first]='%3'").arg(
                        QString::fromStdString(arcirk::enum_synonym(tables::tbDatabaseConfig)),
                        QString::number(release_table_versions[t_name]),
                        QString::fromStdString(arcirk::enum_synonym(t_name))));
                sql.commit();
            }
            is_rebase = true;
        }
    }
    //Проверка представлений
    for (auto view : views_name_array()) {
        verify_database_views(sql, database_views,  arcirk::enum_synonym(view), get_ddl(view), is_rebase);
    }

}

static inline void execute(const std::string& query_text, QSqlDatabase& sql, nlohmann::json& result_table, const std::vector<std::string>& column_ignore = {}){

    using namespace nlohmann;

    QSqlQuery rs(QString::fromStdString(query_text));


    json columns = {"line_number"};
    json roms = {};
    int line_number = 0;

    while (rs.next())
    {
        line_number++;
        QSqlRecord row = rs.record();
        json j_row = {{"line_number", line_number}};

        for(int i = 0; i < row.count(); ++i)
        {
            //const column_properties & props = row.get_properties(i);
            std::string column_name = row.fieldName(i).toStdString();

            if((columns.size()) != row.count() + 1){
                columns.push_back(column_name);
            }

            if(std::find(column_ignore.begin(), column_ignore.end(), column_name) != column_ignore.end()){
                j_row += {column_name, ""};
                continue;
            }

            QVariant val = row.field(i).value();

            if(val.userType() == QMetaType::QString)
                j_row += {column_name, val.toString().toStdString()};
            else if(val.userType() == QMetaType::Double)
                j_row += {column_name, val.toDouble()};
            else if(val.userType() == QMetaType::Int)
                j_row += {column_name, val.toInt()};
            else if(val.userType() == QMetaType::LongLong)
                j_row += {column_name, val.toLongLong()};
            else if(val.userType() == QMetaType::ULongLong)
                j_row += {column_name, val.toULongLong()};

        }

        roms += j_row;
    }

    result_table = {
        {"columns", columns},
        {"rows", roms}
    };

}
static inline nlohmann::json data_synchronization_get_object(const std::string& table_name, const std::string& ref, QSqlDatabase& sql){

    using namespace arcirk::database;

    auto query = builder::query_builder();

    nlohmann::json j_table = table_name;
    nlohmann::json j_object{};
    auto o_table = j_table.get<database::tables>();
    if(o_table == database::tbDocuments){
        std::vector<database::documents> m_vec = query.select().from(table_name).where({{"ref", ref}},
                                                                                            true).rows_to_array<database::documents>(
                                                         sql);
        if(!m_vec.empty()){
            auto r = m_vec[0];
            j_object["object"]["StandardAttributes"] = pre::json::to_json<database::documents>(r);

            query.clear();
            std::vector<database::document_table> m_vec_table = query.select().from(
                                                                                       arcirk::enum_synonym(database::tables::tbDocumentsTables)).where({{"parent", ref}},
                                                                           true).rows_to_array<database::document_table>(
                                                                        sql);
            nlohmann::json n_json_table{};
            foreach (const auto itr , m_vec_table) {
                n_json_table += pre::json::to_json<database::document_table>(itr);
            }

            j_object["object"]["TabularSections"] +=  nlohmann::json{
                {"name", arcirk::enum_synonym(database::tables::tbDocumentsTables)},
                {"strings", n_json_table}
            };


            query.clear();
            std::vector<database::document_marked_table> m_vec_table_m = query.select().from(
                                                                                                arcirk::enum_synonym(database::tables::tbDocumentsMarkedTables)).where({{"document_ref", ref}},
                                                                                    true).rows_to_array<database::document_marked_table>(
                                                                                 sql);
            nlohmann::json n_json_table_m{};
            foreach (const auto itr , m_vec_table_m) {
                n_json_table_m += pre::json::to_json<database::document_marked_table>(itr);
            }

            j_object["object"]["TabularSections"] +=  nlohmann::json{
                {"name", arcirk::enum_synonym(database::tables::tbDocumentsMarkedTables)},
                {"strings", n_json_table_m}
            };

        }
    }else if(o_table == database::tbDevices){
        std::vector<database::devices> m_vec = query.select().from(table_name).where({{"ref", ref}},
                                                                                          true).rows_to_array<database::devices>(
                                                       sql);
        if(!m_vec.empty()){
            auto r = m_vec[0];
            j_object ={
                {"object", {"StandardAttributes", pre::json::to_json<database::devices>(r)}}
            };
        }
    }else if(o_table == database::tbMessages){
        std::vector<database::messages> m_vec = query.select().from(table_name).where({{"ref", ref}},
                                                                                           true).rows_to_array<database::messages>(
                                                        sql);
        if(!m_vec.empty()){
            auto r = m_vec[0];
            j_object ={
                {"object", {"StandardAttributes" ,pre::json::to_json<database::messages>(r)}}
            };
        }
    }else if(o_table == database::tbOrganizations){
        std::vector<database::organizations> m_vec = query.select().from(table_name).where({{"ref", ref}},
                                                                                                true).rows_to_array<database::organizations>(
                                                             sql);
        if(!m_vec.empty()){
            auto r = m_vec[0];
            j_object ={
                {"object", {"StandardAttributes" ,pre::json::to_json<database::organizations>(r)}}
            };
        }
    }else if(o_table == database::tbPriceTypes){
        std::vector<database::price_types> m_vec = query.select().from(table_name).where({{"ref", ref}},
                                                                                              true).rows_to_array<database::price_types>(
                                                           sql);
        if(!m_vec.empty()){
            auto r = m_vec[0];
            j_object ={
                {"object", {"StandardAttributes" ,pre::json::to_json<database::price_types>(r)}}
            };
        }
    }else if(o_table == database::tbSubdivisions){
        std::vector<database::subdivisions> m_vec = query.select().from(table_name).where({{"ref", ref}},
                                                                                               true).rows_to_array<database::subdivisions>(
                                                            sql);
        if(!m_vec.empty()){
            auto r = m_vec[0];
            j_object ={
                {"object", {"StandardAttributes" ,pre::json::to_json<database::subdivisions>(r)}}
            };
        }
    }else if(o_table == database::tbWarehouses){
        std::vector<database::warehouses> m_vec = query.select().from(table_name).where({{"ref", ref}},
                                                                                             true).rows_to_array<database::warehouses>(
                                                          sql);
        if(!m_vec.empty()){
            auto r = m_vec[0];
            j_object ={
                {"object", {"StandardAttributes" ,pre::json::to_json<database::warehouses>(r)}}
            };
        }
    }else if(o_table == database::tbWorkplaces){
        std::vector<database::workplaces> m_vec = query.select().from(table_name).where({{"ref", ref}},
                                                                                             true).rows_to_array<database::workplaces>(
                                                          sql);
        if(!m_vec.empty()){
            auto r = m_vec[0];
            j_object ={
                {"object", {"StandardAttributes" ,pre::json::to_json<database::workplaces>(r)}}
            };
        }
    }else
        throw NativeException("Сериализация выбранной таблицы не поддерживается");

    j_object["table_name"] = table_name;

    return j_object;
}

inline void add_string_representation(std::string& src, const std::string& t_name, json representation){
    if(representation.empty())
        return;

    try {
        if(representation.is_object()){
            std::string tmp = representation["representation"]["template"];
            auto arr = representation["representation"]["values"];
            if(!arr.is_array())
                return;
            for (auto it = arr.begin();  it != arr.end() ; ++it) {
                auto item = *it;
                if(!item.is_object())
                    return;
                boost::replace_all(tmp, item["arg"].get<std::string>(), item["column"].get<std::string>());
            }
            src.append("\n");
            src.append(tmp + " as representation");
        }
    } catch (const std::exception &e) {
        NativeException(e.what());
    }
}

}


#endif // VERIFY_DATABASE_HPP
