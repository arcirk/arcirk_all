#ifndef QUERYBUILDERGENERATOR_H
#define QUERYBUILDERGENERATOR_H

#include "global/arcirk_qt.hpp"
#include "query_builder.hpp"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

using namespace arcirk::database::builder;

namespace arcirk::query_builder_ui {
    class QueryBuilderGenerator : public std::enable_shared_from_this<QueryBuilderGenerator>
    {
    public:
        explicit QueryBuilderGenerator();
        ~QueryBuilderGenerator(){};

        bool from_database(const QString& connection_name, const std::string& query_ref = NIL_STRING_UUID);

    private:
        QSqlDatabase m_db;
        std::vector<std::shared_ptr<arcirk::database::builder::query_builder>> m_querias;
        bool load_from_database(const std::string& query_ref);

        template<typename T>
        T to_std_struct(QSqlQuery& rc);

        std::vector<tree_model::ibase_object_structure> sql_rows(const std::string& packet_id);

    };
}

#endif // QUERYBUILDERGENERATOR_H
