#ifndef ARCIRK_HPP
#define ARCIRK_HPP

#ifdef _WINDOWS
#pragma warning(disable:4100)
#pragma warning(disable:4267)
//#pragma comment(lib, "advapi32")
#endif

#include <iostream>
#include <utility>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

#include <boost/asio.hpp>
#include <boost/variant.hpp>
#include <boost/exception/all.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/detail/sha1.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/regex.hpp>
#include <boost/uuid/detail/md5.hpp>
#include <boost/algorithm/hex.hpp>

#include <boost/format.hpp>

//#ifndef QML_APPLICATION
#include <boost/locale.hpp>
#include <boost/filesystem.hpp>
//#endif

#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/adapted/struct/define_struct.hpp>
#include <boost/fusion/include/define_struct.hpp>
#include <nlohmann/json.hpp>
#include <pre/json/from_json.hpp>
#include <pre/json/to_json.hpp>

static constexpr time_t const NULL_TIME = -1;

using json = nlohmann::json;

#define WS_RESULT_SUCCESS "success"
#define WS_RESULT_ERROR "error"
#define NIL_STRING_UUID "00000000-0000-0000-0000-000000000000"

//#if defined(_WIN32)
//#undef UNICODE
//#include <winsock2.h>
//// Headers that need to be included after winsock2.h:
//#include <iphlpapi.h>
//#include <ws2ipdef.h>

//#pragma comment(lib, "Iphlpapi.lib")

//typedef IP_ADAPTER_UNICAST_ADDRESS_LH Addr;
//typedef IP_ADAPTER_ADDRESSES *AddrList;

typedef unsigned char BYTE;
typedef std::vector<BYTE> ByteArray;

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::http), http_param,
    (std::string, command)
    (std::string, param)    //base64 строка
    )

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::http), http_conf,
    (std::string, host)
    (std::string, token)
    (std::string, table)
    )
BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::client), version_application,
    (int, major)
    (int, minor)
    (int, path)
    )

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::synchronize), http_query_sql_param,
    (std::string, table_name)
    (std::string, query_type)
    )

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::query_builder_ui), query_builder_main,
    (int, _id)
    (std::string, name)
    (std::string, ref)
    (std::string, path)
    (std::string, parent)
    (std::string, data_type)
    (int, is_group)
)

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::database), table_info_sqlite,
    (int, cid)
    (std::string, name)
    (std::string, type)
    (int, notnull)
    (std::string, dflt_value)
    (int, bk)
    );
//BOOST_FUSION_DEFINE_STRUCT(
//    (arcirk::query_builder_ui), query_builder_databases,
//    (int, _id)
//    (std::string, name)
//    (std::string, ref)
//    (std::string, path)
//    (std::string, parent)
//    (int, is_group)
//)

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::query_builder_ui), query_builder_querias,
    (int, _id)
    (std::string, ref)
    (std::string, parent)
    (ByteArray, data)
    (std::string, query_ref)
    (std::string, database_ref)
    (int, is_group)
    )
namespace arcirk {

typedef std::string T_str;
typedef std::vector<T_str> T_vec;
typedef std::vector<std::pair<T_str, T_str>> T_list;

//#ifndef QML_APPLICATION
inline std::string to_local(const std::string& source){
#ifdef BOOST_LOCALE
#ifdef _WINDOWS
    return boost::locale::conv::from_utf(source, "windows-1251");
#else
    return source;
#endif
#else
    return source;
#endif
}
inline std::string to_utf(const std::string& source, const std::string def_charset = "Windows-1251"){
#ifdef BOOST_LOCALE
#ifdef _WINDOWS
    return boost::locale::conv::to_utf<char>(source, def_charset);
#else
    return source;
#endif
#else
    return source;
#endif
}
inline std::string from_utf(const std::string& source){
#ifdef BOOST_LOCALE
#ifdef BOOST_WINDOWS
    return boost::locale::conv::from_utf(source, "windows-1251");
#else
    return source;
#endif
#else
    return source;
#endif
}
//#endif
inline std::string get_sha1(const std::string& p_arg)
{
    boost::uuids::detail::sha1 sha1;
    sha1.process_bytes(p_arg.data(), p_arg.size());
    unsigned hash[5] = {0};
    sha1.get_digest(hash);

    // Back to string
    char buf[41] = {0};

    for (int i = 0; i < 5; i++)
    {
        std::sprintf(buf + (i << 3), "%08x", hash[i]);
    }

    return std::string(buf);
}
inline std::string get_hash(const std::string& first, const std::string& second){
    std::string _usr(first);
    const std::string& _pwd(second);

    boost::trim(_usr);
    boost::to_upper(_usr);

    return get_sha1(_usr + _pwd);
}

inline long int tz_offset(time_t when = NULL_TIME)
{
    if (when == NULL_TIME)
        when = std::time(nullptr);
    auto const tm = *std::localtime(&when);
    std::ostringstream os;
    os << std::put_time(&tm, "%z");
    std::string s = os.str();
    // s is in ISO 8601 format: "±HHMM"
    int h = std::stoi(s.substr(0,3), nullptr, 10);
    int m = std::stoi(s[0]+s.substr(3), nullptr, 10);

    return (h-1) * 3600 + m * 60;
}
inline tm current_date() {
    using namespace std;
    tm current{};
    time_t t = time(nullptr);
#ifdef _WINDOWS
    localtime_s(&current, &t);
#else
    localtime_r(&t, &current);
#endif
    return current;
}

inline tm seconds_to_date(const long int& dt){
    using namespace std;
    time_t t = (time_t)dt;
    tm current{};
#ifdef _WINDOWS
    localtime_s(&current, &t);
#else
    localtime_r(&t, &current);
#endif

    return current;
}

inline long int date_to_seconds(const tm& dt, bool offset) {

    tm current = dt;
    time_t t = time(nullptr);

#ifdef _WIN32
    localtime_s(&current, &t);
#else
    localtime_r(&t, &current);
#endif

    std::chrono::system_clock::time_point tp = std::chrono::system_clock::from_time_t(mktime(&current));

    auto i_offset = offset ? tz_offset() : 0;

    return
        (long int)std::chrono::duration_cast<std::chrono::seconds>(
            tp.time_since_epoch()).count() + i_offset;

}
inline long int add_day(const long int& dt, const int& quantity){
    return dt + (quantity * (60*60*24));
}
inline long int start_day(const std::tm& d){
    auto sec = d.tm_sec + (d.tm_min * 60) + (d.tm_hour * 60 * 60);
    return date_to_seconds(d, false) - sec;
}
inline long int end_day(const std::tm& d){
    auto sec = 60 * 60 * 24 - 1;
    return start_day(d) + sec;
}


inline void fail(const std::string& what, const std::string& error, bool conv = true, const std::string& log_folder = "logs"){

#ifdef BOOST_FILESYSTEM
    const std::tm tm = arcirk::current_date();
    char cur_date[100];
    std::strftime(cur_date, sizeof(cur_date), "%c", &tm);

    std::string res = std::string(cur_date);
    res.append(" " + what + ": ");
    if(conv)
        res.append(arcirk::to_local(error));
    else
        res.append(error);

    std::cerr << res << std::endl;

    if(log_folder.empty())
        return;

    namespace fs = boost::filesystem;

    fs::path log_dir(log_folder);
    log_dir /= "errors";
    if(!fs::exists(log_dir)){
        try {
            fs::create_directories(log_dir);
        } catch (const std::exception &e) {
            std::cerr << e.what() << std::endl;
            return;
        }
    }
    char date_string[100];
    strftime(date_string, sizeof(date_string), "%u_%m_%Y", &tm);

    fs::path file = log_dir / (std::string(date_string) + ".log");

    std::ofstream out;			// поток для записи
    out.open(file.string(), std::ios::app); 		// открываем файл для записи
    if (out.is_open())
    {
        out << res << '\n';
    }
    out.close();
#else
    std::cerr << what << ":" << error << std::endl;
#endif

};
inline void log(const std::string& what, const std::string& message, bool conv = true, const std::string& log_folder = "logs"){

#ifdef BOOST_FILESYSTEM
    const std::tm tm = arcirk::current_date();
    char cur_date[100];
    std::strftime(cur_date, sizeof(cur_date), "%c", &tm);

    std::string res = std::string(cur_date);
    res.append(" " + what + ": ");
    if(conv)
        res.append(arcirk::to_local(message));
    else
        res.append(message);

    std::cout << res << std::endl;

    if(log_folder.empty())
        return;

    namespace fs = boost::filesystem;

    fs::path log_dir(log_folder);
    log_dir /= "days";
    if(!fs::exists(log_dir)){
        try {
            fs::create_directories(log_dir);
        } catch (const std::exception &e) {
            std::cerr << e.what() << std::endl;
            return;
        }
    }
    char date_string[100];
    strftime(date_string, sizeof(date_string), "%u_%m_%Y", &tm);

    fs::path file = log_dir / (std::string(date_string) + ".log");
    std::ofstream out;			// поток для записи
    out.open(file.string(), std::ios::app); 		// открываем файл для записи
    if (out.is_open())
    {
        out << res  << '\n';
    }
    out.close();
#else
    std::cout << what << ":" << message << std::endl;
#endif
};


inline int split_str_to_vec(const T_str& s, const T_str& DELIM, T_vec& v)
{
    size_t l, r;

    for (l = s.find_first_not_of(DELIM), r = s.find_first_of(DELIM, l);
         l != std::string::npos; l = s.find_first_not_of(DELIM, r), r = s.find_first_of(DELIM, l))
        v.push_back(s.substr(l, r - l));
    return (int)v.size();
}
inline T_vec split(const T_str& line, const T_str& sep)
{
    T_vec  v;

    split_str_to_vec(line, sep, v);

    return v;
}
inline void trim(std::string& source){ boost::trim(source);};
inline void to_upper(std::string& source){boost::to_upper(source);};
inline void to_lower(std::string& source){boost::to_lower(source);};
inline std::string sample(const std::string& format_string, const std::vector<std::string>& args){
    boost::format f(format_string);
    for (auto it = args.begin(); it != args.end(); ++it) {
        f % *it;
    }
    return f.str();
}
template<typename... Arguments>
inline std::string str_sample(const std::string& format_string, const Arguments&... args){return boost::str((boost::format(format_string) % ... % args));}
inline std::string left(const std::string &source, const std::string::size_type& count){
    return  source.substr(0, count);
}
inline std::string right(const std::string &source, const int& count){
    return  source.substr((int)source.length() - count, count);
}
inline void only_one_space(const std::string& source, std::string& out){
    boost::regex regular("\\s+");
    std::ostringstream res(std::ios::out);
    std::ostream_iterator<char> ores(res);

    boost::regex_replace( ores, source.begin(), source.end(), regular,
                         " ", boost::match_default | boost::format_all );
    out = res.str();
}
inline int index_of(const std::string& original_string, const std::string& source, int start = 0){
    auto find = original_string.find(source, start);
    if(find == std::string::npos)
        return - 1;
    else
        return (int)find;
}
inline std::vector<int> find_all_occurrences(const std::string& source, const std::string& find_str){
    std::vector<int> result{};
    std::string expr = str_sample("(?i)(%1%)", find_str);
    boost::regex regular(expr);
    std::string::const_iterator start, end;
    start = source.begin();
    end = source.end();
    boost::match_results<std::string::const_iterator> what;
    boost::match_flag_type flags = boost::match_default;
    while(boost::regex_search(start, end, what, regular, flags))
    {
        start = what[0].second;
        int m = 0;
        if(result.size() > 0)
            m = result[result.size() - 1];
        result.push_back((int)what.position() + 1 + m);
    }
    return result;
}

inline void* crypt_t(void* data, unsigned data_size, void* key, unsigned key_size)
{
    assert(data && data_size);
    if (!key || !key_size) return data;

    auto* kptr = (uint8_t*)key; // начало ключа
    uint8_t* eptr = kptr + key_size; // конец ключа

    for (auto* dptr = (uint8_t*)data; data_size--; dptr++)
    {
        *dptr ^= *kptr++;
        if (kptr == eptr) kptr = (uint8_t*)key; // переход на начало ключа
    }
    return data;
}
inline std::string crypt(const std::string &source, const std::string& key) {
    if(source.empty())
        return {};
    try {
#ifdef _WINDOWS
        std::string s  = arcirk::from_utf(source);
        std::string p  = arcirk::from_utf(key);
        std::vector<char> source_(s.c_str(), s.c_str() + s.size() + 1);
        std::vector<char> key_(p.c_str(), p.c_str() + p.size() + 1);
        void* text = std::data(source_);
        void* pass = std::data(key_);
        crypt_t(text, source_.size(), pass, key_.size());
        std::string result(arcirk::to_utf((char*)text));
        return result;
#else
        //#define ARR_SIZE(x) (sizeof(x) / sizeof(x[0]))
        std::vector<char> source_(source.c_str(), source.c_str() + source.size() + 1);
        std::vector<char> key_(key.c_str(), key.c_str() + key.size() + 1);
        void* text = std::data(source_);
        void* pass = std::data(key_);
        //crypt_t(text, ARR_SIZE(source.c_str()), pass, ARR_SIZE(key.c_str()));
        crypt_t(text, source_.size(), pass, key_.size());
        std::string result((char*)text);
        return result;
#endif
    } catch (const std::exception &e) {
        fail(__FUNCTION__, e.what());
        return {};
    }

}

inline void write_file(const std::string& filename, ByteArray& file_bytes){
    std::ofstream file(filename, std::ios::out|std::ios::binary);
    std::copy(file_bytes.cbegin(), file_bytes.cend(),
              std::ostream_iterator<unsigned char>(file));
}
inline void read_file(const std::string &filename, ByteArray &result)
{
    FILE * fp = fopen(filename.c_str(), "rb");

    fseek(fp, 0, SEEK_END);
    size_t flen= ftell(fp);
    fseek(fp, 0, SEEK_SET);

    std::vector<unsigned char> v (flen);

    fread(&v[0], 1, flen, fp);

    fclose(fp);

    result = v;
}

inline T_list parse_section_ini(const std::string& source, const std::string& sep = "\n"){
    T_vec vec = split(source, sep);

    T_list result;

    for (auto itr = vec.begin(); itr != vec.end() ; ++itr) {
        auto index = itr->find_first_of('=');
        if(index != std::string::npos){
            auto first = left(*itr, index);
            auto second = right(*itr, index + 1);
            result.push_back(std::pair<T_str,T_str>(first, second));
        }
    }
    return result;
}

template<typename T>
inline std::string enum_synonym(T value){
    using n_json = nlohmann::json;
    return n_json(value).get<std::string>();
};
template<typename T>
inline nlohmann::json values_from_param(const nlohmann::json& param){
    if(param.empty())
        return {};
    T e = T();
    auto source = pre::json::to_json(e);
    nlohmann::json result = {};

    if(source.is_object()){
        for (auto itr = source.begin(); itr != source.end() ; ++itr) {
            auto i = param.find(itr.key());
            if( i != param.end()){
                result[itr.key()] = i.value();
            }
        }
        return result;
    }else
        return {};
}
inline std::string type_string(nlohmann::json::value_t t){
    using json = nlohmann::json;
    if(t == json::value_t::null) return "null";
    else if(t == json::value_t::boolean) return "boolean";
    else if(t == json::value_t::number_integer) return "number_integer";
    else if(t == json::value_t::number_unsigned) return "number_unsigned";
    else if(t == json::value_t::number_float) return "number_float";
    else if(t == json::value_t::object) return "object";
    else if(t == json::value_t::array) return "array";
    else if(t == json::value_t::string) return "string";
    else if(t == json::value_t::binary) return "binary";
    else return "undefined";
}
template<typename T>
inline T secure_serialization(const nlohmann::json &source, const std::string& fun = "")
{
    using json = nlohmann::json;

    if(!source.is_object())
        return T();

    try {
        auto result = pre::json::from_json<T>(source);
        return result;
    }catch (const std::exception& e){
        std::cerr << __FUNCTION__ << " " << fun << " " << e.what() << std::endl;
    }

    nlohmann::json object = pre::json::to_json(T());

    for (auto it = source.items().begin(); it != source.items().end(); ++it) {
        if(object.find(it.key()) != object.end()){
            if(it.value().type() == object[it.key()].type()){
                object[it.key()] = it.value();
            }else{
                if(object[it.key()].is_boolean()){
                    if(it.value().type() == json::value_t::number_unsigned &&
                        (object[it.key()].type() == json::value_t::number_integer ||
                         object[it.key()].type() == json::value_t::number_float)){
                        auto val = it.value().get<int>();
                        object[it.key()] = val == 0 ? false : true; // it.value();
                    }else
                        object[it.key()] = false;
                }else{
                    if(it.value().type() == json::value_t::number_unsigned &&
                        (object[it.key()].type() == json::value_t::number_integer ||
                         object[it.key()].type() == json::value_t::number_float)){
                        object[it.key()] = it.value();

                    }else{
                        std::cerr << fun << " " << __FUNCTION__ << " Ошибка проверки по типу ключа: " << it.key().c_str() << std::endl;
                                                                                                             std::cerr << it.value() << " " << type_string(it.value().type()) << " " << type_string(object[it.key()].type()) <<  std::endl;
                    }
                }
            }
        }
    }

    return pre::json::from_json<T>(object);
}
template<typename T>
inline T secure_serialization(const std::string &source, const std::string& fun = "")
{
    using json = nlohmann::json;
    if(!json::accept(source))
        return T();

    try {
        auto result = secure_serialization<T>(json::parse(source), fun);
        return result;
    } catch (std::exception& e) {
        if(fun.empty())
            std::cerr << __FUNCTION__ << " " << e.what() << std::endl;
        else
            std::cerr << __FUNCTION__ << " " << fun << " " << e.what() << std::endl;
    }
    return T();
}
template<typename T>
inline T internal_structure(const std::string &name, const nlohmann::json &source)
{
    auto result = T();
    try {
        auto obg = source.value(name, nlohmann::json::object());
        if(!obg.empty())
            result = pre::json::from_json<T>(obg);
    } catch (const std::exception& e) {
        std::cerr <<  e.what() << std::endl;
    }

    return result;
}

inline std::string byte_array_to_string(const ByteArray& data){
    if(data.empty())
        return "";
    return std::string(data.begin(), data.end());
}

inline ByteArray string_to_byte_array(const std::string& str){
    return ByteArray(str.begin(), str.end());
}

inline json table_from_row(const json& row){
    if(!row.is_object())
        return {};

    auto columns = json::array();
    auto rows = json::array();
    rows += row;
    for (auto it = row.items().begin(); it != row.items().end(); ++it) {
        columns += it.key();
    }
    json table = json::object();
    table["columns"]  = columns;
    table["rows"] = rows;
    return table;
}

namespace uuids{
    inline bool is_valid_uuid(std::string const& maybe_uuid, boost::uuids::uuid& result) {
        using namespace boost::uuids;

        try {
            result = string_generator()(maybe_uuid);
            return result.version() != uuid::version_unknown;
        } catch(...) {
            return false;
        }
    }
    inline boost::uuids::uuid string_to_uuid(const std::string& sz_uuid, bool random_uuid = false){

        if (sz_uuid == NIL_STRING_UUID && random_uuid)
            return boost::uuids::random_generator()();

        boost::uuids::uuid uuid{};

        if (is_valid_uuid(sz_uuid, uuid)){
            return uuid;
        }

        if (!random_uuid)
            return boost::uuids::nil_uuid();
        else
            return boost::uuids::random_generator()();
    }
    inline std::string nil_string_uuid(){
        return "00000000-0000-0000-0000-000000000000";
    }
    inline boost::uuids::uuid nil_uuid(){
        return boost::uuids::nil_uuid();
    }
    inline std::string uuid_to_string(const boost::uuids::uuid& uuid){
        try {
            return boost::to_string(uuid);
        } catch(...) {
            return NIL_STRING_UUID;
        }
    }
    inline boost::uuids::uuid random_uuid(){
        return boost::uuids::random_generator()();
    }
    inline ByteArray to_byte_array(const boost::uuids::uuid& uuid){
        ByteArray v(uuid.size());
        std::copy(uuid.begin(), uuid.end(), v.begin());
        return v;
    }
    inline boost::uuids::uuid from_byte_array(const ByteArray& byte){
        boost::uuids::uuid u{};
        memcpy(&u, byte.data(), byte.size());
        return u;
    }
    inline std::string digest_to_string(const boost::uuids::detail::md5::digest_type &digest)
    {
        const auto charDigest = reinterpret_cast<const char *>(&digest);
        std::string result;
        boost::algorithm::hex(charDigest, charDigest + sizeof(boost::uuids::detail::md5::digest_type), std::back_inserter(result));
        return result;
    }
    inline std::string  to_md5(const std::string& source){
        using boost::uuids::detail::md5;
        md5 hash;
        md5::digest_type digest;

        hash.process_bytes(source.data(), source.size());
        hash.get_digest(digest);
        return digest_to_string(digest);
    }
    inline boost::uuids::uuid md5_to_uuid(const std::string& md5_string){
        using boost::uuids::detail::md5;
        using namespace boost::uuids;
        try {
            std::string source = md5_string;
            source = source.insert(8, "-");
            source = source.insert(13, "-");
            source = source.insert(18, "-");
            source = source.insert(23, "-");
            auto result = string_generator()(source);
            return result;
        } catch(...) {
            return boost::uuids::nil_uuid();
        }
    }
}

#ifndef IS_OS_ANDROID
namespace standard_paths{
    inline std::string home(){
        using namespace boost::filesystem;
    #ifdef _WINDOWS
        path p(arcirk::to_utf(getenv("USERPROFILE")));
        return p.string();
    #else
        return getenv("HOME");
    #endif
    }
    inline std::string home_roaming_dir(){
        using namespace boost::filesystem;
    #ifdef _WINDOWS
        path p(arcirk::to_utf(getenv("APPDATA")));
        return p.string();
    #else
        return home();
    #endif
    }
    inline std::string temporary_dir(){
        return boost::filesystem::temp_directory_path().string();
    }
    inline std::string home_local_dir(){
        using namespace boost::filesystem;
    #ifdef _WINDOWS
        path p(arcirk::to_utf(getenv("LOCALAPPDATA")));
        return p.string();
    #else
        return home();
    #endif
    }
    inline std::string program_data_dir(){
        using namespace boost::filesystem;
    #ifdef _WINDOWS
        path p(arcirk::to_utf(getenv("ProgramData")));
        return p.string();
    #else
        return home();
    #endif
    }
    inline bool verify_directory(const std::string& dir_path) {
        using namespace boost::filesystem;
        path p(arcirk::to_local(dir_path));

        if (!exists(p)) {
                try {
                return boost::filesystem::create_directories(p);
                }catch (std::exception& e){
                std::cerr << e.what() << std::endl;
                return false;
                }
        }
        return true;

    }
    inline bool verify_directory(const boost::filesystem::path& dir_path) {
        using namespace boost::filesystem;
        path p(arcirk::to_local(dir_path.string()));
        if (!exists(p)) {
                try {
                return boost::filesystem::create_directories(p);
                }catch (std::exception& e){
                std::cerr << e.what() << std::endl;
                return false;
                }

        }
        return true;
    }
    inline std::string app_conf_dir(const std::string& app_name, bool mkdir_is_not_exists){
        boost::filesystem::path app_conf(home_roaming_dir());
        app_conf /= app_name;
        if (mkdir_is_not_exists)
                verify_directory(app_conf);
        return app_conf.string();
    }
    inline std::string server_conf_dir(const std::string& app_name, bool mkdir_is_not_exists){
        boost::filesystem::path app_conf(program_data_dir());
        app_conf /= app_name;
        if (mkdir_is_not_exists)
                verify_directory(app_conf);
        return app_conf.string();
    };
}
#endif

namespace base64{

    using std::wcout;
    using std::wstring;
    using std::vector;

    inline const std::string base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

    inline const std::string base64_padding[] = {"", "==", "="};
    inline bool byte_is_base64(BYTE c) {
        return (isalnum(c) || (c == '+') || (c == '/'));
    }
    inline std::string byte_to_base64(BYTE const* buf, unsigned int bufLen) {
        std::string ret;
        int i = 0;
        int j = 0;
        BYTE char_array_3[3];
        BYTE char_array_4[4];

        while (bufLen--) {
                char_array_3[i++] = *(buf++);
                if (i == 3) {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for(i = 0; (i <4) ; i++)
                ret += base64_chars[char_array_4[i]];
                i = 0;
                }
        }

        if (i)
        {
                for(j = i; j < 3; j++)
                char_array_3[j] = 0;

                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for (j = 0; (j < i + 1); j++)
                ret += base64_chars[char_array_4[j]];

                while((i++ < 3))
                ret += '=';
        }

        return ret;
    }
    inline ByteArray base64_to_byte(std::string const& encoded_string) {
        int in_len = encoded_string.size();
        int i = 0;
        int j = 0;
        int in_ = 0;
        BYTE char_array_4[4], char_array_3[3];
        std::vector<BYTE> ret;

        while (in_len-- && ( encoded_string[in_] != '=') && byte_is_base64(encoded_string[in_])) {
                char_array_4[i++] = encoded_string[in_]; in_++;
                if (i ==4) {
                for (i = 0; i <4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for (i = 0; (i < 3); i++)
                ret.push_back(char_array_3[i]);
                i = 0;
                }
        }

        if (i) {
                for (j = i; j <4; j++)
                char_array_4[j] = 0;

                for (j = 0; j <4; j++)
                char_array_4[j] = base64_chars.find(char_array_4[j]);

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for (j = 0; (j < i - 1); j++) ret.push_back(char_array_3[j]);
        }

        return ret;
    }
    inline std::string base64_encode(const std::string &s) {
        namespace bai = boost::archive::iterators;

        try {
                std::stringstream os;

                // convert binary values to base64 characters
                typedef bai::base64_from_binary
                    // retrieve 6 bit integers from a sequence of 8 bit bytes
                    <bai::transform_width<const char *, 6, 8>> base64_enc; // compose all the above operations in to a new iterator

                std::copy(base64_enc(s.c_str()), base64_enc(s.c_str() + s.size()),
                          std::ostream_iterator<char>(os));

                os << base64_padding[s.size() % 3];
                return os.str();
        }catch (std::exception& e){
                std::cerr << "error: " << e.what() << std::endl;
                return "";
        }

    }
    inline std::string base64_decode(const std::string &s) {
        namespace bai = boost::archive::iterators;

        try {
                std::stringstream os;

                typedef bai::transform_width<bai::binary_from_base64<const char *>, 8, 6> base64_dec;

                auto size = (unsigned int)s.size();

                // Remove the padding characters, cf. https://svn.boost.org/trac/boost/ticket/5629
                if (size && s[size - 1] == '=') {
                --size;
                if (size && s[size - 1] == '=') --size;
                }
                if (size == 0) return {};

                std::copy(base64_dec(s.data()), base64_dec(s.data() + size),
                          std::ostream_iterator<char>(os));

                return os.str();

        }catch (std::exception& e){
                std::cerr << "error: " << e.what() << std::endl;
                return "";
        }
    }
    inline bool is_base64(const std::string& source){
        boost::regex rx("[^a-zA-Z0-9+/=]");
        boost::smatch xResults;
        if(!boost::regex_search(source, xResults, rx, boost::match_extra) && (source.length()%4) == 0 && source.length()>=4){
                return true;
        }

        return false;
    }
}


namespace url {
    struct uri
    {
    public:
        std::string query_string, path, protocol, host, port, basic_auth, url;

        static uri Parse(const std::string &url_string)
        {

            uri result;

            typedef std::string::const_iterator iterator_t;

            if (url_string.length() == 0)
                return result;

            result.url = url_string;

            iterator_t uriEnd = url_string.end();

            // get query start
            iterator_t queryStart = std::find(url_string.begin(), uriEnd, '?');

            // protocol
            iterator_t protocolStart = url_string.begin();
            iterator_t protocolEnd = std::find(protocolStart, uriEnd, ':');            //"://");

            if (protocolEnd != uriEnd)
            {
                std::string prot = &*(protocolEnd);
                if ((prot.length() > 3) && (prot.substr(0, 3) == "://"))
                {
                    result.protocol = std::string(protocolStart, protocolEnd);
                    protocolEnd += 3;   //      ://
                }
                else
                    protocolEnd = url_string.begin();  // no protocol
            }
            else
                protocolEnd = url_string.begin();  // no protocol

            if(result.protocol != "file"){
                //basic auth
                iterator_t authStart = protocolEnd;
                iterator_t authEnd = std::find(authStart, uriEnd, '@');
                if(authEnd != uriEnd){
                    std::string auth_string = std::string(authStart, authEnd);
                    T_vec v_auth = split(auth_string, ":");
                    std::string usr = v_auth[0];
                    std::string pwd;
                    if(v_auth.size() > 1){
                        pwd = v_auth[1];
                    }
                    std::string base64 = base64::base64_encode(usr + ":" + pwd);
                    result.basic_auth = std::string("Basic ") + base64;
                    authEnd += 1;
                }else
                    authEnd = authStart;

                // host
                iterator_t hostStart = authEnd;
                iterator_t pathStart = std::find(hostStart, uriEnd, '/');  // get pathStart

                iterator_t hostEnd = std::find(authEnd,
                                               (pathStart != uriEnd) ? pathStart : queryStart,
                                               ':');  // check for port

                result.host = std::string(hostStart, hostEnd);

                // port
                if ((hostEnd != uriEnd) && ((&*(hostEnd))[0] == ':'))  // we have a port
                {
                    hostEnd++;
                    iterator_t portEnd = (pathStart != uriEnd) ? pathStart : queryStart;
                    result.port = std::string(hostEnd, portEnd);
                }

                if(result.port.empty()){
                    if(result.protocol == "http")
                        result.port = "80";
                    else if(result.protocol == "https")
                        result.port = "443";
                }
                // path
                if (pathStart != uriEnd)
                    result.path = std::string(pathStart, queryStart);

                // query
                if (queryStart != uriEnd)
                    result.query_string = std::string(queryStart, url_string.end());
            }else{
                result.path = std::string(protocolEnd, uriEnd);
            }

            return result;

        }   // Parse
    };  // uri

//    class bIp{
//        public:
//            static boost::asio::ip::address_v6 sinaddr_to_asio(sockaddr_in6 *addr);
//            static std::vector<boost::asio::ip::address> get_local_interfaces();
//            static std::string get_default_host(const std::string& def_host,  const std::string& seg = "192.168");
//        };
//        boost::asio::ip::address_v6 bIp::sinaddr_to_asio(sockaddr_in6 *addr) {
//            boost::asio::ip::address_v6::bytes_type buf;
//            memcpy(buf.data(), addr->sin6_addr.s6_addr, sizeof(addr->sin6_addr));
//            return boost::asio::ip::make_address_v6(buf, addr->sin6_scope_id);
//        }

//        std::vector<boost::asio::ip::address> bIp::get_local_interfaces() {
//            // It's a windows machine, we assume it has 512KB free memory
//            DWORD outBufLen = 1 << 19;
//            AddrList ifaddrs = (AddrList) new char[outBufLen];
//            struct ifaddrs *ifaddr;

//            std::vector<boost::asio::ip::address> res;

//            ULONG err = GetAdaptersAddresses(AF_UNSPEC,
//                                             GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_DNS_SERVER, NULL, ifaddrs,
//                                             &outBufLen);

//            if (err == NO_ERROR) {
//            for (AddrList addr = ifaddrs; addr != 0; addr = addr->Next)
//            {
//                if (addr->OperStatus != IfOperStatusUp) continue;
//                // if (addr->NoMulticast) continue;

//                // Find the first IPv4 address
//                if (addr->Ipv4Enabled) {
//                    for (Addr *uaddr = addr->FirstUnicastAddress; uaddr != 0; uaddr = uaddr->Next) {
//                        if (uaddr->Address.lpSockaddr->sa_family != AF_INET) continue;
//                        res.push_back(boost::asio::ip::make_address_v4(ntohl(reinterpret_cast<sockaddr_in *>(uaddr->Address.lpSockaddr)->sin_addr.s_addr)));
//                    }
//                }

//                if (addr->Ipv6Enabled) {
//                    for (Addr *uaddr = addr->FirstUnicastAddress; uaddr != 0; uaddr = uaddr->Next) {
//                        if (uaddr->Address.lpSockaddr->sa_family != AF_INET6) continue;
//                        res.push_back(sinaddr_to_asio(reinterpret_cast<sockaddr_in6 *>(uaddr->Address.lpSockaddr)));
//                    }
//                }
//            }
//            } else {

//            }
//            delete[]((char *)ifaddrs);
//            return res;
//        }
//        #elif defined(__APPLE__) || defined(__linux__)
//        #include <arpa/inet.h>
//        #include <ifaddrs.h>
//        #include <net/if.h>
//        #include <sys/types.h>

//            std::vector<boost::asio::ip::address> bIp::get_local_interfaces() {
//                std::vector<boost::asio::ip::address> res;
//                ifaddrs *ifs;
//                if (getifaddrs(&ifs)) {
//                return res;
//                }
//                for (auto addr = ifs; addr != nullptr; addr = addr->ifa_next) {
//                // No address? Skip.
//                if (addr->ifa_addr == nullptr) continue;

//                // Interface isn't active? Skip.
//                if (!(addr->ifa_flags & IFF_UP)) continue;

//                if(addr->ifa_addr->sa_family == AF_INET) {
//                    res.push_back(boost::asio::ip::make_address_v4(ntohl(
//                        reinterpret_cast<sockaddr_in *>(addr->ifa_addr)->sin_addr.s_addr)));
//                } else if(addr->ifa_addr->sa_family == AF_INET6) {
//                    res.push_back(sinaddr_to_asio(reinterpret_cast<sockaddr_in6 *>(addr->ifa_addr)));
//                } else continue;
//                }
//                freeifaddrs(ifs);
//                return res;
//            }
//        #else
//        #error "..."
//        #endif

//            std::string bIp::get_default_host(const std::string& def_host, const std::string& seg) {

//                if (def_host == "localhost" || def_host == "127.0.0.1"  || def_host == "0.0.0.0") {
//                std::vector<boost::asio::ip::address> ip_vec = get_local_interfaces();

//                for (auto iter: ip_vec) {
//                    if (iter.to_string().substr(0, seg.length()) == seg)
//                        return iter.to_string();
//                }
//                }
//                return def_host;
//            }
//        }


//namespace command_liner{
//class cmd_parser{
//public:
//    cmd_parser (int &argc, char **argv);
//    const std::string& option(const std::string &option) const;
//    bool option_exists(const std::string &option) const;
//private:
//    std::vector <std::string> tokens;
//};
//cmd_parser::cmd_parser(int &argc, char **argv){
//    for (int i=1; i < argc; ++i)
//        this->tokens.emplace_back(argv[i]);
//}

//const std::string& cmd_parser::option(const std::string &option) const{
//    std::vector<std::string>::const_iterator itr;
//    itr =  std::find(this->tokens.begin(), this->tokens.end(), option);
//    if (itr != this->tokens.end() && ++itr != this->tokens.end()){
//        return *itr;
//    }
//    static const std::string empty_string;
//    return empty_string;
//}

//bool cmd_parser::option_exists(const std::string &option) const{
//    return std::find(this->tokens.begin(), this->tokens.end(), option)
//           != this->tokens.end();
//}
//}

}

    class NativeException : public std::exception
    {
    public:
        explicit NativeException(const char *msg) : message(msg) {}
        virtual ~NativeException() throw() {}
        virtual const char *what() const throw() { return message.c_str(); }
    protected:
        const std::string message;
    };
}
#endif // ARCIRK_HPP
