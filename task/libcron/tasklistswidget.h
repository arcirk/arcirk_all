#ifndef TASKLISTSWIDGET_H
#define TASKLISTSWIDGET_H

#include <QWidget>
#include "iface/iface.hpp"
#include "gui/treeviewwidget.h"
#include "gui/tabletoolbar.h"
#include "common.hpp"
#include "cronworker.h"

using namespace arcirk::tree_widget;
using namespace arcirk::tree_model;


namespace Ui {
class TaskListsWidget;
}

namespace arcirk::tasks {

typedef arcirk::tree_model::ITree<arcirk::tasks::task_options> ITreeTaskModel;

    class TaskListsWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit TaskListsWidget(QWidget *parent = nullptr);
        ~TaskListsWidget();

        void save();
        void load();

    private:
        Ui::TaskListsWidget *ui;
        TreeViewWidget* treeView;
        TableToolBar* m_toolBar;


    private slots:
        void onToolBarItemClicked(const QString& buttonName);
        void onTreeItemDoubleClicked(const QModelIndex& index, const QString& item_name);


    signals:
        void taskListChanged();
        void startTask(const arcirk::tasks::task_options& task);

    };
}
#endif // TASKLISTSWIDGET_H
