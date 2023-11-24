#ifndef IS_OS_ANDROID
#include "treeviewwidget.h"
#include "sort/treesortmodel.h"
#include <QWidget>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QTapAndHoldGesture>
#include <QEvent>
#include <QGestureEvent>
#include <QDrag>
#include <QApplication>
#include <QMimeData>
#include "rowdialog.h"
#include <QMessageBox>
#include "gui/selectgroupdialog.h"
#include "gui/selectitemdialog.h"

using namespace arcirk::tree_widget;
TreeViewWidget::TreeViewWidget(QWidget *parent, const QString& typeName)
    : QTreeView(parent)
{

    this->setDragDropMode(QAbstractItemView::InternalMove);
    this->setSelectionMode(QAbstractItemView::SingleSelection);
    this->setDragEnabled(true);
    this->setAcceptDrops(true);
    this->setDropIndicatorShown(true);

    m_sort_model = new TreeSortModel(this);
    connect(m_sort_model, &TreeSortModel::modelChanged, this, &TreeViewWidget::onSourceModelChanged);
    this->setModel(m_sort_model);

    //is_proxy = false;
    m_behavior = behaviorDefault;
    m_drag_groups = false;
    data_is_base64_ = false;
    m_hierarchy_list = true;
    m_not_sort = false;
    m_toolBar = nullptr;
    m_inners_dialogs = false;
    m_only_groups_in_root = false;

    setProperty("typeName", typeName);

    connect(this, &QAbstractItemView::clicked, this, &TreeViewWidget::onItemClicked);
    connect(this, &QAbstractItemView::doubleClicked, this, &TreeViewWidget::onItemDoubleClicked);


    auto delegate = new TreeItemDelegate(false, this);
    setItemDelegate(delegate);
    setEditTriggers(QAbstractItemView::AllEditTriggers);

}

TreeItemModel* TreeViewWidget::get_model()
{
    if(this->model()==0)
        return nullptr;
    if(this->model()->property("typeName").toString() == "TreeSortModel"){
        auto m_model = (TreeSortModel*)this->model();
        return (TreeItemModel*)m_model->sourceModel();
    }else{
        return (TreeItemModel*)this->model();
    }
}

QModelIndex TreeViewWidget::current_index() const
{
    auto index = currentIndex();
    if(!index.isValid())
        return QModelIndex();

    if(this->model()->property("typeName").toString() == "TreeSortModel"){
        auto sort_model = (TreeSortModel*)this->model();
        return sort_model->mapToSource(index);
    }else
        return index;
}

void TreeViewWidget::set_current_index(const QModelIndex &index)
{
    if(!index.isValid())
        return;

    if(this->model()->property("typeName").toString() == "TreeSortModel"){
        auto sort_model = (TreeSortModel*)this->model();
        auto sort_index = sort_model->mapFromSource(index);
        setCurrentIndex(sort_index);
    }else
        setCurrentIndex(index);
}

QModelIndex TreeViewWidget::get_index(const QModelIndex &sort_index) const
{
    if(!sort_index.isValid())
        return QModelIndex();
    if(this->model()->property("typeName").toString() == "TreeSortModel"){
        auto sort_model = (TreeSortModel*)this->model();
        return sort_model->mapToSource(sort_index);
    }else
        return sort_index;
}

void TreeViewWidget::insert(const json &object, const QModelIndex &parent)
{
    auto model = get_model();
    if(!model)
        return;
    auto index = model->find(QUuid::fromString(object.value("ref", "").c_str()), QModelIndex());
    if(!index.isValid())
        model->add(object, parent);
}

void TreeViewWidget::remove(const QString &uuid)
{
    auto model = get_model();
    if(!model)
        return;
    auto index = model->find(QUuid::fromString(uuid), QModelIndex());
    if(index.isValid()){
        model->remove(index);
    }
}

void TreeViewWidget::set_drag_exceptions(const QMap<int, QList<QVariant>>& values)
{
    m_drag_exceptions = values;
}

void TreeViewWidget::set_drop_exceptions(const QMap<int, QList<QVariant>>& values)
{
    m_drop_exceptions = values;
}

void TreeViewWidget::set_hierarchy_list(bool value)
{
    m_hierarchy_list = value;

    if(m_hierarchy_list){
        this->resetIndentation();
    }else
        this->setIndentation(0);

}

void TreeViewWidget::setModel(QAbstractItemModel *model)
{
    if(model==0){
        return QTreeView::setModel(model);
    }
    if(model->property("typeName").toString() != "TreeSortModel"){
        m_sort_model->setSourceModel(model);
        connect((TreeItemModel*)model, &TreeItemModel::fetch, this, &TreeViewWidget::onTreeFeth);
//        auto model_ = qobject_cast<TreeItemModel*>(model);
//        if(model_!=0){
//            set_hierarchy_list(model_->hierarchical_list());
//            auto delegate = (TreeItemDelegate*)this->itemDelegate();
//            if(delegate!=0)
//                delegate->setGridLine(!model_->hierarchical_list());
//        }

    }
    //else{
        //auto m_sort = (TreeSortModel*)model;
        //connect(m_sort, &TreeSortModel::modelChanged, this, &TreeViewWidget::onSourceModelChanged);
        //if(m_sort->sourceModel()!=0){
        //    auto model_ = (TreeItemModel*)m_sort->sourceModel();
//            if(model_!=0){
//                set_hierarchy_list(model_->hierarchical_list());
//                auto delegate = (TreeItemDelegate*)this->itemDelegate();
//                if(delegate!=0)
//                    delegate->setGridLine(!model_->hierarchical_list());
//            }
//        }
//    }
    return QTreeView::setModel(m_sort_model);


}

void TreeViewWidget::setEnabled(bool value)
{
    if(m_toolBar != 0)
        m_toolBar->setEnabled(value);
}

int TreeViewWidget::content_width(int column)
{
    auto model = get_model();
    int result = 0;
    for (int i = 0; i < model->rowCount(); ++i) {
        QFontMetrics fm(fontMetrics());
        auto text = model->index(i, column).data().toString();
        result = std::max(fm.horizontalAdvance(text), result);
        auto index = model->index(i, column, QModelIndex());
        result = std::max(text_width(column, index), result);
    }
    return result;
}

void TreeViewWidget::clear()
{
    m_sort_model->setSourceModel(nullptr);
}

void TreeViewWidget::setTableToolBar(TableToolBar *value)
{
    m_toolBar = value;
    connect(m_toolBar, &TableToolBar::itemClicked, this, &TreeViewWidget::onToolBarItemClicked);
    auto model = get_model();
    if(!model)
        return;
    m_toolBar->setHierarchyState(model->hierarchical_list());
}

void TreeViewWidget::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    if(m_toolBar){
        auto model = get_model();
        auto index= get_index(current);
        if(!model){
            return QTreeView::currentChanged(current, previous);;
        }
        if(index.isValid()){
            m_toolBar->setButtonEnabled("edit_item", true);
            m_toolBar->setButtonEnabled("delete_item", true);
            if(model->is_group(index)){
                m_toolBar->setButtonEnabled("add_group", true);
                m_toolBar->setButtonEnabled("add_item", true);
            }else{
                m_toolBar->setButtonEnabled("add_group", false);
                m_toolBar->setButtonEnabled("add_item", false);
            }

            m_toolBar->setButtonEnabled("move_to_item", true);
            if(!model->hierarchical_list()){
                if(index.row() < model->rowCount() - 1)
                    m_toolBar->setButtonEnabled("move_down_item", true);

                if(index.row() != 0)
                    m_toolBar->setButtonEnabled("move_up_item", true);
            }
        }else{
            m_toolBar->setButtonEnabled("add_group", false);
            m_toolBar->setButtonEnabled("add_item", false);
            m_toolBar->setButtonEnabled("move_to_item", false);
            m_toolBar->setButtonEnabled("delete_item", false);
            m_toolBar->setButtonEnabled("edit_item", false);
            m_toolBar->setButtonEnabled("move_down_item", false);
            m_toolBar->setButtonEnabled("move_up_item", false);
        }
    }

    emit treeItemClicked(current);
    return QTreeView::currentChanged(current, previous);
}


void TreeViewWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        startPos = event->pos();
    return QTreeView::mousePressEvent(event);
}

void TreeViewWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        int distance = (event->pos() - startPos).manhattanLength();
        if (distance >= QApplication::startDragDistance())
            performDrag();
    }
    return QTreeView::mouseMoveEvent(event);
}

void TreeViewWidget::dragMoveEvent(QDragMoveEvent *event)
{
    //qDebug() << __FUNCTION__;
    return QTreeView::dragMoveEvent(event);
}

void TreeViewWidget::dropEvent(QDropEvent *event)
{
    auto model = get_model();
    if(!model)
        return;
    if(!model->enable_drop())
        return;
    auto text = event->mimeData()->text();
    auto sender = event->mimeData()->property("typeName");
    if(json::accept(text.toStdString())){
        auto model = get_model();
        auto empty = model->empty_data();
        tree::fill_property_values(empty, json::parse(text.toStdString()));
        if(m_behavior == behaviorDefault){
            auto index = model->add(empty);
            emit doDropEvent(index, sender.toString());
        }else
            emit doDropEventJson(empty, sender.toString());
    }
    return QTreeView::dropEvent(event);
}

void TreeViewWidget::dragEnterEvent(QDragEnterEvent *event)
{
    //qDebug() << __FUNCTION__;
    auto model = get_model();
    if(!model)
        return;
    if(!model->enable_drag())
        return;
    event->acceptProposedAction();
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::InternalMove);
    return QTreeView::dragEnterEvent(event);
}

void TreeViewWidget::changeEvent(QEvent *event)
{
    return QTreeView::changeEvent(event);
}

void TreeViewWidget::performDrag() {

    //    qDebug() << __FUNCTION__;

    auto model = get_model();
    if(!model)
        return;
    auto index = current_index();
    if(!m_drag_groups){
        if(model->rowCount(index)> 0)
            return;
    }
    if(m_drag_exceptions.size() > 0){
        for (auto itr = m_drag_exceptions.begin(); itr != m_drag_exceptions.end(); ++itr) {
            QVariant val = model->index(index.row(), itr.key(), index.parent()).data();
            QList<QVariant> lst = itr.value();
            if(lst.indexOf(val) != -1)
                return;
        }
    }
    QMimeData *mimeData = new QMimeData;
    mimeData->setProperty("typeName", this->property("typeName"));
    if(m_behavior == behaviorDefault || m_behavior == behaviorEmit )
        mimeData->setText(model->to_object(index).dump().c_str());
    else if(m_behavior == behaviorText){
        if(m_drop_column.isEmpty()){
            auto text = index.data().toString();
            if(data_is_base64_)
                text = QByteArray::fromBase64(text.toUtf8());
            mimeData->setText(index.data().toString());
        }else{
            int i = model->column_index(m_drop_column);
            if(i != -1){
                auto text = model->index(index.row(), i, index.parent()).data().toString();
                if(data_is_base64_)
                    text = QByteArray::fromBase64(text.toUtf8());
                mimeData->setText(text);
            }
        }
    }
    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->exec(Qt::MoveAction);

}

int TreeViewWidget::text_width(int column, const QModelIndex &parent, const int& result)
{
    auto model = get_model();
    if(!model)
        return 0;
    int res = result;
    for (int itr = 0; itr < model->rowCount(parent); ++itr) {
        QString text = model->data(model->index(itr, column, parent)).toString();
        if(!text.isEmpty()){
            QFontMetrics fm(fontMetrics());
            res = std::max(fm.horizontalAdvance(text), res);

        }
        res = text_width(column, model->index(itr, column, parent), res);
    }
    return res;
}

void TreeViewWidget::onItemClicked(const QModelIndex &index)
{
    if(!index.isValid())
        return;
    auto index_ = get_index(index);
    emit itemClicked(index_, get_model()->object_name(index_));
}

void TreeViewWidget::onItemDoubleClicked(const QModelIndex &index)
{
    if(!index.isValid())
        return;
    auto index_ = get_index(index);
    emit itemDoubleClicked(index_, get_model()->object_name(index_));
}

void TreeViewWidget::onSourceModelChanged()
{
    auto model_ = get_model();
    if(model_!=0){
        set_hierarchy_list(model_->hierarchical_list());
        auto delegate = (TreeItemDelegate*)this->itemDelegate();
        if(delegate!=0)
            delegate->setGridLine(!model_->hierarchical_list());
        if(!m_not_sort)
            setSortingEnabled(!model_->hierarchical_list());
        else
            setSortingEnabled(false);
    }
}

void TreeViewWidget::onTreeFeth(const QModelIndex &parent)
{
    emit fetch(parent);
}

void TreeViewWidget::onToolBarItemClicked(const QString &buttonName)
{
    if(!m_inners_dialogs)
        emit toolBarItemClicked(buttonName);
    else{
        auto model = get_model();
        if(!model)
            return;
        json b = buttonName.toStdString();
        auto btn = b.get<toolbar_buttons>();
        auto current_index = this->current_index();
        json row_data = model->empty_data();
        if(btn == add_item){
            if(m_only_groups_in_root){
                if (!current_index.isValid() || !model->is_group(current_index))
                    return;
                row_data["is_group"] = 0;
                row_data["ref"] = "";
                row_data["parent"] = quuid_to_string(model->ref(current_index)).toStdString();
            }else
                row_data["parent"] = NIL_STRING_UUID;

            User_Data m_udata{};
            for (int i = 0; i < model->rowCount(); ++i) {
                auto ctrl = model->user_role_data(model->column_name(i), tree::WidgetRole);
                auto ext = model->user_role_data(model->column_name(i), tree::UserRoleExt);
                m_udata.insert(model->column_name(i).toStdString(),
                               qMakePair(ctrl, ext));
            }
            QString m_parent_name = "";
            if(row_data["parent"] != NIL_STRING_UUID){
                auto indexParent = model->find(QUuid::fromString(row_data["parent"].get<std::string>().c_str()));
                if(indexParent.isValid()){
                    auto obj = model->to_object(indexParent);
                    m_parent_name = obj.value("first", "").c_str();
                    if(m_parent_name.isEmpty())
                        m_parent_name = obj.value("name", "").c_str();
                }
            }
            std::string m_current_parent = row_data.value("parent", NIL_STRING_UUID);
            auto dlg = RowDialog(row_data, m_udata, this, model->column_aliases_default(),
                                 QList<QString>{"data_type", "_id", "path"},
                                 model->columns_order(), model->not_null_fields(),
                                 m_parent_name);
            dlg.setIcon(model->rows_icon(item_icons_enum::Item));
            if(dlg.exec()){
                auto data = dlg.data();
                if(data["parent"].get<std::string>() != m_current_parent)
                    current_index = QModelIndex();
                auto n_index = model->add(dlg.data(), current_index);
                emit addTreeItem(n_index, dlg.data());
            }

        }else if(btn == add_group){
            row_data["is_group"] = 1;
            row_data["ref"] = "";
            if(current_index.isValid() && model->is_group(current_index)){
                row_data["parent"] = quuid_to_string(model->ref(current_index)).toStdString();
            }else{
                if(!model->is_group(current_index))
                    return;
                row_data["parent"] = NIL_STRING_UUID;
            }
            User_Data m_udata{};
            for (int i = 0; i < model->rowCount(); ++i) {
                auto ctrl = model->user_role_data(model->column_name(i), tree::WidgetRole);
                auto ext = model->user_role_data(model->column_name(i), tree::UserRoleExt);
                m_udata.insert(model->column_name(i).toStdString(),
                               qMakePair(ctrl, ext));
            }
            QString m_parent_name = "";
            std::string m_current_parent = row_data.value("parent", NIL_STRING_UUID);
            if(row_data["parent"] != NIL_STRING_UUID){
                auto indexParent = model->find(QUuid::fromString(row_data["parent"].get<std::string>().c_str()));
                if(indexParent.isValid()){
                    auto obj = model->to_object(indexParent);
                    m_parent_name = obj.value("first", "").c_str();
                    if(m_parent_name.isEmpty())
                        m_parent_name = obj.value("name", "").c_str();
                }
            }
            auto dlg = RowDialog(row_data, m_udata, this, model->column_aliases_default(), QList<QString>{"data_type", "_id", "path"},
                                 model->columns_order(),
                                 model->not_null_fields(),
                                 m_parent_name);
            dlg.setIcon(model->rows_icon(item_icons_enum::ItemGroup));
            if(dlg.exec()){
                auto data = dlg.data();
                if(data["parent"].get<std::string>() != m_current_parent)
                    current_index = QModelIndex();
                auto n_index = model->add(dlg.data(), current_index);
                emit addTreeItem(n_index, dlg.data());
            }
        }else if(btn == delete_item){
            if(!current_index.isValid())
                return;
            if(QMessageBox::question(this, "Удаление", "Удалить выбранную строку?") == QMessageBox::Yes){
                auto obj = model->to_object(current_index);
                model->remove(current_index);
                emit deleteTreeItem(obj);
            }
        }else if(btn == edit_item){
            if(!current_index.isValid())
                return;
            row_data = model->to_object(current_index);
            QString m_parent_name = "";
            if(row_data["parent"] != NIL_STRING_UUID){
                auto indexParent = model->find(QUuid::fromString(row_data["parent"].get<std::string>().c_str()));
                if(indexParent.isValid()){
                    auto obj = model->to_object(indexParent);
                    m_parent_name = obj.value("first", "").c_str();
                    if(m_parent_name.isEmpty())
                        m_parent_name = obj.value("name", "").c_str();
                }
            }
            User_Data m_udata{};
            for (int i = 0; i < model->rowCount(); ++i) {
                auto ctrl = model->user_role_data(model->column_name(i), tree::WidgetRole);
                auto ext = model->user_role_data(model->column_name(i), tree::UserRoleExt);
                m_udata.insert(model->column_name(i).toStdString(),
                               qMakePair(ctrl, ext));
            }
            auto dlg = RowDialog(row_data, m_udata, this, model->column_aliases_default(), QList<QString>{"data_type", "_id", "path"}, model->columns_order(), {}, m_parent_name);
            if(dlg.exec()){
                model->set_object(current_index, dlg.data());
                emit editTreeItem(current_index, dlg.data());
            }
        }else if(btn == move_to_item){
            if(!current_index.isValid())
                return;

            auto table = model->to_table_model(QModelIndex(),true, true);
            auto gr_model = new TreeItemModel(this);
            gr_model->set_columns_order(model->columns_order());
            gr_model->set_column_aliases(gr_model->column_aliases_default());
            gr_model->set_table(table);

            auto dlg = SelectItemDialog(gr_model, this);
            dlg.setWindowTitle("Выбрать группу");
            if(dlg.exec()){
                auto obj = dlg.result();
                auto parent_index = model->find(QUuid::fromString(obj.value("ref", NIL_STRING_UUID).c_str()));
                auto current_index = this->current_index();
                auto current_oject = model->to_object(current_index);
                if(parent_index.isValid()){
                    auto verify = model->belongsToItem(parent_index, current_index);
                    if(verify){
                        QMessageBox::critical(this, "Ошибка", "В выбранную группу перемещение не возможно!");
                        return;
                    }
                    if(current_index.parent() == parent_index)
                        return;
                    model->move_to(current_index, parent_index);
                    auto n_index = model->find(QUuid::fromString(current_oject["ref"].get<std::string>().c_str()), parent_index);
                    if(n_index.isValid())
                        emit editTreeItem(n_index, model->to_object(n_index));
                }

            }
        }
    }
}


#endif
