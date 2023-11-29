#include "query_parser.h"
//#include <regex>
#include <boost/regex.h>
#include <iostream>
using namespace arcirk::database::builder;
using namespace arcirk::query_builder_ui;


query_parser::query_parser(const object_t& dataStruct)
    : data_struct(dataStruct)
{
    m_is_valid = false;
}

bool query_parser::parse(const std::string &text)
{
    //qDebug() << text.c_str();

    m_is_valid = false;

    if(text.empty()){
        m_last_error = "Пустой текст запроса!";
        return false;
    }

    std::string query_text;

    //оставим только одни пробел
    only_one_space(text, query_text);
    //определим тип запроса
    query_type = type_query(query_text);
    if(query_type == query_type_INVALID){
        m_last_error = "Тип запроса не определен!";
        return false;
    }else if(query_type == Select){
        return parse_select(query_text);
    }

//    std::string::const_iterator start, end;
//    start = query_text.begin();
//    end = query_text.end();
//    boost::match_results<std::string::const_iterator> what;
//    boost::match_flag_type flags = boost::match_default;
//    while(boost::regex_search(start, end, what, regular, flags))
//    {
//        start = what[0].second;
//        qDebug() << what.position();
//    }

    //qDebug() index_of(query_text, "")

        //std::regex regular("(select|from)");

//        if(boost::regex_search(text.c_str(), result, regular)){
//            for (int i = 0; i < result.size(); ++i) {
//                qDebug() << result[i].str().c_str() << result[i].;
//            }
 //           for (auto itr = result.begin(); itr != result.end(); ++itr) {
    //qDebug() << itr->str().c_str() << itr.;
//            }
//        }
//    } catch (const std::exception& e) {
//        qCritical() << e.what();
//    }



//    m_last_error = "";
//    m_main_query_list = T_vec();
//    //разделим по словам
//    to_vector(m_main_query_list, text);

//    if(m_main_query_list.size() < 2){
//        m_last_error = "Ошибка чтения текста!";
//        return false;
//    }

//    int current_index = 0;

//    auto q_type = type_query(m_main_query_list, current_index);
//    if(q_type == query_type_INVALID){
//        m_last_error = "Не верный тип запроса!";
//        return false;
//    }

//    m_main_query = std::make_shared<query_builder>();


    return true;
}

std::string query_parser::last_error() const
{
    return m_last_error;
}

json query_parser::to_json() const
{
    if(!m_is_valid)
        return {};

    auto result = json::object();

    auto packet = query_builder_packet();
    packet.name = "Запрос 1";
    packet.parent = NIL_STRING_UUID;
    packet.ref = boost::to_string(arcirk::uuids::random_uuid());
    packet.type = (int)query_type;
    result["qbPackets"] = json::array({pre::json::to_json(packet)});

    auto fields_arr = json::array();
    if(query_type == sql_query_type::Select){

        for (int i = 0; i < m_qb_fileds.size(); ++i) {
            auto item = m_qb_fileds[i];
            item.package_ref = packet.ref;
            if(item.alias.empty())
                item.alias = item.name;

            fields_arr += pre::json::to_json(item);
        }
    }

    result["qbFields"] = fields_arr;

    return result;

}

void database::builder::query_parser::to_vector(T_vec &vec, const std::string &source, const std::string &sep)
{
    vec = arcirk::split(source, sep);
}

sql_query_type query_parser::type_query(const std::string& query_text)
{

    if(search_type_query(query_text, "select")){
        return sql_query_type::Select;
    }else if(search_type_query(query_text, "update")){
        return sql_query_type::Update;
    }else if(search_type_query(query_text, "insert")){
        return sql_query_type::Insert;
    }else if(search_type_query(query_text, "create")){
        return sql_query_type::Create;
    }


    return sql_query_type::query_type_INVALID;
}

bool query_parser::search_type_query(const std::string &query_text, const std::string &find_text)
{
    std::string expr = str_sample("^(?i)%1%", find_text);
    boost::regex regular(expr);
    boost::cmatch match;

    return boost::regex_search(query_text.c_str(), match, regular);

}

int query_parser::regexp_index_of(const std::string &query_text, const std::string &find_text)
{
    std::string expr = str_sample("\\b(?i)%1%\\b", find_text);
    boost::regex regular(expr);
    std::string::const_iterator start, end;
    start = query_text.begin();
    end = query_text.end();
    boost::match_results<std::string::const_iterator> what;
    boost::match_flag_type flags = boost::match_default;
    if(boost::regex_search(start, end, what, regular, flags))
    {
        return (int)what.position();
    }

    return -1;

}

bool query_parser::parse_select(const std::string &query_text)
{
    auto index = regexp_index_of(query_text, "from");
    if(index == -1){
        m_last_error = "Ошибка синтаксиса!";
        return false;
    }

    std::string fields_str = query_text.substr(6, index - 6);
    arcirk::trim(fields_str);
    verify_functions(fields_str);
    m_qb_fileds = set_fields(fields_str);

    m_is_valid = true;

    return true;
}

void query_parser::verify_functions(std::string &query_text)
{
    //1. найти вхождения '('
    //2. найти по index_of ')'
    //3. заменить все ',' на непечатоемый символ
    auto m_arr = find_all_occurrences(query_text, "\\(");

    const std::string source(query_text);

    std::string new_query;

    for (int i = 0; i < m_arr.size(); ++i) {
        auto start = m_arr[i];
        auto end = arcirk::index_of(source, ")", start);
        auto str = source.substr(start, end - start);
        boost::replace_all(str, ",", "\n" );
        query_text.replace(start, str.length(), str);
    }

}

object_array query_parser::set_fields(std::string &query_text)
{
    T_vec m_fields = arcirk::split(query_text, ",");
    object_array m_result{};

    for (int i = 0; i < m_fields.size(); ++i) {
        auto text = m_fields[i];
        bool is_fun = arcirk::index_of(text, "\n") != -1;
        if(is_fun)
            boost::replace_all(text, "\n", ", ");
        if(!is_fun){
            is_fun = arcirk::find_all_occurrences(text, " \\|\\| ").size() > 0;
        }

        auto rc = tree_model::ibase_object_structure();
        rc.ref = boost::to_string(arcirk::uuids::random_uuid());
        rc.object_type = "field";
        auto m_alias = arcirk::find_all_occurrences(text, " as ");
        if(m_alias.size() > 0){
            auto index= m_alias[0];
            rc.full_name = arcirk::left(text, index);
            rc.alias = arcirk::right(text, text.length() - index - 4);
            if(!is_fun){
                auto index_f = arcirk::index_of(rc.full_name, ".");
                if(index_f != -1){
                    rc.name = arcirk::right(rc.full_name, rc.full_name.length() - index_f - 1);
                }else
                    rc.name = rc.full_name;
                rc.user_query = 0;
                rc.parent_name = arcirk::left(rc.full_name, index_f);
            }else{
                rc.query = rc.full_name;
                rc.user_query = 1;
            }
        }else{
            rc.full_name = text;
            auto index_f = arcirk::index_of(rc.full_name, ".");
            if(index_f != -1){
                int sz =  (int)text.length() - index_f - 1;
                rc.name = arcirk::right(rc.full_name, sz);
            }else
                rc.name = rc.full_name;
            rc.alias = rc.name;
            rc.user_query = 0;
            rc.parent_name = arcirk::left(rc.full_name, index_f);
        }
        m_result.push_back(rc);
    }

    return m_result;
}

T_vec query_parser::get_fields(const T_vec &vec, int &current_ind)
{
    T_vec result{};

//    for (int i = current_ind; i < vec.size(); ++i) {
//        auto value = vec[i];
//        int is_fun = arcirk::in
//    }

    return result;
}
