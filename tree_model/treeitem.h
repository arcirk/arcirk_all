#ifndef TREEITEM_H
#define TREEITEM_H

#include "global/arcirk_qt.hpp"
#include <QUuid>
#include <memory>

using namespace arcirk::tree;

namespace arcirk::tree_model {
    class TreeItem
    {
    public:
        explicit TreeItem(const json& data, TreeItem *parentItem = 0);
        explicit TreeItem(const json& data, std::shared_ptr<root_tree_conf> conf);
        ~TreeItem();

        void appendChild(TreeItem *child);
        TreeItem *child(int row);
        int childCount() const;
        QVariant data(int column, int role = Qt::DisplayRole) const;
        bool setData(int column, const QVariant &value, int role = Qt::EditRole);
        int childNumber() const;

        TreeItem *parentItem();

        bool insertChildren(int position, int count, int columns);
        bool removeChildren(int position, int count);
        bool moveChildren(int source, int position);
        void clearChildren();
        void setParent(TreeItem* parent);

        QUuid ref() const{return m_ref;};

        std::shared_ptr<root_tree_conf>conf(){
            if(!is_root_item)
                return m_parentItem->conf();
            else
                return m_conf;
        };
        QVariant text_color() const { return m_text_color;};
        bool is_group(){return m_is_group;};

        json to_object(){return m_item_data;};
        void set_object(const json& object, bool upgrade_database = false);
        bool mapped(){return m_mapped;};
        void set_mapped(bool value){m_mapped = value;};
        void set_object_name(const QString& name);
        QString object_name() const{
            return m_object_name;
        };


        void set_widget(tree::item_editor_widget_roles role);
        tree::item_editor_widget_roles widget() const;

        void set_icon(const QIcon& ico){m_icon = ico;};
        QIcon icon() const {return m_icon;};

    private:
        QUuid m_ref;
        bool m_is_group;
        TreeItem* m_parentItem;
        QList <TreeItem*> m_childItems;
        json m_item_data;
        bool is_root_item;
        std::shared_ptr<root_tree_conf> m_conf;
        QVariant m_text_color;
        bool m_mapped;
        QString m_object_name;
        QMap<int, QList<QVariant>> m_userData;
        tree::item_editor_widget_roles m_widget;
        QIcon m_icon;

        void init_ids();
    };
}

#endif // TREEITEM_H
