#ifndef TREEMODEL_HPP
#define TREEMODEL_HPP

#include "shared_struct.hpp"
#include <QList>
#include <QVector>
#include <QVariant>
#include <QAbstractItemModel>
#include <QModelIndex>

using json = nlohmann::json;

inline QVariant to_variant(const json& value){
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

inline json to_json(const QVariant& value){
    if(value.typeId() == QMetaType::QString){
        return value.toString().toStdString();
    }else if(value.typeId() == QMetaType::Int){
        return value.toInt();
    }else if(value.typeId() == QMetaType::Double){
        return value.toDouble();
    }else if(value.typeId() == QMetaType::LongLong){
        return value.toLongLong();
    }else if(value.typeId() == QMetaType::Bool){
        return value.toBool();
    }else
        return {};
}

class TreeItem
{
public:
    explicit TreeItem(const json &data,
                      TreeItem *parentItem = 0) ;
     ~TreeItem();
    void appendChild(TreeItem *child);
    TreeItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int childNumber() const;

    TreeItem *parentItem();
    bool insertChildren(int position, int count, int columns);
    bool insertColumns(int position, int columns);
    bool removeChildren(int position, int count);
    bool removeColumns(int position, int columns);
    bool setData(int column, const QVariant &value);

    QVector<std::string> columns() const;

private:
    QList <TreeItem*> m_childItems;
    json m_itemData;
    TreeItem *m_parentItem;
};

class TreeModel : public QAbstractItemModel {
    Q_OBJECT
 public:
    TreeModel(QObject *parent = 0);
    ~TreeModel();

    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
    int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
    const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value,
    int role = Qt::EditRole) override;
    bool insertColumns(int position, int columns,const QModelIndex &parent = QModelIndex()) override;
    bool removeColumns(int position, int columns, const QModelIndex &parent = QModelIndex()) override;
    bool insertRows(int position, int rows, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex()) override;

    void set_table(const nlohmann::json& table);

    void clear(){
        delete rootItem;
        rootItem = new TreeItem(json::object());
    }

private:
    void setupModelData(const QStringList &lines, TreeItem *parent);
    TreeItem *getItem(const QModelIndex &index) const;
    TreeItem *rootItem;
    QVector<QString> m_columns;
};

TreeItem::TreeItem (const json &data, TreeItem *parent) {
    m_parentItem = parent;
    m_itemData = data;
}

TreeItem::~TreeItem() { qDeleteAll(m_childItems); }

QVector<std::string> TreeItem::columns() const{
    QVector<std::string> vec;
    if(!m_itemData.is_object())
        return vec;
    for (auto itr = m_itemData.items().begin(); itr != m_itemData.items().end(); ++itr) {
        vec.push_back(itr.key());
    }
    return vec;
}

void TreeItem::appendChild(TreeItem *item) {
    m_childItems.append(item);
}

TreeItem *TreeItem::child (int row) {
    return m_childItems.value(row);
}

int TreeItem::childCount() const {
    return m_childItems.count();
}

int TreeItem::columnCount() const {
    return m_itemData.size();
}

QVariant TreeItem::data (int column) const {
    auto cols = columns();
    Q_ASSERT(cols.size() > column);
    return m_itemData.value(cols[column], 0);
}

TreeItem *TreeItem::parentItem() {
 return m_parentItem;
}

int TreeItem::childNumber() const {
    if (m_parentItem) return m_parentItem->m_childItems.indexOf(const_cast<TreeItem*>(this));
    return 0;
}

bool TreeItem::insertChildren(int position, int count, int columns) {
    if (position < 0 || position > m_childItems.size()) return false;
    for (int row = 0; row < count; ++row) {
        auto data = json::object();
        TreeItem *item = new TreeItem(data, this);
        m_childItems.insert(position, item);
    }
    return true;
}

bool TreeItem::insertColumns(int position, int columns) {
//    if (position < 0 || position > m_itemData.size()) return false;
//    for (int column = 0; column < columns; ++column) m_itemData.insert(position, QVariant());
//    foreach (TreeItem *child, m_childItems) child->insertColumns(position, columns);
    return true;
}

bool TreeItem::removeChildren(int position, int count) {
    if (position < 0 || position + count > m_childItems.size()) return false;
    for (int row = 0; row < count; ++row) delete m_childItems.takeAt(position);
    return true;
}

bool TreeItem::removeColumns(int position, int columns) {
//    if (position < 0 || position + columns > m_itemData.size()) return false;
//    for (int column = 0; column < columns; ++column) m_itemData.removeAt(position);
//    foreach (TreeItem *child, m_childItems) child->removeColumns(position, columns);
    return true;
}

bool TreeItem::setData(int column, const QVariant &value) {
    if (column < 0 || column >= m_itemData.size()) return false;
    m_itemData[columns()[column]] = to_json(value);
    return true;
}

TreeModel::TreeModel( QObject *parent)
    : QAbstractItemModel(parent) {
//    QVector <QVariant> rootData;
//    rootData << "Узел" << "Описание";
    rootItem = new TreeItem(json::object());
//    setupModelData(data.split(QString("\n")), rootItem);
}

TreeModel::~TreeModel() { delete rootItem; }

int TreeModel::columnCount(const QModelIndex &parent) const {
    return rootItem->columnCount();
}



void TreeModel::set_table(const nlohmann::json& table){

    clear();



}


QVariant TreeModel::data (const QModelIndex &index, int role) const {
    if (!index.isValid()) return QVariant();
    if (role != Qt::DisplayRole  && role != Qt::EditRole) return QVariant();
    TreeItem *item = getItem(index);
    return item->data(index.column());
}

TreeItem *TreeModel::getItem(const QModelIndex &index) const {
    if (index.isValid()) {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        if (item) return item;
    }
    return rootItem;
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const {
    if (!index.isValid()) return (Qt::ItemFlags)0;
    return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) return rootItem->data(section);
    return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const {
    if (parent.isValid() && parent.column() != 0) return QModelIndex();
    TreeItem *parentItem = getItem(parent);
    TreeItem *childItem = parentItem->child(row);
    if (childItem) return createIndex(row, column, childItem);
    else return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex &index) const {
    if (!index.isValid()) return QModelIndex();
    TreeItem *childItem = getItem(index);
    TreeItem *parentItem = childItem->parentItem();
    if (parentItem == rootItem) return QModelIndex();
    return createIndex(parentItem->childNumber(), 0, parentItem);
}

int TreeModel::rowCount(const QModelIndex &parent) const {
    TreeItem *parentItem = getItem(parent);
    return parentItem->childCount();
}

void TreeModel::setupModelData(const QStringList &lines, TreeItem *parent) {
//    QList<TreeItem*> parents;
//    QList<int> indentations;
//    parents << parent;
//    indentations << 0;
//    int number = 0;
//    //Ищем первый непробельный символ с номером position
//    while (number < lines.count()) {
//        int position = 0;
//        while (position < lines[number].length()) {
//            if (lines[number].at(position) != ' ') break;
//            position++;
//        }
//        //Отрезаем пробельное начало строки
//        QString lineData = lines[number].mid(position).trimmed();
//        if (!lineData.isEmpty()) {
//            //Читаем остальную часть строки, если она есть
//            QStringList columnStrings = lineData.split(":::", Qt::SkipEmptyParts);
//            //Учитываем разделитель столбцов
//            QVector <QVariant> columnData; //Список данных столбцов
//            for (int column = 0; column < columnStrings.count(); ++column)
//                columnData << columnStrings[column];
//            if (position > indentations.last()) {
//                //Последний потомок текущего родителя теперь будет новым родителем,
//                //пока у текущего родителя нет потомков
//                if (parents.last()->childCount() > 0) {
//                    parents << parents.last()->child(parents.last()->childCount()-1);
//                    indentations << position;
//                }
//            }
//            else {
//                while (position < indentations.last() && parents.count() > 0) {
//                    parents.pop_back();
//                    indentations.pop_back();
//                }
//            }
//            //Добавить новый узел в список потомков текущего родителя
//            parents.last()->appendChild(new TreeItem(columnData, parents.last()));
//        }
//        ++number;
//    }
}

bool TreeModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (role != Qt::EditRole) return false;
    TreeItem *item = getItem(index);
    bool result = item->setData(index.column(), value);
    if (result) {
        emit dataChanged(index, index);
    }
    return result;
}

bool TreeModel::setHeaderData(int section, Qt::Orientation orientation,
                              const QVariant &value, int role) {
    if (role != Qt::EditRole || orientation != Qt::Horizontal) return false;
        bool result = rootItem->setData(section, value);
    if (result) {
        emit headerDataChanged(orientation, section, section);
    }
    return result;
}

bool TreeModel::insertColumns(int position, int columns, const QModelIndex &parent) {
    bool success;
    beginInsertColumns(parent, position, position + columns - 1);
    success = rootItem->insertColumns(position, columns);
    endInsertColumns();
    return success;
}

bool TreeModel::insertRows(int position, int rows, const QModelIndex &parent) {
    TreeItem *parentItem = getItem(parent);
    bool success;
    beginInsertRows(parent, position, position + rows - 1);
    success = parentItem->insertChildren(position, rows, rootItem->columnCount());
    endInsertRows();
    return success;
}

bool TreeModel::removeColumns(int position, int columns, const QModelIndex &parent) {
    bool success;
    beginRemoveColumns(parent, position, position + columns - 1);
    success = rootItem->removeColumns(position, columns);
    endRemoveColumns();
    if (rootItem->columnCount() == 0) removeRows(0, rowCount());
    return success;
}

bool TreeModel::removeRows(int position, int rows, const QModelIndex &parent) {
    TreeItem *parentItem = getItem(parent);
    bool success = true;
    beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    endRemoveRows();
    return success;
}
#endif // TREEMODEL_HPP
