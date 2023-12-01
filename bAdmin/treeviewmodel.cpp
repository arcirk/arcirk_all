#include "treeviewmodel.h"
#include "query_builder/ui/query_builder.hpp"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>
#include <QException>

namespace arcirk::tree_model {
    QString TreeViewModel::cryptPass(const QString &source, const QString &key)
    {
        std::string result = arcirk::crypt(source.toStdString(), key.toStdString());

        return QString::fromStdString(result);
    }

    nlohmann::json TreeViewModel::http_data(const QString &parentUuid) const{

        std::string uuid_form_ = arcirk::uuids::nil_string_uuid();

        nlohmann::json param = {
                {"table", true},
                {"uuid_form", uuid_form_},
                {"empty_column", false},
                {"recursive", false},
                {"parent", parentUuid.toStdString()}
        };

        auto http_param = arcirk::synchronize::http_param();
        http_param.command = arcirk::enum_synonym(arcirk::server::server_commands::ProfileDirFileList);
        http_param.param = QByteArray(param.dump().data()).toBase64().toStdString();

        QEventLoop loop;
        int httpStatus = 200;
        QByteArray httpData;
        QNetworkAccessManager httpService;

        auto finished = [&loop, &httpStatus, &httpData](QNetworkReply* reply) -> void
        {
           QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
           if(status_code.isValid()){
               httpStatus = status_code.toInt();
               if(httpStatus != 200){
                    qCritical() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__ << "Error: " << httpStatus << " " + reply->errorString() ;
               }else
               {
                   httpData = reply->readAll();
               }
           }
           loop.quit();

        };

        loop.connect(&httpService, &QNetworkAccessManager::finished, finished);

        QUrl ws(conf_.server_host.data());
        QString protocol = ws.scheme() == "wss" ? "https://" : "http://";
        QString http_query = "/api/info";
        QUrl url(protocol + ws.host() + ":" + QString::number(ws.port()) + http_query);
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QString headerData = "Token " + authString_;
        request.setRawHeader("Authorization", headerData.toLocal8Bit());

        httpService.post(request, QByteArray::fromStdString(pre::json::to_json(http_param).dump()));

        loop.exec();

        if(httpStatus != 200){
            return WS_RESULT_ERROR;
        }

        if(httpData.isEmpty())
            return WS_RESULT_ERROR;

        if(httpData == "error"){
            return WS_RESULT_ERROR;
        }

        auto msg = pre::json::from_json<arcirk::server::server_response>(httpData.toStdString());

        if(msg.result.empty())
            return WS_RESULT_ERROR;

        if(msg.result == WS_RESULT_ERROR)
            return WS_RESULT_ERROR;

        auto http_result = nlohmann::json::parse(QByteArray::fromBase64(msg.result.data()).toStdString());

        return http_result;
    }

    nlohmann::json TreeViewModel::read_node_data(const nlohmann::json &parent)
    {
        //qDebug() << __FUNCTION__;

        using json = nlohmann::json;

        if(model_conf_.table_name.empty())
            return {};

        using namespace arcirk::database;
        auto query = builder::query_builder();

        auto where_root = nlohmann::json::object();
        if(model_conf_.hierarchical_list){
            where_root[model_conf_.parent_id_column] = parent;
        }

        auto str = arcirk::byte_array_to_string(model_conf_.where_detailed);
        json where_{};
        if(json::accept(str))
           where_ = json::parse(str);

        if(!model_conf_.with_recursive && model_conf_.where_detailed.size() > 0){

            for (auto itr = where_.items().begin(); itr != where_.items().end(); ++itr) {
                where_root[itr.key()] = itr.value();
            }
        }

        if(group_only_){
            where_root["is_group"] = 1;
        }

//        if(sql_columns.size() == 0){
//            foreach (auto &itr, columns_) {
//                sql_columns.emplace_back(builder::sql_value(itr.toStdString(), "", {}));
//            }
//        }

        std::string query_text;

        if(!model_conf_.with_recursive){
            query_text = query.select(sql_columns).from(model_conf_.table_name).where(where_root, true).prepare();
        }else{
            query_text = query.with_recursive(model_conf_.table_name, sql_columns, where_root, where_, "is_group");
        }

        if(m_max_rows > 0){
            query_text.append("\nLIMIT ");
            query_text.append(std::to_string(m_max_rows));
            if(m_start_id > 0){
                query_text.append(" OFFSET ");
                query_text.append(std::to_string(m_start_id));
            }
        }

        qDebug() << __FUNCTION__ << qPrintable(query_text.c_str());

        auto param = nlohmann::json::object();
        param["query_text"] = query_text;

        auto http_param = arcirk::synchronize::http_param();
        http_param.command = arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery);
        http_param.param = QByteArray(param.dump().data()).toBase64().toStdString();

        QEventLoop loop;
        int httpStatus = 200;
        QByteArray httpData;
        QNetworkAccessManager httpService;

        auto finished = [&loop, &httpStatus, &httpData](QNetworkReply* reply) -> void
        {
           QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
           if(status_code.isValid()){
               httpStatus = status_code.toInt();
               if(httpStatus != 200){
                    qCritical() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__ << "Error: " << httpStatus << " " + reply->errorString() ;
               }else
               {
                   httpData = reply->readAll();
               }
           }
           loop.quit();

        };

        loop.connect(&httpService, &QNetworkAccessManager::finished, finished);

        QUrl ws(conf_.server_host.data());
        QString protocol = ws.scheme() == "wss" ? "https://" : "http://";
        QString http_query = "/api/info";
        QUrl url(protocol + ws.host() + ":" + QString::number(ws.port()) + http_query);
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QString headerData = "Token " + authString_;
        request.setRawHeader("Authorization", headerData.toLocal8Bit());

        httpService.post(request, QByteArray::fromStdString(pre::json::to_json(http_param).dump()));
        loop.exec();

        if(httpStatus != 200){
            return WS_RESULT_ERROR;
        }

        if(httpData.isEmpty())
            return WS_RESULT_ERROR;

        if(httpData == "error"){
            return WS_RESULT_ERROR;
        }

        auto msg = pre::json::from_json<arcirk::server::server_response>(httpData.toStdString());

        if(msg.result.empty())
            return WS_RESULT_ERROR;

        if(msg.result == WS_RESULT_ERROR)
            return WS_RESULT_ERROR;

        auto http_result = nlohmann::json::parse(QByteArray::fromBase64(msg.result.data()).toStdString());

        return http_result;
    }

//    std::string TreeViewModel::prepare_query(const std::string &parent)
//    {
//        using namespace arcirk::database;
//        using json = nlohmann::json;
//        auto query = builder::query_builder();

//        bool is_group_by_column = model_conf_.id_column == model_conf_.parent_id_column;

//        if(sql_columns.size() == 0 && columns_.size() > 0){
//            foreach (auto &itr, columns_) {
//                std::string field = itr.toStdString();
//                if(is_group_by_column && (field == "ref" || field == "parent" || field == "is_group")){
//                    continue;
//                }
//                auto sql_v = builder::sql_value();
//                sql_v.alias = field;
//                if(is_group_by_column){
//                    sql_v.value = "";
//                    sql_v.field_name = "";
//                }else{
//                    sql_v.value = field;
//                    sql_v.field_name = field;
//                }
//                sql_columns.emplace_back(sql_v);
//            }
//            if(is_group_by_column){
//                sql_columns.insert(sql_columns.begin(), builder::sql_value("", "ref", parent));
//                sql_columns.insert(sql_columns.begin() + 1, builder::sql_value("", "is_group", 1));
//                sql_columns.insert(sql_columns.begin() + 2, builder::sql_value("", "parent", ""));
//            }

//        }

//        sql_values child_fields{};

//        if(is_group_by_column && sql_columns.size() == 0 && columns_.size() > 0){
//            foreach (auto &itr, columns_) {
//                std::string field = itr.toStdString();
//                if(is_group_by_column && (field == "ref" || field == "parent" || field == "is_group")){
//                    continue;
//                }
//                auto sql_v = builder::sql_value();
//                sql_v.field_name = field;
//                sql_v.alias = field;
//                child_fields.emplace_back(sql_v);
//            }
//            child_fields.insert(sql_columns.begin(), builder::sql_value(model_conf_.id_column, "ref", ""));
//            child_fields.insert(sql_columns.begin() + 1, builder::sql_value("", "is_group", 0));
//            child_fields.insert(sql_columns.begin() + 2, builder::sql_value("", "parent", parent));
//        }

//        auto where_ = json::object();

//        if(!is_group_by_column){
//            where_[model_conf_.parent_id_column] = parent;
//        }

//        if(!where_.empty()){
//            for (auto itr = where_.items().begin(); itr != where_.items().end(); ++itr) {
//                where_[itr.key()] = itr.value();
//            }
//        }

//        if(group_only_){
//            where_["is_group"] = 1;
//        }

//        std::string query_text;

//    //    if(h_fileds.child_id != h_fileds.parent_id){
//    //        if(sql_columns.size() == 0){
//                query_text = query.select(sql_columns).from(table_name_.toStdString()).where(where_, true)
//                                 .order_by(nlohmann::json{{"is_group", 1}}).prepare();
//    //        }else{
//    //            query_text = query.select(sql_columns).from(table_name_.toStdString()).where(where_root, true).order_by(nlohmann::json{{"is_group", 1}}).prepare();
//    //        }
//    //    }else{
//    //        query_text =
//    //    }

//                return query_text;
//    }

    QVariant TreeViewModel::object_qt_variant(const nlohmann::json &object, int col) const
    {
        Q_ASSERT(col < columns_.size());
        auto val = object[columns_[col].toStdString()];
        if(val.is_string())
            return QString::fromStdString(val.get<std::string>());
        else if(val.is_number_float())
            return val.get<double>();
        else if(val.is_number_integer())
            return val.get<int>();
        else if(val.is_boolean())
            return val.get<bool>();
        else
            return QVariant();
    }

    TreeViewModel::TreeViewModel(const arcirk::client::client_conf& conf, QObject *parent)
        : QAbstractItemModel{parent},
          conf_(conf)
    {
        model_conf_.type_query = TYPE_REQUEST_TABLE;
        model_conf_.id_column = "ref";
        model_conf_.parent_id_column = "parent";

        set_limit_sql_rows(0);
//        set_start_id(0);

        QByteArray data = QByteArray(conf.hash.data()).toBase64();
        authString_ = data;
        item_icons.insert(item_icons_enum::ItemGroup, QIcon(":/img/group.png"));
        item_icons.insert(item_icons_enum::Item, QIcon(":/img/item.png"));
        item_icons.insert(item_icons_enum::DeletedItemGroup, QIcon(":/img/groupDeleted.png"));
        item_icons.insert(item_icons_enum::DeletedItem, QIcon(":/img/deletionMarkItem.png"));

        is_loaded_ = false;
        group_only_ = false;

        column_aliases = {};
        server_object_ = arcirk::server::server_objects::OBJ_INVALID;
        _nodes = {};
        auto ba = arcirk::string_to_byte_array(nlohmann::json(NIL_STRING_UUID).dump());
        model_conf_.default_parent = ByteArray(ba.size());
        std::copy(ba.begin(), ba.end(), model_conf_.default_parent.begin());
        model_conf_.hierarchical_list = true;

    }

    TreeViewModel::TreeViewModel(QObject *parent)
        : QAbstractItemModel{parent}
    {
        model_conf_.type_query = TYPE_REQUEST_TABLE;
        model_conf_.id_column = "ref";
        model_conf_.parent_id_column = "parent";

        set_limit_sql_rows(0);

        conf_ = arcirk::client::client_conf();
        item_icons.insert(item_icons_enum::ItemGroup, QIcon(":/img/group.png"));
        item_icons.insert(item_icons_enum::Item, QIcon(":/img/item.png"));
        item_icons.insert(item_icons_enum::DeletedItemGroup, QIcon(":/img/groupDeleted.png"));
        item_icons.insert(item_icons_enum::DeletedItem, QIcon(":/img/deletionMarkItem.png"));

        is_loaded_ = false;
        group_only_ = false;

        column_aliases = {};
        server_object_ = arcirk::server::server_objects::OBJ_INVALID;
        _nodes = {};
        auto ba = arcirk::string_to_byte_array(nlohmann::json(NIL_STRING_UUID).dump());
        model_conf_.default_parent = ByteArray(ba.size());
        std::copy(ba.begin(), ba.end(), model_conf_.default_parent.begin());
        model_conf_.hierarchical_list = true;

    }

    struct TreeViewModel::NodeInfo
    {
        NodeInfo():
            parent(0),
            mapped(false)
        {}

        NodeInfo(const nlohmann::json& rowData, NodeInfo* parent = 0):
            rowData(rowData),
            parent(parent),
            mapped(false)
        {this->children = {};}

        bool operator ==(const NodeInfo& another) const
        {
            bool r = this->rowData == another.rowData;
            Q_ASSERT(!r || this->parent == another.parent);
            Q_ASSERT(!r || this->mapped == another.mapped);
            Q_ASSERT(!r || this->children == another.children);
            return r;
        }

        nlohmann::json rowData;
        QVector<NodeInfo> children;
        NodeInfo* parent;
        QColor textColor;
        QIcon icon;
        QUuid uuid;
        bool mapped;
        std::string ref;
    };

    TreeViewModel::~TreeViewModel()
    {}

    QModelIndex TreeViewModel::index(int row, int column, const QModelIndex &parent) const
    {
        if (!hasIndex(row, column, parent)) {
            return QModelIndex();
        }

        if (!parent.isValid()) {
            Q_ASSERT(_nodes.size() > row);
            return createIndex(row, column, const_cast<NodeInfo*>(&_nodes[row]));
        }

        NodeInfo* parentInfo = static_cast<NodeInfo*>(parent.internalPointer());
        Q_ASSERT(parentInfo != 0);
        Q_ASSERT(parentInfo->mapped);
        Q_ASSERT(parentInfo->children.size() > row);
        return createIndex(row, column, &parentInfo->children[row]);
    }

    QModelIndex TreeViewModel::parent(const QModelIndex &child) const
    {
        if (!child.isValid()) {
            return QModelIndex();
        }

        NodeInfo* childInfo = static_cast<NodeInfo*>(child.internalPointer());
        Q_ASSERT(childInfo != 0);
        NodeInfo* parentInfo = childInfo->parent;        
        if (parentInfo != 0) {
            //Q_ASSERT(!parentInfo->rowData.is_null());
            return createIndex(findRow(parentInfo), 0, parentInfo);
        }
        else {
            return QModelIndex();
        }

        return QModelIndex();
    }

    QHash<int, QByteArray> TreeViewModel::roleNames() const
    {
        QHash<int, QByteArray> names;

        if (columns_.isEmpty())
            return names;

        for (int i = 0; i < columns_.size(); ++i) {
            const QString& key = columns_[i];
            int index = Qt::UserRole + i;
            names[index] = key.toUtf8();
        }

        return names;
    }

    QVector<QString> TreeViewModel::columnNames()
    {
        QVector<QString> result;
        result.resize(columns_.size());
        std::copy(columns_.begin(), columns_.end(), result.begin());
        return result;
    }

    int TreeViewModel::findRow(const NodeInfo *nodeInfo) const
    {
        Q_ASSERT(nodeInfo != 0);
        const NodeInfoList& parentInfoChildren = nodeInfo->parent != 0 ? nodeInfo->parent->children: _nodes;
        return parentInfoChildren.indexOf(*nodeInfo);
    //        NodeInfoList::const_iterator position = std::find(parentInfoChildren.constBegin(), parentInfoChildren.end(),  *nodeInfo);// qFind(parentInfoChildren, *nodeInfo);
    //        Q_ASSERT(position != parentInfoChildren.end());
    //        return std::distance(parentInfoChildren.begin(), position);


    }

    int TreeViewModel::rowCount(const QModelIndex &parent) const
    {
        if (!parent.isValid()) {
            return _nodes.size();
        }
        const NodeInfo* parentInfo = static_cast<const NodeInfo*>(parent.internalPointer());
        Q_ASSERT(parentInfo != 0);

        return parentInfo->children.size();
    }

    bool TreeViewModel::hasChildren(const QModelIndex &parent) const
    {
        if (parent.isValid()) {
            const NodeInfo* parentInfo = static_cast<const NodeInfo*>(parent.internalPointer());
            Q_ASSERT(parentInfo != 0);
            if (!parentInfo->mapped) {
                return true;//QDir(parentInfo->fileInfo.absoluteFilePath()).count() > 0;
            }
        }
        return QAbstractItemModel::hasChildren(parent);
    }

    int TreeViewModel::columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent)
        return columns_.size();
    }

    bool TreeViewModel::field_is_exists(const nlohmann::json &object, const std::string &name) const {
        auto itr = object.find(name);
        return itr != object.end();
    }

    QVariant TreeViewModel::data(const QModelIndex &index, int role) const
    {
        if (!index.isValid()) {
            return QVariant();
        }

        const NodeInfo* nodeInfo = static_cast<NodeInfo*>(index.internalPointer());
        const nlohmann::json& rowData = nodeInfo->rowData;
        Q_ASSERT(nodeInfo != 0);

        if (role != Qt::ForegroundRole) {
            if(index.column() < columns_.size()){
                if(index.column() == 0)
                    return firstData(nodeInfo, role, index);
                else{
                    if (role == Qt::DisplayRole) {
                        if(!field_is_exists(rowData, columns_[index.column()].toStdString()))
                            return QVariant();
                        auto val = rowData[columns_[index.column()].toStdString()];
                        if(val.is_string()){
                            if(sql_columns.size() == 0)
                                return QString::fromStdString(val.get<std::string>());
                            else{
                                std::string col = columns_[index.column()].toStdString();
                                foreach (auto itr, sql_columns) {
                                    if(itr.field_name == "CAST(0 AS INTEGER)" && itr.alias == col){
                                        return get_number(val);
                                    }
                                }
                            }
                        }else if(val.is_number_integer())
                            if(columns_[index.column()] != "size")
                                return val.get<int>();
                            else{
                                if(model_conf_.table_name != "ProfileDirectory")
                                    return val.get<int>();
                                else{
                                    auto val_i = val.get<int>();
                                    if(val_i > 1000)
                                        return val.get<int>() / 1000;
                                    else{
                                        if(val.get<int>() > 0){
                                            double value = (double)val.get<int>() / (double)1000;
                                            double exp = 100.0;
                                            double result = std::trunc(value * exp) / exp;
                                            return result;
                                        }else
                                            return QVariant();
                                    }
                                }
                            }
                        else if(val.is_number_float())
                            return val.get<double>();
                        else if(val.is_boolean()){
                            return  val.get<bool>();// ? "🗹" : "☐";
                        }
                    }else if (role == Qt::DecorationRole) {
                        auto val = rowData[columns_[index.column()].toStdString()];
    //                    auto itr = m_logical_icon.constFind(index.column());

    //                    if(itr != m_logical_icon.end() && val.is_boolean()){
    //                        return val.get<bool>() ? itr.value().first : itr.value().second;
    //                    }else{
    //                        return  val.get<bool>() ? QIcon(":/img/checkbox-checked.png") : QIcon(":/img/checkbox-unchecked.png") ;
    ////                        auto pair = qMakePair(index.row(), index.column());
    ////                        auto iter = m_rowIcon.constFind(pair);
    ////                        if(iter !=  m_rowIcon.end()){
    ////                            return iter.value();////m_rowIcon[pair];
    ////                        }
    //                    }

                        if(val.is_boolean()){
                            return  val.get<bool>() ? QIcon(":/img/checkbox-checked.png") : QIcon(":/img/checkbox-unchecked.png") ;
                        }
                    }
                }

            }
        }else{
            return nodeInfo->textColor;
        }

        return QVariant();
    }

    bool TreeViewModel::setData(const QModelIndex &index, const QVariant &value, int role)
    {
        if (!index.isValid()) {
            return false;
        }
    //    if (role != Qt::EditRole) {
    //        return false;
    //    }
    //	if (index.column() != NameColumn) {
    //		return false;
    //	}
        if(role != Qt::ForegroundRole){
            auto column_name = get_column_name(role);
            nlohmann::json val;
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
            }else
                val = "";
            NodeInfo* nodeInfo = static_cast<NodeInfo*>(index.internalPointer());
            Q_ASSERT(nodeInfo != 0);
        //    auto object = nodeInfo->rowData;
        //    nodeInfo->rowData = object;
            emit dataChanged(index, index.sibling(index.row(), columns_.size()));
            try {
                nodeInfo->rowData[column_name.toStdString()] = val;
            } catch (...) {
                return false;
            }
        }else{
            if(index.column() == 0){
                qDebug() << index.column();
            }
            NodeInfo* nodeInfo = static_cast<NodeInfo*>(index.internalPointer());
            Q_ASSERT(nodeInfo != 0);
            nodeInfo->textColor = value.value<QColor>();
        }

    //	QString newName = value.toString();
    //	if (newName.contains('/') || newName.contains(QDir::separator())) {
    //		return false;
    //	}
    //	NodeInfo* nodeInfo = static_cast<NodeInfo*>(index.internalPointer());
    //	QString fullNewName = nodeInfo->fileInfo.absoluteDir().path() +"/" + newName;
    //	QString fullOldName = nodeInfo->fileInfo.absoluteFilePath();
    //	qDebug() << fullOldName << fullNewName;
    //	bool renamed = QFile::rename(fullOldName, fullNewName);
    //	qDebug() << renamed;
    //	if (renamed) {
    //        nodeInfo->rowData. = QFileInfo(fullNewName);
    //		emit dataChanged(index, index.sibling(index.row(), ColumnCount));
    //	}
    //	return renamed;

        return true;
    }

    QVariant TreeViewModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section < columns_.size()) {
            auto name = QString::fromStdString(columns_[section].toStdString());
            auto itr = column_aliases.find(name);
            if(itr != column_aliases.end())
                return itr.value();
            else
                return name;
        }
        return QVariant();
    }

    bool TreeViewModel::canFetchMore(const QModelIndex &parent) const
    {
        //qDebug() << __FUNCTION__;

        if (!parent.isValid()) {
            return false;
        }

        const NodeInfo* parentInfo = static_cast<const NodeInfo*>(parent.internalPointer());
        Q_ASSERT(parentInfo != 0);
        if(parentInfo->mapped)
            emit canFetch(parent);
        return !parentInfo->mapped;
    }

    void TreeViewModel::fetchMore(const QModelIndex &parent)
    {

        //qDebug() << __FUNCTION__;

        Q_ASSERT(parent.isValid());
        NodeInfo* parentInfo = static_cast<NodeInfo*>(parent.internalPointer());
        Q_ASSERT(parentInfo != 0);
        Q_ASSERT(!parentInfo->mapped);

        const nlohmann::json& row = parentInfo->rowData;
        int is_group = row.value("is_group", 0);

        Q_ASSERT(is_group == 1);

        if(model_conf_.type_query == TYPE_REQUEST_ARBITRARY){
            //qDebug() << parentInfo->children.size();
            emit fetch(parent);
            return;
        }

        if(model_conf_.table_name.empty())
            return;

        nlohmann::json http_result{};
        if(model_conf_.table_name != "ProfileDirectory"){
            http_result = read_node_data(row[model_conf_.id_column].get<std::string>().c_str());
            set_current_parent_path(row[model_conf_.id_column].get<std::string>().c_str());
        }else{
            http_result = http_data(row["path"].get<std::string>().c_str());
            set_current_parent_path(row["path"].get<std::string>().c_str());
        }

        if(http_result.is_object()){

            auto children = http_result["rows"];

            if(children.is_array()){
                int insrtCnt = (int)children.size() - 1;
                if (insrtCnt < 0) {
                    insrtCnt = 0;
                }
                beginInsertRows(parent, 0, insrtCnt);
                parentInfo->children.reserve(children.size());
                for(auto entry = children.begin(); entry != children.end(); ++entry) {
                    nlohmann::json e = *entry;
                    NodeInfo nodeInfo(e, parentInfo);
                    nodeInfo.mapped = e["is_group"] != 1;
                    parentInfo->children.push_back(std::move(nodeInfo));
                }
                parentInfo->mapped = true;
                endInsertRows();
            }
        }

        emit fetch(parent);
    }

    void TreeViewModel::set_table(const nlohmann::json& tableModel){

        beginResetModel();

        model_conf_.type_query = TYPE_REQUEST_ARBITRARY;

        clear();

        if(!tableModel.is_object()){
            endResetModel();
            return;            
        }

        model_conf_.custom_cols = columns_init();
        if(!model_conf_.custom_cols){
            set_columns_from_array(tableModel.value("columns", nlohmann::json::array()));
        }
        auto rows = tableModel.value("rows", nlohmann::json::array());
        set_rows_from_array(model_conf_.custom_cols, rows);
        endResetModel();
        is_loaded_ = true;
    }

    void TreeViewModel::add_table(const nlohmann::json &tableModel)
    {
        if(_nodes.size() == 0){
            set_table(tableModel);
            return;
        }

        //beginResetModel();
        if(!tableModel.is_object()){
            //endResetModel();
            return;
        }

        auto rows = tableModel.value("rows", nlohmann::json::array());
        set_rows_from_array(model_conf_.custom_cols, rows, true);
        //endResetModel();
        is_loaded_ = true;
    }

    QString TreeViewModel::current_parent_path() const
    {
        return current_parent_path_;
    }

    void TreeViewModel::refresh(const QModelIndex& parent)
    {
        Q_UNUSED(parent);
    //    NodeInfo* parentInfo = static_cast<NodeInfo*>(parent.internalPointer());
    //    parentInfo->children.erase(parentInfo->children.cbegin(), parentInfo->children.cend());
    //    reset();
    ////    if (parent.model()->hasChildren())
    ////        parent.model()->removeRows(0,parent.model()->rowCount());
    //    //    fetchMore(parent);
    }

    void TreeViewModel::remove(const QModelIndex &index)
    {
        if(!index.isValid())
            return;

        NodeInfo* itemInfo = static_cast<NodeInfo*>(index.internalPointer());
        NodeInfo* parentInfo = itemInfo->parent;
        auto pos = findRow(itemInfo);

        if (parentInfo != 0) {
            beginRemoveRows(index.parent(), pos, pos);
            parentInfo->children.remove(pos);
            endRemoveRows();
        }else{
            beginRemoveRows(index.parent(), pos, pos);
            _nodes.remove(pos);
            endRemoveRows();
        }

    }

    void TreeViewModel::add(const nlohmann::json object, const QModelIndex &parent)
    {
        //auto parent_ = index(parent.row(), 0, parent.parent());
        NodeInfo* parentInfo = static_cast<NodeInfo*>(parent.internalPointer());
        if(parentInfo == 0){
            if(columns_.size() == 0){
                bool is_valid_hierarchy = false;
                for (auto itr = object.items().begin(); itr != object.items().end(); ++itr) {
                    std::string name = itr.key();
                    if(!use_hierarchy_.empty() && use_hierarchy_ == name){
                        is_valid_hierarchy = true;
                        continue;
                    }
                    columns_.push_back(QString::fromStdString(name));
                }
                if(is_valid_hierarchy){
                    columns_.insert(0, QString::fromStdString(use_hierarchy_));
                }
            }

            NodeInfo nodeInfo(object);
            int is_group = object.value("is_group", 0);
            nodeInfo.mapped = is_group != 1;// e["is_group"] != 1;
            _nodes.push_back(std::move(nodeInfo));
            reset();
        }else{
            int insrtCnt = parentInfo->children.size() - 1;
            if (insrtCnt < 0) {
                insrtCnt = 0;
            }

            beginInsertRows(parent, 0, insrtCnt);
            NodeInfo nodeInfo(nlohmann::json{}, parentInfo);
            int is_group = object.value("is_group", 0);
            nodeInfo.mapped = is_group != 1;
            parentInfo->mapped = true;
            parentInfo->children.push_back(std::move(nodeInfo));
            endInsertRows();
        }

    }

    void TreeViewModel::set_object(const QModelIndex &index, const nlohmann::json &object)
    {

        if(!index.isValid())
            return;
        NodeInfo* nodeInfo = static_cast<NodeInfo*>(index.internalPointer());
        Q_ASSERT(nodeInfo != 0);
        nodeInfo->rowData = object;
        emit dataChanged(index, index.sibling(index.row(), columns_.size()));
    }

    void TreeViewModel::use_hierarchy(const std::string &column)
    {
        use_hierarchy_ = column;
    }

    QModelIndex TreeViewModel::find_in_table(QAbstractItemModel *model, const QString &value, int column, int role, bool findData)
    {
        int rows =  model->rowCount();
        for (int i = 0; i < rows; ++i) {
            auto index = model->index(i, column);
            if(findData){
                if(value == index.data(role).toString())
                    return index;
            }else{
                QString data = index.data().toString();
                if(value == data)
                    return index;
            }
        }

        return QModelIndex();
    }

    QModelIndex TreeViewModel::find(const QString& find_value, int col, const QModelIndex& parent) const{
        QModelIndexList matches = match(
                    index(0,col, parent),
                    Qt::DisplayRole,
                    QVariant(find_value),
                    -1,
                    Qt::MatchRecursive);
        foreach (const QModelIndex &match, matches){
            return match;
        }
        return QModelIndex();
    }

    void TreeViewModel::move_up(const QModelIndex &index)
    {
        Q_ASSERT(index.isValid());
        if(index.row() < 1)
            return;

        beginMoveRows(index.parent(), index.row(), index.row(), index.parent(), index.row()-1);
        NodeInfo* itemInfo = static_cast<NodeInfo*>(index.internalPointer());
        Q_ASSERT(itemInfo != 0);
        NodeInfo* parentInfo = itemInfo->parent;
        auto pos = findRow(itemInfo);

        if(parentInfo != 0)
            parentInfo->children.move(pos, pos - 1);
        else
            _nodes.move(pos, pos - 1);

        endMoveRows();
    }

    void TreeViewModel::move_down(const QModelIndex &index)
    {
        Q_ASSERT(index.isValid());
        if(index.row() > rowCount(index.parent()) - 2)
            return;


        NodeInfo* itemInfo = static_cast<NodeInfo*>(index.internalPointer());
        Q_ASSERT(itemInfo != 0);
        NodeInfo* parentInfo = itemInfo->parent;
        auto pos = findRow(itemInfo);

        if(parentInfo != 0){
            beginMoveRows(index.parent(), pos, pos, index.parent(), (pos >= parentInfo->children.size()) ? parentInfo->children.size() : (pos + 2));
            parentInfo->children.move(pos, pos + 1);
        }else{
            beginMoveRows(index.parent(), pos, pos, index.parent(), (pos >= _nodes.size()) ? _nodes.size() : (pos + 2));
            _nodes.move(pos, pos + 1);
        }

        endMoveRows();
    }

    void TreeViewModel::move_to(const QModelIndex &index, const QModelIndex &new_parent)
    {
        Q_ASSERT(index.isValid());
        Q_ASSERT(new_parent.isValid());

        NodeInfo* itemInfo = static_cast<NodeInfo*>(index.internalPointer());
        Q_ASSERT(itemInfo != 0);
        NodeInfo* parentInfo = static_cast<NodeInfo*>(new_parent.internalPointer());
        Q_ASSERT(parentInfo != 0);
        NodeInfo* sourceParentInfo = static_cast<NodeInfo*>(index.parent().internalPointer());
        Q_ASSERT(sourceParentInfo != 0);

        if(parentInfo->mapped){
            beginMoveRows(index.parent(), index.row(), index.row(), new_parent, rowCount(new_parent));
            auto rowData = nlohmann::json(itemInfo->rowData);
            rowData["parent"] = parentInfo->rowData["ref"];
            NodeInfo nodeInfo(rowData, parentInfo);
            int is_group = rowData.value("is_group", 0);
            nodeInfo.mapped = is_group != 1;
            parentInfo->children.push_back(std::move(nodeInfo));
            endMoveRows();
        }
        remove(index);
    }

    void TreeViewModel::set_group_only(bool value)
    {
        group_only_ = value;
    }

    bool TreeViewModel::group_only() const
    {
        return group_only_;
    }

    void TreeViewModel::set_logical_icon(int column, QPair<QIcon, QIcon> icons)
    {
        m_logical_icon.insert(column, icons);
    }

    void TreeViewModel::set_current_parent_path(const QString &value)
    {
        current_parent_path_ = value;
    }

    void TreeViewModel::fetchRoot(const QString& table_name, const QString& root_dir, const nlohmann::json& where)
    {
        qDebug() << __FUNCTION__;

        model_conf_.type_query = TYPE_REQUEST_TABLE;

        clear();

        if(table_name.toStdString() == model_conf_.table_name && is_loaded_)
            return;

        bool custom_cols = columns_init();

        model_conf_.table_name = table_name.toStdString();
        current_parent_path_ = root_dir;

        nlohmann::json http_result{};
        if(model_conf_.table_name != "ProfileDirectory"){
            auto str = arcirk::byte_array_to_string(model_conf_.default_parent);
            nlohmann::json def{};
            if(nlohmann::json::accept(str))
                def = nlohmann::json::parse(str);
            http_result = read_node_data(def);
        }else{
            http_result = http_data(root_dir);
        }

        if(http_result == WS_RESULT_ERROR)
            return;

        if(http_result.is_object()){
            if(!custom_cols){
                set_columns_from_array(http_result.value("columns", nlohmann::json::array()));
            }else{
                if(!use_hierarchy_.empty() && columns_.indexOf(use_hierarchy_.c_str()) != -1){
                    auto i = columns_.indexOf(use_hierarchy_.c_str());
                    if(i != 0){
                        columns_.move(i, 0);
                    }
                }
            }
            beginResetModel();
            auto rows = http_result["rows"];
            if(rows.is_array()){
                set_rows_from_array(custom_cols, rows);
            }
            endResetModel();
            is_loaded_ = true;
        }

        emit fetch(QModelIndex());
    }

    void TreeViewModel::fetch_recursive(const std::string &table_name, const std::vector<std::string> &columns, const std::string &hierarchical_column, const nlohmann::json& default_parent, const nlohmann::json &where_detailed)
    {
        using json = nlohmann::json;

        if(table_name == model_conf_.table_name && is_loaded_)
            return;

        if(columns.empty())
            return;

        model_conf_.with_recursive = true;
        model_conf_.table_name = table_name;
        model_conf_.parent_id_column = "parent";
        model_conf_.id_column = "ref";

        std::vector<std::string> cols(columns.size());
        std::copy(columns.begin(), columns.end(), cols.begin());

        if(std::find(cols.begin(), cols.end(), "ref") == cols.end()){
            cols.emplace_back("ref");
        }
        if(std::find(cols.begin(), cols.end(), "parent") == cols.end()){
            cols.emplace_back("parent");
        }

        columns_.clear();
        foreach (auto itr, cols) {
            columns_.push_back(itr.c_str());
        }

        if(!where_detailed.empty())
            model_conf_.where_detailed = arcirk::string_to_byte_array(where_detailed.dump());
        auto str = arcirk::byte_array_to_string(model_conf_.default_parent);
        json def{};
        if(json::accept(str))
            def = json::parse(str);

        auto http_result = read_node_data(def);

    }

    QVariant TreeViewModel::firstData(const NodeInfo* nodeInfo, int role, const QModelIndex& index) const
    {
        auto rowData = nodeInfo->rowData;

        switch (role) {
        case Qt::EditRole:{
            return object_qt_variant(rowData);
        }case Qt::DisplayRole:{
            return object_qt_variant(rowData);
        }case Qt::DecorationRole:{
            auto is_group = rowData.value("is_group", 0);
            auto deletion_mark = rowData.value("deletion_mark", 0);
            auto p = qMakePair(index.row(), 0);
            auto itr = row_icons.find(p);
            auto itr_ico = row_ico.find(nodeInfo->uuid);

            if(itr_ico != row_ico.end())
                return itr_ico.value();
            else{

            if(itr != row_icons.end())
                return itr.value();
            else{
                    if(rows_icon.isNull()){
                        if(is_group == 0){
                            if(deletion_mark == 0)
                                return item_icons[item_icons_enum::Item];
                            else{
                                return item_icons[item_icons_enum::DeletedItem];
                            }
                        }else{
                            if(deletion_mark == 0)
                                return item_icons[item_icons_enum::ItemGroup];
                            else
                                return item_icons[item_icons_enum::DeletedItemGroup];
                        }
                    }else
                        return rows_icon;
                }
            }
        }default:
            return QVariant();
        }
        Q_UNREACHABLE();
    }

    TreeViewModel::NodeInfo *TreeViewModel::find_recursive(const std::string &ref, NodeInfoList &lst)
    {

        foreach (auto &itr, lst) {
            if(itr.ref == ref){
                return const_cast<NodeInfo*>(&itr);
            }
            if(itr.children.size() > 0){
                auto obj = find_recursive(ref, const_cast<NodeInfo*>(&itr)->children);
                if(obj)
                    return obj;

            }
        }

        return nullptr;
    }

    bool TreeViewModel::is_loaded()
    {
        return is_loaded_;
    }

    void TreeViewModel::set_column_aliases(const QMap<QString, QString> values)
    {
        column_aliases = values;
    }

    int TreeViewModel::get_column_index(const QString &name) const
    {
        auto names = roleNames();
        if (names.size() == 0)
            return -1;

        foreach (const auto& key , names.keys()) {

            if (names[key] == name.toUtf8()){
                return key - Qt::UserRole;
            }
        }

        return -1;
    }

    QString TreeViewModel::get_column_name(int column) const
    {

        if(column < columns_.size())
            return columns_[column];

        return {};
    }

    void TreeViewModel::set_establish_order(const QVector<QString> &names)
    {
        QByteArray bytes;
        QDataStream stream(&bytes, QIODevice::WriteOnly);
        stream << names;
        model_conf_.columns_order = ByteArray(bytes.begin(), bytes.end());
    }

    void TreeViewModel::set_columns(const QVector<QString> cols)
    {
        using namespace arcirk::database::builder;

        QVector<QString> cols_;
        foreach (auto itr, cols) {
            sql_value item(itr.toStdString(), "", json{});
            auto object = sql_value_to_json(item);
            cols_.push_back(object.dump().c_str());
        }

        QByteArray bytes;
        QDataStream stream(&bytes, QIODevice::WriteOnly);
        stream << cols_;
        model_conf_.columns = ByteArray(bytes.begin(), bytes.end());

//        //columns_ = cols;
//        sql_columns.clear();
//        foreach (auto &itr, cols) {
//            sql_columns.emplace_back(arcirk::database::builder::sql_value(itr.toStdString(), "", {}));
//        }


    }

    void TreeViewModel::set_server_object(arcirk::server::server_objects obj)
    {
        server_object_ = obj;
    }

    arcirk::server::server_objects TreeViewModel::server_object() const
    {
        return server_object_;
    }

    nlohmann::json TreeViewModel::to_object(const QModelIndex &index) const
    {
        const NodeInfo* nodeInfo = static_cast<NodeInfo*>(index.internalPointer());
        const nlohmann::json& rowData = nodeInfo->rowData;
        Q_ASSERT(nodeInfo != 0);
        //костыль, soci возвращает стринг
        std::vector<std::string> cols;
        foreach (auto itr, sql_columns) {
            if(itr.field_name == "CAST(0 AS INTEGER)"){
                cols.push_back(itr.alias);
            }
        }
        if(!cols.empty()){
            auto r = rowData;
            foreach (auto itr, cols) {
                auto v = rowData[itr];
                r[itr] = get_number(v);
                if(r[itr].is_null())
                    r[itr] = "";
            }
            return r;
        }

        return rowData;

    }

    nlohmann::json TreeViewModel::to_array(const QModelIndex &parent) const
    {
        NodeInfo* parentInfo = static_cast<NodeInfo*>(parent.internalPointer());

        using json = nlohmann::json;

        json result = json::array();

        if(parentInfo == 0){
            for (auto itr = _nodes.begin(); itr != _nodes.end(); ++itr) {
                result += itr->rowData;
            }
        }else{
            for (auto itr = parentInfo->children.begin(); itr != parentInfo->children.end(); ++itr) {
                result += itr->rowData;
            }
        }

        return result;

    }

    nlohmann::json TreeViewModel::to_table_model(const QModelIndex &parent) const
    {
        using json = nlohmann::json;
        auto roles = roleNames();
        auto columns_j = json::array();
        foreach (const auto& key , roles.keys()) {
            columns_j += roles[key].toStdString();
        }

        auto rows = to_array(parent);

        return json::object({
            {"columns" , columns_j},
            {"rows", rows}
        });
    }

    void TreeViewModel::clear()
    {
        beginResetModel();
        row_icons.clear();
        columns_.clear();
        _nodes.clear();
        is_loaded_ = false;
        model_conf_.table_name = "";
        endResetModel();
    }

    void TreeViewModel::reset()
    {
        //qDebug() << "start reset" << QTime::currentTime();
        beginResetModel();
        //qDebug() << "end reset" << QTime::currentTime();
        endResetModel();
    }

    Qt::ItemFlags TreeViewModel::flags(const QModelIndex &index) const
    {
        Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    //	if (index.isValid() && index.column() == NameColumn) {
    //		const NodeInfo* nodeInfo = static_cast<const NodeInfo*>(index.internalPointer());
    //		if (!nodeInfo->fileInfo.isRoot()) {
    //			flags |= Qt::ItemIsEditable;
    //		}
    //	}
        return flags;
    }

    void TreeViewModel::set_rows_icon(const QIcon &ico)
    {
        rows_icon = ico;
    }

    void TreeViewModel::set_icon(const QModelIndex &index, const QIcon &ico)
    {
        row_icons.insert(qMakePair(index.row(), index.column()), ico);
    }

    void TreeViewModel::set_icon(const QUuid& uuid, const QIcon &ico)
    {
        row_ico.insert(uuid, ico);
    }

    QModelIndex TreeViewModel::find(const nlohmann::json &find_value, const QModelIndex &parent) const
    {
        if(!find_value.is_object())
            return QModelIndex();

        for (auto itr = find_value.items().begin(); itr != find_value.items().end(); ++itr) {
            int col = get_column_index(itr.key().c_str());
            nlohmann::json val = itr.value();
            if(col!=-1){

                QModelIndexList matches = match(
                            index(0,col, parent),
                            Qt::DisplayRole,
                            to_variant(val),
                            -1,
                            Qt::MatchRecursive);
                foreach (const QModelIndex &match, matches){
                    auto object = to_object(match);
                    //bool is_commpare = false;
                    int count = find_value.size();
                    for (auto it = find_value.items().begin(); it != find_value.items().end(); ++it) {
                        if(object[it.key()] == it.value()){
                            count--;
                        }
                    }
                    if(count == 0)
                        return match;

                }
            }
        }

        return QModelIndex();

    }

    QVariant TreeViewModel::to_variant(const nlohmann::json &value) const
    {
        if(value.is_string())
            return QString::fromStdString(value.get<std::string>());
        else if(value.is_number_float())
            return value.get<double>();
        else if(value.is_number_integer())
            return value.get<int>();
        else if(value.is_boolean())
            return value.get<bool>();
        else
            return QVariant();
    }

    int TreeViewModel::table_row_count()
    {
        using builder = arcirk::database::builder::query_builder;

        if(model_conf_.table_name.empty())
            return 0;

        auto query = builder().row_count().from(model_conf_.table_name);

        auto where_root = nlohmann::json::object();

        auto str = arcirk::byte_array_to_string(model_conf_.default_parent);
        nlohmann::json parent{};
        if(nlohmann::json::accept(str))
            parent = nlohmann::json::parse(str);

        if(model_conf_.hierarchical_list){
            where_root[model_conf_.parent_id_column] = parent;
        }

        str = arcirk::byte_array_to_string(model_conf_.where_detailed);
        nlohmann::json where_{};
        if(nlohmann::json::accept(str))
            where_ = nlohmann::json::parse(str);


        if(model_conf_.where_detailed.size() > 0){

            for (auto itr = where_.items().begin(); itr != where_.items().end(); ++itr) {
                where_root[itr.key()] = itr.value();
            }
        }

        if(group_only_){
            where_root["is_group"] = 1;
        }

        std::string query_text;
        query_text = query.row_count().from(model_conf_.table_name).where(where_root, true).prepare();

        //qDebug() << qPrintable(query_text.c_str());

        auto param = nlohmann::json::object();
        param["query_text"] = query_text;

        auto http_param = arcirk::synchronize::http_param();
        http_param.command = arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery);
        http_param.param = QByteArray(param.dump().data()).toBase64().toStdString();

        QEventLoop loop;
        int httpStatus = 200;
        QByteArray httpData;
        QNetworkAccessManager httpService;

        auto finished = [&loop, &httpStatus, &httpData](QNetworkReply* reply) -> void
        {
            QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
            if(status_code.isValid()){
                httpStatus = status_code.toInt();
                if(httpStatus != 200){
                    qCritical() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__ << "Error: " << httpStatus << " " + reply->errorString() ;
                }else
                {
                    httpData = reply->readAll();
                }
            }
            loop.quit();

        };

        loop.connect(&httpService, &QNetworkAccessManager::finished, finished);

        QUrl ws(conf_.server_host.data());
        QString protocol = ws.scheme() == "wss" ? "https://" : "http://";
        QString http_query = "/api/info";
        QUrl url(protocol + ws.host() + ":" + QString::number(ws.port()) + http_query);
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QString headerData = "Token " + authString_;
        request.setRawHeader("Authorization", headerData.toLocal8Bit());

        httpService.post(request, QByteArray::fromStdString(pre::json::to_json(http_param).dump()));
        loop.exec();

        if(httpStatus != 200){
            return 0;
        }

        if(httpData.isEmpty())
            return 0;

        if(httpData == "error"){
            return 0;
        }

        auto msg = pre::json::from_json<arcirk::server::server_response>(httpData.toStdString());

        if(msg.result.empty())
            return 0;

        if(msg.result == WS_RESULT_ERROR)
            return 0;

        auto http_result = nlohmann::json::parse(QByteArray::fromBase64(msg.result.data()).toStdString());

        //qDebug() << qPrintable(http_result.dump(4).c_str());
        try {
            if(http_result.is_object()){
                auto rows =  http_result.value("rows", nlohmann::json::array());
                if(rows.size() > 0){
                    return std::stoi(rows[0]["count(*)"].get<std::string>());
                }
            }else
                return 0;
        } catch (...) {
            return 0;
        }

        return 0;
    }

    void TreeViewModel::set_hierarchy_list_icon(item_icons_enum state, const QIcon &icon)
    {
        item_icons[state] = icon;
    }

    const QUuid TreeViewModel::row_uuid(const QModelIndex &index)
    {
        if(!index.isValid())
            return {};

        NodeInfo* nodeInfo = static_cast<NodeInfo*>(index.internalPointer());

        if(nodeInfo)
            return nodeInfo->uuid;

        return {};

    }

    bool TreeViewModel::columns_init()
    {
        bool custom_cols = false;

        if(model_conf_.columns.size() > 0){
            using namespace arcirk::database::builder;
            sql_columns.clear();
            QVector<QString> vec;
            auto ba = QByteArray((char*)model_conf_.columns.data(), model_conf_.columns.size());
            QDataStream stream(ba);
            stream >> vec;
            foreach (QString itr, vec) {
                auto object = json::parse(itr.toStdString());
                sql_value item  = sql_value_from_json(object);
                columns_.push_back(item.alias.empty() ? item.field_name.c_str() : item.alias.c_str());
                sql_columns.push_back(std::move(item));

            }
            custom_cols = true;
        }
        if(model_conf_.columns_order.size() > 0){
            auto ba = QByteArray((char*)model_conf_.columns_order.data(), model_conf_.columns_order.size());
            QDataStream stream(ba);
            QVector<QString> cols(columns_);
            columns_.clear();
            stream >> columns_;
            foreach (auto itr, cols) {
                if(columns_.indexOf(itr) == -1){
                    columns_.push_back(itr);
                }
            }
            custom_cols = true;
        }



        return custom_cols;
    }

    void TreeViewModel::set_columns_from_array(json columns_array)
    {
        columns_.clear();
        if(columns_array.is_array()){
            bool is_valid_hierarchy = false;
            for (auto itr = columns_array.begin(); itr != columns_array.end(); ++itr) {
                std::string name = *itr;
                if(!use_hierarchy_.empty() && use_hierarchy_ == name){
                    is_valid_hierarchy = true;
                    continue;
                }
                columns_.push_back(QString::fromStdString(name));
            }
            if(is_valid_hierarchy){
                columns_.insert(0, QString::fromStdString(use_hierarchy_));
            }
        }
    }

    void TreeViewModel::set_rows_from_array(const bool &custom_cols, json rows_array, bool insert)
    {

        for(auto entry = rows_array.begin(); entry != rows_array.end(); ++entry) {
            nlohmann::json e = *entry;
            if(custom_cols){
                e = json::object();
                foreach (auto itr, columns_) {
                    e[itr.toStdString()] = (*entry).value(itr.toStdString(), json{});
                }
            }
            std::string parent_id = e.value(model_conf_.parent_id_column, NIL_STRING_UUID);
            std::string ref_id = e.value(model_conf_.id_column, NIL_STRING_UUID);

            int is_group = e.value("is_group", 0);

            if(parent_id == NIL_STRING_UUID || !model_conf_.hierarchical_list){
                int insrtCnt = (int)_nodes.size() - 1;
                if (insrtCnt < 0) {
                    insrtCnt = 0;
                }
                if(insert){
                    int insrtCnt = (int)_nodes.size() - 1;
                    if (insrtCnt < 0) {
                        insrtCnt = 0;
                    }
                    beginInsertRows(QModelIndex(), insrtCnt, _nodes.size() );
                }
                NodeInfo nodeInfo(e);
                nodeInfo.ref = ref_id;
                if(model_conf_.type_query == TYPE_REQUEST_TABLE)
                    nodeInfo.mapped = is_group != 1;
                else
                    nodeInfo.mapped = true;
                if(!QUuid::fromString(ref_id.c_str()).isNull())
                    nodeInfo.uuid = QUuid::fromString(ref_id.c_str());
                else
                    nodeInfo.uuid = QUuid::createUuid();

                _nodes.push_back(std::move(nodeInfo));

                if(insert){
                    endInsertRows();
                }

            }else{

                NodeInfo* parentInfo = find_recursive(parent_id, _nodes);

                if(parentInfo != 0){
                    if(insert){
                        int insrtCnt = (int)parentInfo->children.size() - 1;
                        if (insrtCnt < 0) {
                            insrtCnt = 0;
                        }
                        auto i = findRow(parentInfo);
                        auto ind = index(i, 0, QModelIndex());// createIndex(i, 0, parentInfo); //find(json{{"ref", parentInfo->ref}}, QModelIndex());
                        Q_ASSERT(ind.isValid());
                        beginInsertRows(ind, 0, insrtCnt);
                    }

                    NodeInfo nodeInfo(e, parentInfo);
                    if(model_conf_.type_query == TYPE_REQUEST_TABLE)
                        nodeInfo.mapped = is_group != 1;
                    else
                        nodeInfo.mapped = true;
                    nodeInfo.ref = ref_id;
                    if(!QUuid::fromString(ref_id.c_str()).isNull())
                        nodeInfo.uuid = QUuid::fromString(ref_id.c_str());
                    else
                        nodeInfo.uuid = QUuid::createUuid();
                    parentInfo->children.push_back(std::move(nodeInfo));

                    if(insert){
                        endInsertRows();
                    }
                }else{
                    if(insert){
                        int insrtCnt = (int)_nodes.size() - 1;
                        if (insrtCnt < 0) {
                            insrtCnt = 0;
                        }
                        beginInsertRows(QModelIndex(), 0, insrtCnt);
                    }
                    NodeInfo nodeInfo(e);
                    if(model_conf_.type_query == TYPE_REQUEST_TABLE)
                        nodeInfo.mapped = is_group != 1;
                    else
                        nodeInfo.mapped = true;
                    nodeInfo.ref = ref_id;
                    if(!QUuid::fromString(ref_id.c_str()).isNull())
                        nodeInfo.uuid = QUuid::fromString(ref_id.c_str());
                    else
                        nodeInfo.uuid = QUuid::createUuid();
                    _nodes.push_back(std::move(nodeInfo));
                    if(insert){
                        endInsertRows();
                    }
                }

            }

        }
    }
}
