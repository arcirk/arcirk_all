#ifndef TREEVIEWMODEL_H
#define TREEVIEWMODEL_H

#include <QAbstractItemModel>
#include <QObject>
#include "shared_struct_qt.hpp"
#include <QIcon>
#include <QVector>
//#include "sortmodel.h"
#include "query_builder/ui/query_builder.hpp"
#include <QUuid>//>

//enum item_icons_enum{
//    ItemGroup = 0,
//    Item,
//    DeletedItemGroup,
//    DeletedItem
//};

using json = nlohmann::json;

namespace arcirk::tree_model {
    class TreeViewModel : public QAbstractItemModel
    {
        Q_OBJECT

    public:
        explicit TreeViewModel(const arcirk::client::client_conf& conf, QObject *parent = nullptr);
        explicit TreeViewModel(QObject *parent = nullptr);
        ~TreeViewModel();

        QModelIndex             index(int row, int column, const QModelIndex &parent) const override;
        QModelIndex             parent(const QModelIndex &child) const override;

        QHash<int, QByteArray>  roleNames() const override;
        QVector<QString>        columnNames();
        int                     rowCount(const QModelIndex &parent) const override;
        int                     columnCount(const QModelIndex &parent) const override;
        QVariant                data(const QModelIndex &index, int role) const override;
        bool                    setData(const QModelIndex &index, const QVariant &value, int role) override;
        QVariant                headerData(int section, Qt::Orientation orientation, int role) const override;
        bool                    canFetchMore(const QModelIndex &parent) const override;
        void                    fetchMore(const QModelIndex &parent) override;
        Qt::ItemFlags           flags(const QModelIndex &index) const override;
        bool                    hasChildren(const QModelIndex &parent) const override;

        QModelIndex             find(const QString& find_value, int col, const QModelIndex& parent) const;
        QModelIndex             find(const nlohmann::json& find_value, const QModelIndex& parent) const;
        //QModelIndex             find(const QUuid& uuid, const QModelIndex& parent, bool recursive = true);

        void fetchRoot(const QString& table_name, const QString& root_dir = "", const nlohmann::json& child_where = {});

        void fetch_recursive(const std::string& table_name,
                             const std::vector<std::string>& columns,
                             const std::string& hierarchical_column,
                             const nlohmann::json& default_parent,
                             const nlohmann::json& where_detailed = {});

        bool is_loaded();
        void set_column_aliases(const QMap<QString, QString> values);
        int get_column_index(const QString& name) const;
        QString get_column_name(int column) const;
        void set_establish_order(const QVector<QString>& names);
        void set_columns(const QVector<QString> cols);
        void set_server_object(arcirk::server::server_objects obj);
        arcirk::server::server_objects server_object() const;
        nlohmann::json to_object(const QModelIndex &index) const;
        nlohmann::json to_array(const QModelIndex &parent) const;
        nlohmann::json to_table_model(const QModelIndex &parent) const;
        void set_rows_icon(const QIcon &ico);
        void set_icon(const QModelIndex& index, const QIcon &ico);
        void set_icon(const QUuid& uuid, const QIcon &ico);

        void clear();
        void reset();

        void set_table(const nlohmann::json& tableModel);
        void add_table(const nlohmann::json& tableModel);

        QString current_parent_path() const;

        void refresh(const QModelIndex& parent);

        void remove(const QModelIndex &index);
        void add(const nlohmann::json object, const QModelIndex &parent = QModelIndex());
        void set_object(const QModelIndex &index, const nlohmann::json& object);

        void use_hierarchy(const std::string& column);

        static QModelIndex find_in_table(QAbstractItemModel * model, const QString& value, int column, int role = Qt::DisplayRole, bool findData = false);

        void move_up(const QModelIndex &index);
        void move_down(const QModelIndex &index);
        void move_to(const QModelIndex &index, const QModelIndex &new_parent);

        void set_group_only(bool value);
        bool group_only() const;

        void set_logical_icon(int column, QPair<QIcon, QIcon> icons);

        QVariant to_variant(const nlohmann::json& value) const;

        void set_sql_columns(const sql_values& values){
            sql_columns.clear();
            sql_columns = sql_values(values.size());
            std::copy(values.begin(), values.end(), sql_columns.begin());
        }

        void set_hierarchy_fields(const std::string& child, const std::string& parent){
            model_conf_.id_column = child;
            model_conf_.parent_id_column = parent;
        }

        int table_row_count();

        bool hierarchical_list(){
            return model_conf_.hierarchical_list;
        }

        void set_hierarchical_list(bool value){
            model_conf_.hierarchical_list = value;
        }

        hierarchical_list_conf& conf(){
            return model_conf_;
        }

        void set_default_parent(const nlohmann::json& value){
            auto ba = arcirk::string_to_byte_array(value.dump());
            model_conf_.default_parent = ByteArray(ba.size());
            std::copy(ba.begin(), ba.end(), model_conf_.default_parent.begin());
        }

        void set_limit_sql_rows(const int& count, const int& start = 0){
            m_max_rows = count;
            m_start_id = start;
        }

        void set_hierarchy_list_icon(item_icons_enum state, const QIcon& icon);

        const QUuid row_uuid(const QModelIndex& index);

    private:
        arcirk::server::server_objects server_object_;
        QVector<QString> columns_;

        sql_values sql_columns;

        struct NodeInfo;
        typedef QVector<NodeInfo> NodeInfoList;
        NodeInfoList _nodes;

        client::client_conf conf_;
        //QString table_name_;
        QByteArray authString_;
        QMap<QString, QString> column_aliases;
        bool is_loaded_;
        std::string use_hierarchy_;
        bool group_only_;
        QString current_parent_path_;
        QMap<int,QPair<QIcon, QIcon>> m_logical_icon;
        //nlohmann::json where_;

        //hierarchy_fields h_fileds;
        hierarchical_list_conf model_conf_;

        int m_start_id;
        int m_max_rows;

        bool columns_init();
        void set_columns_from_array(json columns_array);
        void set_rows_from_array(const bool& custom_cols, json rows_array, bool insert = false);

        void set_current_parent_path(const QString& value);

        QMap<item_icons_enum, QIcon> item_icons;
        QIcon rows_icon;
        QMap<QPair<int,int>, QIcon> row_icons;
        QMap<QUuid, QIcon> row_ico;

        int findRow(const NodeInfo* nodeInfo) const;
        QVariant nameData(const nlohmann::json& rowData, int role) const;
        QString cryptPass(const QString &source, const QString &key);
        nlohmann::json read_node_data(const nlohmann::json& parent);
        //std::string prepare_query(const std::string &parent);

        QVariant object_qt_variant(const nlohmann::json &object, int col = 0) const;
        nlohmann::json http_data(const QString &parentUuid) const;
        bool field_is_exists(const nlohmann::json &object, const std::string &name) const;

        QVariant firstData(const NodeInfo* nodeInfo, int role, const QModelIndex& index) const;

        static int get_number(const nlohmann::json& v){
            if(v.is_number())
                return v.get<int>();
            else if(v.is_string()){
                std::string s = v.get<std::string>();
                if(arcirk::is_number(s))
                    return std::stoi(s);
                else
                    return 0;
            }else
                return 0;
        }

        json node_data(const json& source, const bool &custom_cols){
            if(!custom_cols){
                return json(source);
            }else{
                json e_ = json::object();
                foreach (auto itr, columns_) {
                    e_[itr.toStdString()] = source.value(itr.toStdString(), json{});
                }
                return e_;
            }
        }

        NodeInfo* find_recursive(const std::string& ref, NodeInfoList& lst);

    signals:
        void fetch(const QModelIndex& parent);
        void canFetch(const QModelIndex& parent) const;

    };

}

#endif // TREEVIEWMODEL_H
