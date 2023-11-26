#ifndef SQLITE_UTILS_HPP
#define SQLITE_UTILS_HPP

#include <sqlite3.h>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QVariant>
#include "query_builder.hpp"
#include "global/arcirk_qt.hpp"

namespace arcirk::database {
/*
** This function is used to load the contents of a database file on disk
** into the "main" database of open database connection pInMemory, or
** to save the current contents of the database opened by pInMemory into
** a database file on disk. pInMemory is probably an in-memory database,
** but this function will also work fine if it is not.
**
** Parameter zFilename points to a nul-terminated string containing the
** name of the database file on disk to load from or save to. If parameter
** isSave is non-zero, then the contents of the file zFilename are
** overwritten with the contents of the database opened by pInMemory. If
** parameter isSave is zero, then the contents of the database opened by
** pInMemory are replaced by data loaded from the file zFilename.
**
** If the operation is successful, SQLITE_OK is returned. Otherwise, if
** an error occurs, an SQLite error code is returned.
*/
inline bool sqliteDBMemFile(QSqlDatabase memdb, QString filename, bool save )
{
    bool state = false;
    QVariant v = memdb.driver()->handle();
    if( v.isValid() && qstrcmp(v.typeName(),"sqlite3*") == 0 )
    {
        // v.data() returns a pointer to the handle
        sqlite3 * handle = *static_cast<sqlite3 **>(v.data());
        if( handle != 0 ) // check that it is not NULL
        {
            sqlite3 * pInMemory = handle;
            QByteArray array = filename.toLocal8Bit();
            const char * zFilename = array.data();
            int rc;                   /* Function return code */
            sqlite3 *pFile;           /* Database connection opened on zFilename */
            sqlite3_backup *pBackup;  /* Backup object used to copy data */
            sqlite3 *pTo;             /* Database to copy to (pFile or pInMemory) */
            sqlite3 *pFrom;           /* Database to copy from (pFile or pInMemory) */

            /* Open the database file identified by zFilename. Exit early if this fails
            ** for any reason. */
            rc = sqlite3_open( zFilename, &pFile );
            if( rc==SQLITE_OK ){

                /* If this is a 'load' operation (isSave==0), then data is copied
              ** from the database file just opened to database pInMemory.
              ** Otherwise, if this is a 'save' operation (isSave==1), then data
              ** is copied from pInMemory to pFile.  Set the variables pFrom and
              ** pTo accordingly. */
                pFrom = ( save ? pInMemory : pFile);
                pTo   = ( save ? pFile     : pInMemory);

                /* Set up the backup procedure to copy from the "main" database of
              ** connection pFile to the main database of connection pInMemory.
              ** If something goes wrong, pBackup will be set to NULL and an error
              ** code and  message left in connection pTo.
              **
              ** If the backup object is successfully created, call backup_step()
              ** to copy data from pFile to pInMemory. Then call backup_finish()
              ** to release resources associated with the pBackup object.  If an
              ** error occurred, then  an error code and message will be left in
              ** connection pTo. If no error occurred, then the error code belonging
              ** to pTo is set to SQLITE_OK.
              */
                pBackup = sqlite3_backup_init(pTo, "main", pFrom, "main");
                if( pBackup ){
                    (void)sqlite3_backup_step(pBackup, -1);
                    (void)sqlite3_backup_finish(pBackup);
                }
                rc = sqlite3_errcode(pTo);
            }

            /* Close the database connection opened on database file zFilename
            ** and return the result of this function. */
            (void)sqlite3_close(pFile);

            if( rc == SQLITE_OK ) state = true;
        }
    }
    return state;
}

static inline QList<QString> get_database_tables(QSqlDatabase& sql){

    QSqlQuery rs("SELECT name FROM sqlite_master WHERE type='table';", sql);
    rs.exec();
    QList<QString> result;
    while (rs.next())
    {
        result.append(rs.value(0).toString());
    }

    return result;
}

static inline QList<QString> get_database_views(QSqlDatabase& sql){

    QSqlQuery rs("SELECT name FROM sqlite_master WHERE type='view';", sql);
    rs.exec();
    QList<QString> result;
    while (rs.next())
    {
        result.append(rs.value(0).toString());
    }

    return result;
}

static inline std::map<std::string, arcirk::database::table_info_sqlite>  table_info(QSqlDatabase& sql, const std::string& table, builder::sql_database_type type, const std::string& database_name = "") {

    std::map<std::string, table_info_sqlite> result{};
    std::string  query = arcirk::str_sample("PRAGMA table_info(\"%1%\");", table);
    std::string c_name = "name";
    std::string c_type = "type";
    if(type == builder::sql_database_type::type_ODBC){
        QSqlQuery rc(arcirk::str_sample("USE %1%;", database_name).c_str(), sql);
        rc.exec();
        query = arcirk::str_sample("SELECT * FROM INFORMATION_SCHEMA.COLUMNS WHERE table_name = '%1%'", table);
        c_name = "COLUMN_NAME";
        c_type = "DATA_TYPE";
    }

    QSqlQuery rs(query.c_str(), sql);

    while (rs.next()) {
        auto info = table_info_sqlite();
        info.name = rs.value(c_name.c_str()).toString().toStdString();
        info.type = rs.value(c_type.c_str()).toString().toStdString();
        result.emplace(info.name, info);
    }
    return result;
}

inline nlohmann::json get_database_structure(QSqlDatabase& sql, const std::string& parent = NIL_STRING_UUID)
{
    using json = nlohmann::json;

    //using query_builder = arcirk::database::builder::query_builder;
    using namespace arcirk::database;
    using namespace arcirk::tree_model;

    if(!sql.isOpen())
        return {};

    auto database_tables = get_database_tables(sql);
    auto database_views = get_database_views(sql);

    std::vector<json> m_groups;
    std::vector<json> m_childs;

    auto res = json::object();
    auto m_tables= ibase_object_structure();
    m_tables.name = "Таблицы";
    m_tables.alias = "Таблицы";
    m_tables.full_name = "Таблицы";
    m_tables.object_type = "tablesRoot";
    m_tables.ref = boost::to_string(arcirk::uuids::md5_to_uuid(arcirk::uuids::to_md5(m_tables.name + m_tables.object_type)));
    m_tables.parent = parent;
    m_tables.is_group = 1;
    m_groups.push_back(pre::json::to_json(m_tables));
    auto m_views= ibase_object_structure();
    m_views.name = "Представления";
    m_views.alias = "Представления";
    m_views.alias = "Представления";
    m_views.object_type = "viewsRoot";
    m_views.ref = boost::to_string(arcirk::uuids::md5_to_uuid(arcirk::uuids::to_md5(m_views.name + m_views.object_type)));
    m_views.parent = parent;
    m_views.is_group = 1;
    m_groups.push_back(pre::json::to_json(m_views));

    for (auto const& table: database_tables) {
        //int count = 0;
        if(table == "sqlite_sequence")
            continue;

        auto m_struct = ibase_object_structure();
        m_struct.name = table.toStdString();
        m_struct.alias = table.toStdString();
        m_struct.full_name = table.toStdString();
        m_struct.data_type = "";
        m_struct.ref = boost::to_string(arcirk::uuids::md5_to_uuid(arcirk::uuids::to_md5(m_struct.name + m_struct.object_type))); //
        m_struct.parent = m_tables.ref;
        m_struct.is_group = 1;
        m_struct.object_type = "table";
        m_struct.base_ref = m_struct.ref; //arcirk::uuids::random_uuid());
        m_struct.base_parent = m_tables.ref;

        m_groups.push_back(pre::json::to_json(m_struct));


        auto details = table_info(sql, table.toStdString(), builder::sql_database_type::type_Sqlite3);

        for (auto const& itr : details) {
            auto m_details = ibase_object_structure();
            m_details.data_type = itr.second.type;
            m_details.name = itr.second.name;
            m_details.alias = itr.second.name;
            m_details.full_name = table.toStdString() + "." + itr.second.name;
            m_details.query = table.toStdString() + "." + itr.second.name;
            m_details.ref = boost::to_string(arcirk::uuids::md5_to_uuid(arcirk::uuids::to_md5(m_details.name + m_details.object_type))); //arcirk::uuids::random_uuid());
            m_details.parent = m_struct.ref;
            m_details.base_ref = m_details.ref; //arcirk::uuids::random_uuid());
            m_details.base_parent = m_struct.ref;
            m_details.is_group = 0;
            m_details.object_type = "field";
            m_details.parent_alias = m_struct.name;
            m_details.parent_name = m_struct.name;
            m_childs.push_back(pre::json::to_json(m_details));
        }
    }

    for (auto const& view: database_views) {
        auto m_struct = ibase_object_structure();
        m_struct.name = view.toStdString();
        m_struct.alias = view.toStdString();
        m_struct.full_name = view.toStdString();
        m_struct.object_type = "view";
        m_struct.ref = boost::to_string(arcirk::uuids::md5_to_uuid(arcirk::uuids::to_md5(m_struct.name + m_struct.object_type))); //arcirk::uuids::random_uuid());
        m_struct.parent = m_views.ref;
        m_struct.is_group = 0;
        m_struct.base_ref = m_struct.ref; //arcirk::uuids::random_uuid());
        m_struct.base_parent = m_views.ref;
        m_childs.push_back(pre::json::to_json(m_struct));
    }

    auto m_empty = pre::json::to_json(ibase_object_structure());

    auto columns = json::array();

    for (auto itr = m_empty.items().begin(); itr != m_empty.items().end(); ++itr) {
        columns += itr.key();
    }
    res["columns"] = columns;

    auto rows = json::array();
    for (auto gr : m_groups) {
        rows += gr;
    }
    for (auto gr : m_childs) {
        rows += gr;
    }
    res["rows"] = rows;

    return res;
}


}

#endif // SQLITE_UTILS_HPP
