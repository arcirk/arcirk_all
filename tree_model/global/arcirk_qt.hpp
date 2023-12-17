#ifndef ARCIRK_QT_HPP
#define ARCIRK_QT_HPP


#include "arcirk.hpp"

#include <QtCore>
#include <QMap>
#include <QTreeView>
#include <QModelIndex>
#include <boost/variant.hpp>
#include <tuple>
#include <QSplitter>
#include <QWidget>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QRegularExpression>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlField>
#include <QIcon>
#include <QFileIconProvider>

#ifdef BOOST_SERIALIZATION
#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#endif

using json = nlohmann::json;
using namespace arcirk;

enum ref_type_enum{
    baseLink = 0,
    fileData,
    fileLink,
    stringArray
};

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::tree_model), ref_type,
    (std::string, uuid)
    (int, type)
    (std::string, representation)
    (ByteArray, data)
)

#ifndef IS_OS_ANDROID
BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::tree_model), query_inners_item,
    (int, _id)
    (int, line_num)
    (std::string, left_table)
    (std::string, left_table_ref)
    (std::string, right_table)
    (std::string, right_table_ref)
    (std::string, left_field)
    (std::string, left_field_ref)
    (std::string, right_field)
    (std::string, right_field_ref)
    (bool, left_all)
    (bool, right_all)
    (std::string, compare)
    (std::string, ref)
    (std::string, parent)
    (bool, user_query)
    (std::string, query)
    (std::string, package_ref)
    (int, is_group)
    )

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::tree_model), ibase_object_structure,
    (int, _id)
    (std::string, name)
    (std::string, alias)
    (std::string, full_name)
    (std::string, object_type)
    (std::string, data_type)
    (std::string, query)
    (bool, user_query)
    (std::string, ref)
    (std::string, parent)
    (std::string, base_ref)
    (std::string, base_parent)
    (std::string, parent_name)
    (std::string, parent_alias)
    (std::string, groupe_as)
    (std::string, package_ref)
    (std::string, def_value)
    (int, size)
    (int, is_group)
)

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::query_builder_ui), query_builder_packet,
    (int, _id)
    (int, line_num)
    (std::string, name)
    (std::string, ref)
    (std::string, parent)
    (int, type)
    (bool, use_limit)
    (int, row_limit)
    (bool, use_distinct)
    (int, is_group)
)

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::tree_model), profile_directory,
    (std::string, name)
    (std::string, path)
    (std::string, ref)
    (std::string, parent)
    (int, size)
    (int, is_group)
)

#endif

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::database), sqlite_functions_info,
    (std::string, name)
    (std::string, fun)
    (std::string, desc)
    (std::string, ref)
    (std::string, parent)
    (int, is_group)
    (int, deletion_mark)
    (int, version)
    );

inline QStringList sqlite_types_qt = {"INTEGER",
    "BIGINT",
    "DATE",
    "DATTETIME",
    "DECIMAL",
    "DOUBLE",
    "INT",
    "NONE",
    "NUMERIC",
    "REAL",
    "STRING",
    "TEXT",
    "TIME",
    "VARCHAR",
    "CHAR"
};


namespace arcirk::tree {

    enum user_role{
        UserRoleDef = Qt::UserRole,
        UserRoleExt = Qt::UserRole + 1,
        TextColorRole = Qt::UserRole + 2,
        ValidateTextRole = Qt::UserRole + 3,
        WidgetRole = Qt::UserRole + 4,
        ReadOnlyRole = Qt::UserRole + 5,
        FixedColumnWidthRole = Qt::UserRole + 6,
        RowCountRole = Qt::UserRole + 7,
        CompareWidgetLeftRole = Qt::UserRole + 8,
        CompareWidgetRightRole = Qt::UserRole + 9,
        WidgetStateRole = Qt::UserRole + 10,
        FormatTextRole = Qt::UserRole + 11,
        NotNullRole = Qt::UserRole + 12,
        WidgetInnerRole = Qt::UserRole + 13,
        UseRole = Qt::UserRole + 14,
        RepresentationRole = Qt::UserRole + 15,
        RawDataRole = Qt::UserRole + 16,
        EmptyRole = Qt::UserRole + 17
    };

    inline int user_roles_max(){
        return (int)EmptyRole - (int)Qt::UserRole;
    }

    enum item_editor_widget_roles{
        widgetLabelRole,
        widgetTextLineRole,
        widgetTextEditRole,
        widgetComboBoxRole,
        widgetCheckBoxRole,
        widgetCompareRole,
        widgetSpinBoxRole,
        widgetVariantRole,
        widgetINVALID= -1
    };

    enum tree_editor_inner_role{
        widgetNullType,
        widgetFilePath,
        widgetDirectoryPath,
        widgetColor,
        widgetText,
        widgetMultiText,
        widgetByteArray,
        widgetInteger,
        widgetArray,
        widgetBoolean,
        widgetInnerRoleINVALID=-1
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(tree_editor_inner_role, {
        {widgetInnerRoleINVALID, nullptr}    ,
        {widgetNullType, "widgetNullType"}  ,
        {widgetFilePath, "widgetFilePath"}  ,
        {widgetDirectoryPath, "widgetDirectoryPath"}  ,
        {widgetColor, "widgetColor"}  ,
        {widgetText, "widgetText"}  ,
        {widgetMultiText, "widgetMultiText"}  ,
        {widgetByteArray, "widgetByteArray"}  ,
        {widgetInteger, "widgetInteger"}  ,
        {widgetArray, "widgetArray"}  ,
        {widgetBoolean, "widgetBoolean"}  ,
    })

    enum attribute_use{
        forFolder,
        forFolderAndItem,
        forItem
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(item_editor_widget_roles, {
        {widgetINVALID, nullptr}    ,
        {widgetLabelRole, "widgetLabelRole"}  ,
        {widgetTextLineRole, "widgetTextLineRole"}  ,
        {widgetTextEditRole, "widgetTextEditRole"}  ,
        {widgetComboBoxRole, "widgetComboBoxRole"}  ,
        {widgetCheckBoxRole, "widgetCheckBoxRole"}  ,
        {widgetSpinBoxRole, "widgetSpinBoxRole"}  ,
        {widgetSpinBoxRole, "widgetSpinBoxRole"}  ,
        {widgetVariantRole, "widgetVariantRole"}  ,
    })

    enum item_icons_enum{
        ItemGroup = 0,
        Item,
        DeletedItemGroup,
        DeletedItem
    };


    inline void fill_property_values(json& receiver, const json& source){
        for (auto it = source.items().begin(); it != source.items().end(); ++it) {
            if(receiver.find(it.key()) != receiver.end()){
                receiver[it.key()] = it.value();
            }
        }
    }

    inline void set_splitter_size(QSplitter* splitter, QWidget* parent, int leftPersent){
        QRect rcParent = parent->geometry();
        QList<int> currentSizes = splitter->sizes();
        currentSizes[0] = rcParent.width() / 100 * leftPersent;
        currentSizes[1] = rcParent.width() / 100 * (100 - leftPersent);
        splitter->setSizes(currentSizes);
    }

    inline QUuid to_qt_uuid(const std::string& ref){
        return QUuid::fromString(ref.c_str());
    }

    inline QString quuid_to_string(const QUuid& uuid){
        return uuid.toString(QUuid::WithoutBraces);
    }

    inline QString generate_uuid(){return QUuid::createUuid().toString(QUuid::WithoutBraces);};

    inline QStringList to_string_list(const json& array){
        QStringList result;
        for (auto itr = array.begin(); itr != array.end(); ++itr) {
            std::string r = itr->get<std::string>();
            result.append(r.c_str());
        }
        return result;
    }

//    inline QByteArray byte_array_to_qt(const ByteArray& bt){
//        QByteArray* q_data = new QByteArray(reinterpret_cast<const char*>(data.data()), data.size());
//    }

} // namespace arcirk::tree

namespace arcirk::tree_model{

    inline QIcon system_icon(const QString& path, const QSize& size){
        auto m_file = path.split(QDir::separator());
        QString f_name;
        if(m_file.size() > 0)
            f_name = m_file[m_file.size() - 1];
        else
            f_name = path;

        auto file = QFile(QTemporaryDir().path() + QDir::separator() + f_name);
        if(file.open(QIODevice::WriteOnly)){
            file.write(0);
        }
        file.close();
        QFileIconProvider provider;
        QFileInfo f(file.fileName());
        auto ico = provider.icon(f).pixmap(size);
        file.remove();
        return ico;
    }

    inline QVariant to_variant(const json &value)
    {
            if(value.is_string())
                return QString::fromStdString(value.get<std::string>());
            else if(value.is_number_float())
                return value.get<double>();
            else if(value.is_number_integer())
                return value.get<int>();
            else if(value.is_boolean())
                return value.get<bool>();
            else if(value.is_array())
                return arcirk::tree::to_string_list(value);
            else
                return QVariant();
    }

    inline json to_json(const QVariant &value)
    {
            json val{};
            if(value.typeId() == QMetaType::QString){
                val = value.toString().toStdString();
            }else if(value.typeId() == QMetaType::Int){
                val = value.toInt();
            }else if(value.typeId() == QMetaType::Double){
                val = value.toDouble();
            }else if(value.typeId() == QMetaType::Float){
                val = value.toFloat();
            }else if(value.typeId() == QMetaType::Bool){
                val = value.toBool();
            }else if(value.typeId() == QMetaType::QStringList){
                val = json::array();
                foreach (auto itr, value.toStringList()) {
                    val += itr.toStdString();
                }
            }else
                val = "";

            return val;
    }

    inline json from_sql(QSqlDatabase& sql, const QString& query){
        QSqlQuery rs(sql);
        rs.exec(query);
        auto result = json::array();
        while (rs.next()) {
            QSqlRecord row = rs.record();
            json j_row{};
            for (int i = 0; i < row.count(); ++i) {
                std::string column_name = row.fieldName(i).toStdString();
                QVariant val = row.field(i).value();

                if(val.userType() == QMetaType::QString)
                    j_row[column_name] = val.toString().toStdString();
                else if(val.userType() == QMetaType::Double)
                    j_row[column_name] = val.toDouble();
                else if(val.userType() == QMetaType::Int)
                    j_row[column_name] = val.toInt();
                else if(val.userType() == QMetaType::LongLong)
                    j_row[column_name] = val.toLongLong();
                else if(val.userType() == QMetaType::ULongLong)
                    j_row[column_name] = val.toULongLong();
                else
                    j_row[column_name] = "";
            }
            result += j_row;
        }
        return result;
    }


    inline bool executeQueryPackade(const QByteArray &qf, QSqlDatabase &db) {

            bool result = true;

            QString queryStr(qf);
            //Check if SQL Driver supports Transactions
            if(db.driver()->hasFeature(QSqlDriver::Transactions)) {
            //Replace comments and tabs and new lines with space
            queryStr = queryStr.replace(QRegularExpression("(\\/\\*(.|\\n)*?\\*\\/|^--.*\\n|\\t|\\n)", QRegularExpression::CaseInsensitiveOption|QRegularExpression::MultilineOption), " ");
            //Remove waste spaces
            queryStr = queryStr.trimmed();

            //Extracting queries
            QStringList qList = queryStr.split(';', Qt::SkipEmptyParts);

            //Initialize regular expression for detecting special queries (`begin transaction` and `commit`).
            //NOTE: I used new regular expression for Qt5 as recommended by Qt documentation.
            QRegularExpression re_transaction("\\bbegin.transaction.*", QRegularExpression::CaseInsensitiveOption);
            QRegularExpression re_commit("\\bcommit.*", QRegularExpression::CaseInsensitiveOption);

            //Check if query file is already wrapped with a transaction
            bool isStartedWithTransaction = re_transaction.match(qList.at(0)).hasMatch();
            if(!isStartedWithTransaction)
                db.transaction();     //<=== not wrapped with a transaction, so we wrap it with a transaction.

            //Execute each individual queries
            foreach(const QString &s, qList) {
                if(re_transaction.match(s).hasMatch())    //<== detecting special query
                    db.transaction();
                else if(re_commit.match(s).hasMatch())    //<== detecting special query
                    db.commit();
                else {
                    auto query = QSqlQuery(db);
                    query.exec(s);                        //<== execute normal query
                    if(query.lastError().type() != QSqlError::NoError) {
                        qDebug() << query.lastError().text();
                        db.rollback();
                        result = false;//<== rollback the transaction if there is any problem
                    }
                }
            }
            if(!isStartedWithTransaction)
                db.commit();          //<== ... completing of wrapping with transaction

            //Sql Driver doesn't supports transaction
            } else {

            //...so we need to remove special queries (`begin transaction` and `commit`)
            queryStr = queryStr.replace(QRegularExpression("(\\bbegin.transaction.*;|\\bcommit.*;|\\/\\*(.|\\n)*?\\*\\/|^--.*\\n|\\t|\\n)", QRegularExpression::CaseInsensitiveOption|QRegularExpression::MultilineOption), " ");
            queryStr = queryStr.trimmed();

            //Execute each individual queries
            QStringList qList = queryStr.split(';', Qt::SkipEmptyParts);
            foreach(const QString &s, qList) {
                auto query = QSqlQuery(db);
                query.exec(s);
                if(query.lastError().type() != QSqlError::NoError) {
                    qDebug() << query.lastError().text();
                    result = false;
                }
            }
            }

            return result;
    }

#ifdef BOOST_SERIALIZATION
    class data_ref
    {
        private:
        friend class boost::serialization::access;
        std::string uuid;
        std::string synonim;
        std::string table;

        template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
        {
            ar & uuid;
            ar & synonim;
            ar & table;
        }
        public:
        data_ref(){}
        data_ref(const std::string& uuid_, const std::string& synonim_, const std::string& table_):
            uuid(uuid_), synonim(synonim_), table(table_)
        {}
    };
    //BOOST_CLASS_VERSION(data_ref, 1);
#endif

    struct root_tree_conf : public std::enable_shared_from_this<root_tree_conf>{
        public:
            enum typeConnection{
                noConnection = 0,
                sqlIteConnection,
                httpConnection,
                sqlIteMemoryConnection,
                serverDirectorias
            };
            explicit root_tree_conf(){
            #ifndef IS_OS_ANDROID
                Q_INIT_RESOURCE(resurce);
            #endif
                m_size                  = QSize(22, 22);
                m_columns               = QList<QString>{"ref", "parent, is_group"};
                m_fetch_expand          = false;
                m_hierarchical_list     = true;
                m_type_connection       = noConnection;
                m_enable_rows_icons     = true;
                m_database_changed      = false;
                m_database              = nullptr;
                m_use_binary_links      = false;
                m_http_conf = http::http_conf();
                enable_drag_and_drop(false);
                m_row_icons.insert(tree::item_icons_enum::ItemGroup, QIcon(":/img/group.png"));
                m_row_icons.insert(tree::item_icons_enum::Item, QIcon(":/img/item.png"));
                m_row_icons.insert(tree::item_icons_enum::DeletedItemGroup, QIcon(":/img/groupDeleted.png"));
                m_row_icons.insert(tree::item_icons_enum::DeletedItem, QIcon(":/img/deletionMarkItem.png"));
                m_logical_icon_def      = qMakePair(QIcon(":/img/checkbox-checked.png"), QIcon(":/img/checkbox-unchecked.png"));
                user_data_init();
            };

#ifdef BOOST_SERIALIZATION
            static char * ref_to_buffer(const data_ref& data){
                //std::stringstream ofs;
                boost::asio::streambuf ofs;
                //boost::archive::text_oarchive oa(ofs);
                boost::archive::binary_oarchive oa(ofs);
                oa << data;
                const size_t sz = sizeof(data_ref);
                char * buff = new char[sz];
                oa.save_binary(buff, sizeof(buff));
                return buff;
            }

            static data_ref ref_from_buffer(const char * buff){
                data_ref result;
                std::istringstream ifs(buff);
                //boost::archive::text_iarchive ia(ifs);
                boost::archive::binary_iarchive ia(ifs);
                //boost::archive::binary_iarchive ia();
                //a.load_binary(buff, sizeof(data_ref));
                ia >> result;
                return result;
            }
#endif

            QSize size() const{return m_size;};
            void set_section_size(int section, const QSize& size){
                Q_UNUSED(section);
                m_size = size;
            }

            QList<QString> columns() const{return m_columns;};

            QString column_name(int index) const {
                Q_ASSERT(m_columns.size() > index);
                Q_ASSERT(index>=0);
                return m_columns[index];
            }

            QString column_name_(int index, bool alias = false) const {
                Q_ASSERT(m_columns.size() > index);
                Q_ASSERT(index>=0);
                if(!alias)
                    return m_columns[index];
                else{
                    auto itr = m_column_aliases.find(m_columns[index]);
                    if(itr != m_column_aliases.end())
                        return itr.value();
                    else
                        return m_columns[index];
                }
            }

            int column_index(const QString& name){return m_columns.indexOf(name);}

            void set_header_data(int section, const QVariant &value, int role){
                auto c_name = column_name(section);
                auto itr = m_header_data.find(c_name);
                if(itr == m_header_data.end())
                    m_header_data.insert(c_name, QMap<int, QVariant>{qMakePair(role, value)});
                else{
                    itr.value().insert(role, value);
                }
            }

            QVariant header_data(int section, int role) const{
                auto c_name = column_name(section);
                auto itr = m_header_data.find(c_name);
                if(itr != m_header_data.end()){
                    auto res = itr.value().find(role);
                    if(res != itr.value().constEnd()){
                        return itr->value(role);
                    }else
                        return QVariant();

                }else
                    return QVariant();

            }

            bool fetch_expand() {return m_fetch_expand;};

            void set_fetch_expand(bool value){m_fetch_expand = value;};

            void set_root_data(const json& data){

                if(!data.is_object())
                    return;

                m_root_data = data;
                m_columns.clear();

                if(m_root_data.is_object()){
                    for (auto itr = m_root_data.items().begin(); itr != m_root_data.items().end(); ++itr) {
                        auto obj = *itr;
                        m_columns.append(obj.key().c_str());
                    }
                    if(m_columns.indexOf("ref")==-1)
                        m_columns.append("ref");
                    if(m_columns.indexOf("parent")==-1)
                        m_columns.append("parent");
                }

                user_data_init();
            }

            void reset_columns(const json& cols){
                Q_ASSERT(cols.is_array());
                auto dt = json::object();
                for (auto itr = cols.begin(); itr != cols.end(); ++itr) {
                    auto c_name = *itr;
                    if(!c_name.is_string())
                        throw arcirk::NativeException("Не верный тип массива!");
                    dt[c_name.get<std::string>()] = json{};
                }

                set_root_data(dt);
            }

            void set_columns_order(const QList<QString>& names){

                auto m_cols = m_columns;
                m_columns = names;
                foreach (auto it, m_cols) {
                    if(m_columns.indexOf(it) ==-1)
                        m_columns.append(it);
                }
                m_custom_cols = true;
                m_columns_order = names;
            }

            QList<QString> columns_order() const {return m_columns_order;};

            bool custom_cols() {return m_custom_cols;};
            void set_hierarchical_list(bool value){m_hierarchical_list = value;};
            bool hierarchical_list(){return m_hierarchical_list;};

            typeConnection type_connection() const {return m_type_connection;};
            void set_type_connection(typeConnection value){m_type_connection = value;};

            QString connection_string() {return m_connection_string;};
            void set_connection_string(const QString& value){m_connection_string = value;};

            QString table_name(){return m_table_name;};
            void set_table_name(const QString& value){m_table_name = value;};

            bool read_only(){return m_read_only;};
            void set_read_only(bool value){m_read_only = value;};

            void set_enable_drag(bool value){m_drag = value;};
            void set_enable_drop(bool value){m_drop = value;};
            void enable_drag_and_drop(bool value){
                m_drag = value;
                m_drop = value;
            }
            bool enable_drag(){return m_drag;};
            bool enable_drop(){return m_drop;};

            QPair<QIcon, QIcon> logical_icons() const {return m_logical_icon_def;};
            QMap<tree::item_icons_enum, QIcon> rows_icons() const{ return m_row_icons;};
            void set_rows_icons(tree::item_icons_enum state, const QIcon &icon)
            {
                m_row_icons[state] = icon;
            }
            QIcon rows_icon(tree::item_icons_enum state){
                return m_row_icons[state];
            }

            bool enable_rows_icons(){return m_enable_rows_icons;};
            void set_enable_rows_icons(bool value){m_enable_rows_icons = value;};

            void set_column_widget(const QString& column, tree::item_editor_widget_roles role){
                m_widgets.insert(column, role);
            }

            tree::item_editor_widget_roles column_widget(const QString& column){
                if(m_widgets.find(column) != m_widgets.end())
                    return m_widgets[column];
                else
                    return tree::item_editor_widget_roles::widgetINVALID;
            }

            void set_user_data(const QString& column, const QVariant& value, tree::user_role role){
                auto itr = m_user_data.find(role);
                if(itr != m_user_data.end()){
                    auto it = itr.value().find(column);
                    if(it != itr.value().end()){
                        m_user_data[role][column] = value;
                    }else{
                        m_user_data[role].insert(column, value);
                    }
                }
            }

            QVariant user_data(const QString& column, tree::user_role role){
                auto itr = m_user_data.find(role);
                if(itr != m_user_data.end()){
                    auto it = itr.value().find(column);
                    if(it != itr.value().end()){
                        return m_user_data[role][column];
                    }
                }
                return QVariant();
            }

            QMap<tree::user_role, QMap<QString, QVariant>> user_data_values() const{
                return m_user_data;
            }

            void set_http_conf(const QString& host, const QString& token, const QString& table){m_http_conf = http::http_conf(host.toStdString(), token.toStdString(), table.toStdString());}

            http::http_conf http_conf() const {return m_http_conf;};

            void set_column_aliases(const QMap<QString, QString> &aliases)
            {
                m_column_aliases = aliases;
            }

            const QMap<QString, QString> columns_aliases()
            {
                return m_column_aliases;
            }

            std::string id_column() const {return "ref";};
            std::string parent_column() const {return "parent";};

            void set_user_sql_where(const json value){m_user_where = value;};
            json user_sql_where(){return m_user_where;};

            void set_database_changed(bool value){m_database_changed = value;};
            bool is_database_changed()const {return m_database_changed;};

            QSqlDatabase* get_database(){return m_database;};
            void set_database(QSqlDatabase* value){m_database = value;};

            QMap<QString, QString> column_aliases_default() const{

                QMap<QString, QString> m_colAliases{};
                m_colAliases.insert("uuid", "ID");
                m_colAliases.insert("session_uuid", "ID сессии");
                m_colAliases.insert("name", "Имя");
                m_colAliases.insert("uuid_user", "ID пользователя");
                m_colAliases.insert("user_uuid", "ID пользователя");
                m_colAliases.insert("app_name", "Приложение");
                m_colAliases.insert("user_name", "Имя пользователя");
                m_colAliases.insert("ip_address", "IP адрес");
                m_colAliases.insert("address", "IP адрес");
                m_colAliases.insert("host_name", "Host");
                m_colAliases.insert("Ref", "Ссылка");
                m_colAliases.insert("ref", "Ссылка");
                m_colAliases.insert("FirstField", "Имя");
                m_colAliases.insert("SecondField", "Представление");
                m_colAliases.insert("first", "Имя");
                m_colAliases.insert("second", "Представление");
                m_colAliases.insert("privateKey", "Контейнер");
                m_colAliases.insert("_id", "Иднекс");
                m_colAliases.insert("sid", "SID");
                m_colAliases.insert("cache", "cache");
                m_colAliases.insert("subject", "Кому выдан");
                m_colAliases.insert("issuer", "Кем выдан");
                m_colAliases.insert("container", "Контейнер");
                m_colAliases.insert("notValidBefore", "Начало действия");
                m_colAliases.insert("parentUser", "Владелец");
                m_colAliases.insert("notValidAfter", "Окончание дейтствия");
                m_colAliases.insert("serial", "Серийный номер");
                m_colAliases.insert("volume", "Хранилище");
                m_colAliases.insert("cache", "Кэш");
                m_colAliases.insert("role", "Роль");
                m_colAliases.insert("device_id", "ID устройства");
                m_colAliases.insert("ipadrr", "IP адрес");
                m_colAliases.insert("product", "Продукт");
                m_colAliases.insert("typeIp", "Тип адреса");
                m_colAliases.insert("defPort", "Стандартный порт");
                m_colAliases.insert("notFillWindow", "Оконный режим");
                m_colAliases.insert("password", "Пароль");
                m_colAliases.insert("width", "Ширина");
                m_colAliases.insert("height", "Высота");
                m_colAliases.insert("port", "Порт");
                m_colAliases.insert("user", "Пользователь");
                m_colAliases.insert("updateUser", "Обновлять учетку");
                m_colAliases.insert("start_date", "Дата подключения");
                m_colAliases.insert("organization", "Организация");
                m_colAliases.insert("subdivision", "Подразделение");
                m_colAliases.insert("warehouse", "Склад");
                m_colAliases.insert("price", "Тип цен");
                m_colAliases.insert("workplace", "Рабочее место");
                m_colAliases.insert("device_type", "Тип устройства");
                m_colAliases.insert("is_group", "Это группа");
                m_colAliases.insert("size", "Размер");
                m_colAliases.insert("rows_count", "Количество записей");
                m_colAliases.insert("interval", "Интервал");
                m_colAliases.insert("start_task", "Начало выполнения");
                m_colAliases.insert("end_task", "Окончание выполнения");
                m_colAliases.insert("allowed", "Разрешено");
                m_colAliases.insert("predefined", "Предопределенный");
                m_colAliases.insert("days_of_week", "По дням недели");
                m_colAliases.insert("synonum", "Представление");
                m_colAliases.insert("script", "Скрипт");
                m_colAliases.insert("parent_user", "Владелец");
                m_colAliases.insert("not_valid_before", "Начало действия");
                m_colAliases.insert("not_valid_after", "Окончание дейтствия");
                m_colAliases.insert("type", "Тип");
                m_colAliases.insert("host", "Хост");
                m_colAliases.insert("system_user", "Пользователь ОС");
                m_colAliases.insert("info_base", "Информационная база");
                m_colAliases.insert("parent", "Владелец");
                m_colAliases.insert("data_type", "Тип данных");
                m_colAliases.insert("path", "Путь");

                return m_colAliases;
            }

        private:
            QSize                               m_size;
            QList<QString>                      m_columns;
            QMap<QString, QMap<int, QVariant>>  m_header_data;
            QMap<QString, QString>              m_column_aliases;
            bool                                m_fetch_expand;
            json                                m_root_data;
            QList<QString>                      m_columns_order;
            bool                                m_custom_cols;
            bool                                m_hierarchical_list;
            QString                             m_connection_string;
            typeConnection                      m_type_connection;
            QString                             m_table_name;
            bool                                m_read_only;
            bool                                m_drag;
            bool                                m_drop;
            bool                                m_enable_rows_icons;
            QPair<QIcon, QIcon>                 m_logical_icon_def; //логическое представление ячейки
            QMap<tree::item_icons_enum, QIcon>  m_row_icons; //представление в иерархии по умолчанию
            //QMap<QUuid, QIcon> user_data_row_icon; //установленный пользователем значек ячейки
            QMap<QString, tree::item_editor_widget_roles>   m_widgets;
            QMap<tree::user_role, QMap<QString, QVariant>>  m_user_data;
            http::http_conf                           m_http_conf;
            json                                m_user_where;
            bool                                m_database_changed;
            QSqlDatabase*                       m_database;
            bool                                m_use_binary_links;

            void user_data_init()
            {
                m_user_data.clear();
                for (int i = 0; i < tree::user_roles_max(); ++i) {
                    QMap<QString, QVariant> m_val{};
                    m_user_data.insert((tree::user_role)(Qt::UserRole +i), QMap<QString, QVariant>());
                    foreach (auto column, m_columns) {
                        //m_val.insert(column, QVariant());
                        m_user_data[(tree::user_role)(Qt::UserRole +i)].insert(column, QVariant());
                    }
                    //m_user_data.insert((tree::user_role)(Qt::UserRole +i), m_val);
                }

            }

    };

}

#endif // ARCIRK_QT_HPP
