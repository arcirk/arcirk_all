#ifndef QUERY_PARSER_H
#define QUERY_PARSER_H

#include "query_builder.hpp"
#include "global/arcirk_qt.hpp"

namespace arcirk::database::builder {

typedef std::map<std::string, tree_model::ibase_object_structure> details_t;
typedef std::map<std::string, details_t> object_t;
typedef std::pair<std::string, tree_model::ibase_object_structure> details_pair;
typedef std::pair<std::string, details_t> object_pair;
typedef std::vector<tree_model::ibase_object_structure> object_array;

    class query_parser
    {
    public:
        query_parser(const object_t& dataStruct);

        bool parse(const std::string& text);

        std::string last_error() const;

        json to_json() const;

        bool isValid(){return m_is_valid;};

    private:
        bool m_is_valid;
        std::string m_last_error;
        std::vector<sql_field> m_fields;
        std::vector<tree_model::ibase_object_structure> m_qb_fileds;
        T_vec m_main_query_list;
        std::shared_ptr<query_builder> m_main_query;
        sql_query_type query_type;
        const object_t& data_struct;


        void to_vector(T_vec& vec, const std::string& source, const std::string& sep = " ");
        sql_query_type type_query(const std::string& query_text);
        bool search_type_query(const std::string& query_text, const std::string& find_text);
        int regexp_index_of(const std::string& query_text, const std::string& find_text);

        bool parse_select(const std::string& query_text);
        void verify_functions(std::string& query_text);
        object_array set_fields(std::string& query_text);


        T_vec get_fields(const T_vec& vec, int& current_ind);
    };
}
#endif // QUERY_PARSER_H
