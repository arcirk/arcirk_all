#include "treeitem.h"


#include "query_builder.hpp"
#include <QSqlQuery>
#include <QSqlError>

using namespace arcirk::tree_model;

TreeItem::TreeItem(const json &data, TreeItem *parent)
{
    m_parentItem = parent;
    if(data.size() > 0)
        m_item_data = data;
    else{
        auto tmp = parent->to_object();
        m_item_data = json::object();
        for (auto itr = tmp.items().begin(); itr != tmp.items().end(); ++itr) {
            m_item_data[itr.key()] = json();
        }
    }
    is_root_item = false;
    m_mapped = false;
    m_object_name = "TreeItemModel";
    m_childItems = {};

    if(m_item_data.empty() || !m_item_data.is_object())
        m_item_data = json::object();

    if(m_item_data.find("ref") == m_item_data.end())
        m_item_data["ref"] = tree::generate_uuid().toStdString();
    else{
        if(m_item_data["ref"].is_null())
          m_item_data["ref"] = tree::generate_uuid().toStdString();
    }
    if(m_item_data.find("is_group") == m_item_data.end())
        m_item_data["is_group"] = 0;
    else{
        if(m_item_data["is_group"].is_null())
          m_item_data["is_group"] = 0;
    }

    m_conf = parent->conf();
    if(m_conf->hierarchical_list())
        m_item_data["parent"] = tree::quuid_to_string(parent->ref()).toStdString();

    m_is_group = m_item_data.value("is_group", 0) != 0;

    init_ids();
}

TreeItem::TreeItem(const json &data, std::shared_ptr<root_tree_conf> conf)
{
    //root item
    m_parentItem = nullptr;
    m_item_data = data;
    m_ref = QUuid::fromString(NIL_STRING_UUID);
    m_conf = conf;
    is_root_item = true;
    m_mapped = true;
    m_is_group = true;
    m_object_name = "TreeItemModel";
    m_childItems = {};
}

TreeItem::~TreeItem()
{
    qDeleteAll(m_childItems);
}

void TreeItem::appendChild(TreeItem *child)
{
    m_childItems.append(child);
}

TreeItem *TreeItem::child(int row)
{
    return m_childItems.value(row);
}

int TreeItem::childCount() const
{
    return m_childItems.count();
}

QVariant TreeItem::data(int column, int role) const
{
    if(m_item_data.size() == 0)
        return QVariant();
    Q_ASSERT(m_item_data.size() > column);

    auto c_name = m_conf->column_name(column);

    if(role == Qt::DisplayRole){
        auto value = m_item_data.value(c_name.toStdString(), json{});
        if(value.is_null())
            return QVariant();
//        else{
//            auto fr = data(column, FormatTextRole);
//            if(fr.isValid()){
//                QString v;
//                if(value.is_number()){

//                }
//            }
//        }
        return to_variant(value);
    }else if(role >= Qt::UserRole && role <= Qt::UserRole + tree::user_roles_max()){
        auto itr =m_userData.find(role);
        if(itr != m_userData.end())
            if(role != tree::WidgetRole)
                return itr.value()[column];
            else{
                if(m_widget == tree::item_editor_widget_roles::widgetINVALID)
                    return itr.value()[column];
                else
                    return m_widget;
            }
        else
            //if(m_widget == tree::item_editor_widget_roles::widgetINVALID)
                return QVariant();
//            else
//                return m_widget;

    }else if(role == Qt::ForegroundRole)
        return m_text_color;

    return QVariant();
}

void TreeItem::set_widget(tree::item_editor_widget_roles role)
{
    m_widget = role;
}

arcirk::tree::item_editor_widget_roles TreeItem::widget() const
{
    return m_widget;
}

bool TreeItem::setData(int column, const QVariant &value, int role)
{
    if (column < 0 || column >= m_item_data.size()) return false;
    if(role == Qt::EditRole){
        auto c_name = m_conf->column_name(column);
        m_item_data[c_name.toStdString()] = to_json(value);
        //validate_text();
    }else if(role >= Qt::UserRole && role <= Qt::UserRole + tree::user_roles_max()){
        if(m_userData.find(role) == m_userData.end()){
                QList<QVariant> m_vec;
                m_vec.resize(m_conf->columns().size());
                m_userData.insert(role, m_vec);
                m_userData[role][column] = value;
                //return false;
        }
            m_userData[role][column] = value;
    }else if(role == Qt::ForegroundRole){
        m_text_color = value;
    }
    return true;
}

int TreeItem::childNumber() const
{
    if (m_parentItem) return m_parentItem->m_childItems.indexOf(const_cast<TreeItem*>(this));
    return 0;
}

TreeItem *TreeItem::parentItem()
{
    return m_parentItem;
}

bool TreeItem::insertChildren(int position, int count, int columns)
{
    if (position < 0 || position > m_childItems.size()) return false;
    for (int row = 0; row < count; ++row) {
        auto data = json::object();
        TreeItem *item = new TreeItem(data, this);
        m_childItems.insert(position, item);
    }
    return true;
}


bool TreeItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > m_childItems.size()) return false;
    for (int row = 0; row < count; ++row) m_childItems.removeAt(position);
    return true;
}

bool TreeItem::moveChildren(int source, int position)
{
    if(position < 0 || position >= m_childItems.size())
        return false;
    m_childItems.move(source, position);
    return true;
}


void TreeItem::clearChildren()
{
    m_childItems.clear();
}

void TreeItem::setParent(TreeItem *parent)
{
    m_parentItem = parent;
    m_conf = parent->conf();

}

void TreeItem::set_object(const json &object, bool upgrade_database){

    m_item_data = object;

    if(m_item_data.find("ref") == m_item_data.end())
        m_item_data["ref"] = tree::generate_uuid().toStdString();

    if(m_conf->hierarchical_list())
        m_item_data["parent"] = tree::quuid_to_string(m_parentItem->ref()).toStdString();

    if(m_item_data.find("is_group") == m_item_data.end())
        m_item_data["is_group"] = 0;

    m_is_group = m_item_data.value("is_group", 0) != 0;

    init_ids();

    if(upgrade_database && m_conf->is_database_changed()){
        if((m_conf->type_connection() == root_tree_conf::sqlIteMemoryConnection ||
             m_conf->type_connection() == root_tree_conf::sqlIteConnection) &&
            !m_conf->table_name().isEmpty() && m_conf->get_database()){
                using namespace arcirk::database;
                auto query = builder::query_builder();
                auto text = query.row_count().from(m_conf->table_name().toStdString()).where(
                    json{{"ref", quuid_to_string(ref()).toStdString()}}, true).prepare();
                auto rc = m_conf->get_database()->exec(text.c_str());
                int rc_count = 0;
                while (rc.next()){
                    rc_count = rc.value(0).toInt();
                }
                auto data = to_object();
                auto object_ = json::object();
                for (auto itr = data.items().begin(); itr != data.items().end(); ++itr) {
                    if(itr.value().is_boolean()){
                    object_[itr.key()] = itr.value().get<bool>() ? 1 : 0;
                    }else{
                    object_[itr.key()] = itr.value();
                    }
                }
                query.clear();
                query.use(object_);
                if(rc_count > 0){
                    text = query.update(m_conf->table_name().toStdString(), true).where(json{{"ref", quuid_to_string(ref()).toStdString()}}, true).prepare();
                }else{
                    text = query.insert(m_conf->table_name().toStdString(), true).prepare();
                }
                rc = m_conf->get_database()->exec(text.c_str());
                //qDebug() << qPrintable(text.c_str());
                if(rc.lastError().isValid()){
                    qCritical() << rc.lastError().text();
                }
        }
    }
}

void TreeItem::set_object_name(const QString &name)
{
    m_object_name = name;
}

void TreeItem::init_ids()
{
    m_ref = QUuid::fromString(m_item_data.value("ref", tree::generate_uuid().toStdString()).c_str());
}
