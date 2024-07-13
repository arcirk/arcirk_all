#include "tasklistswidget.h"
#include "ui_tasklistswidget.h"
#include "taskitem.h"
#include <QToolButton>
#include "propertydialog.h"
#include "croncpp.hpp"

using namespace arcirk::tasks;

TaskListsWidget::TaskListsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TaskListsWidget)
{
    ui->setupUi(this);

    Q_INIT_RESOURCE(cron_res);

    auto model = new ITreeTaskModel(this);
    model->set_hierarchical_list(false);
    model->enable_drag_and_drop(false);
    model->set_column_aliases(QMap<QString,QString>{
        qMakePair("uuid", "Ссылка"),
        qMakePair("name", "Наименование"),
        qMakePair("synonum", "Представление"),
        qMakePair("start_task", "Начало"),
        qMakePair("end_task", "Конец"),
        qMakePair("interval", "Интервал"),
        qMakePair("allowed", "Разрешено"),
        qMakePair("days_of_week", "Выполнять по дням"),
        qMakePair("script", "Скрипт"),
        qMakePair("comment", "Комментарий"),
        qMakePair("predefined", "Предопределенное"),
        qMakePair("cron_string", "Параметры Cron"),
    });
    model->set_enable_rows_icons(false);
    model->set_columns_order(QList<QString>{"allowed", "synonum", "start_task", "end_task", "interval"});

    m_toolBar = new TableToolBar(this);
    treeView = new TreeViewWidget(this);

    treeView->setModel(model);
    treeView->setTableToolBar(m_toolBar);

    treeView->set_inners_dialogs(false);
    treeView->allow_def_commands(false);
    treeView->enable_sort(false);
    treeView->hide_default_columns();
    treeView->hideColumn(model->column_index("script"));
    treeView->hideColumn(model->column_index("script_synonum"));
    treeView->hideColumn(model->column_index("param"));
    treeView->hideColumn(model->column_index("reset_version"));
    treeView->hideColumn(model->column_index("uuid"));

    m_toolBar->addButton("btnTaskStart", QIcon(":/img/startTask.png"));
    m_toolBar->addButton("btnTaskPause", QIcon(":/img/taskPause.png"));
    m_toolBar->insertSeparator();
    m_toolBar->addButton("btnTaskParam", QIcon(":/img/options.png"));
    m_toolBar->insertSeparator();
    m_toolBar->addButton("btnGetTimeUntil", QIcon(":/img/options.png"));

    ui->verticalLayout->addWidget(m_toolBar);
    ui->verticalLayout->addWidget(treeView);

    auto header = treeView->header();
    header->resizeSection(0, 20);
    header->setSectionResizeMode(0, QHeaderView::Fixed);

    connect(treeView, &TreeViewWidget::toolBarItemClicked, this, &TaskListsWidget::onToolBarItemClicked);
    connect(treeView, &TreeViewWidget::itemDoubleClicked, this, &TaskListsWidget::onTreeItemDoubleClicked);
}

TaskListsWidget::~TaskListsWidget()
{
    delete ui;
}

void TaskListsWidget::save()
{
    auto model = treeView->get_model();
    auto table = model->to_table_model(QModelIndex());
    auto file_name = QPath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    file_name /= "tasks";
    if(!file_name.exists())
        file_name.mkpath();

    file_name /= "tasks.json";
    auto f = QFile(file_name.path());
    if(f.open(QIODevice::WriteOnly)){
        f.write(table.dump().c_str());
        f.close();
    }

}

void TaskListsWidget::load()
{
    auto model = treeView->get_model();
    auto table = model->to_array(QModelIndex());
    auto file_name = QPath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    file_name /= "tasks";
    file_name /= "tasks.json";

    auto f = QFile(file_name.path());
    if(f.open(QIODevice::ReadOnly)){
        auto str = f.readAll();
        if(json::accept(str.toStdString())){
            auto table = json::parse(str.toStdString());
            model->set_table(table);
        }
    }
}

void TaskListsWidget::onToolBarItemClicked(const QString &buttonName)
{
    if(buttonName == "add_item"){
        auto task = task_options();
        auto dlg = TaskItem(task, this);
        if(dlg.exec()){
            auto model = treeView->get_model();
            model->add(pre::json::to_json(task));
            emit taskListChanged();
        }
    }else if(buttonName == "edit_item"){
        const auto index = treeView->current_index();
        if(index.isValid()){
            auto model = (ITreeTaskModel)treeView->get_model();
            auto task = model.object(index);
            auto dlg = TaskItem(task, this);
            if(dlg.exec()){
                model.set_struct(task, index);
                emit taskListChanged();
            }
        }
    }else if(buttonName == "delete_item"){
        const auto index = treeView->current_index();
        if(index.isValid()){
            if(QMessageBox::question(this, "Удаление", "Удалить выбранную задачу?") == QMessageBox::Yes){
                auto model = (ITreeTaskModel)treeView->get_model();
                model.remove(index);
                emit taskListChanged();
            }
        }
    }else if(buttonName == "move_up_item"){
        treeView->moveUp();
        emit taskListChanged();
    }else if(buttonName == "move_down_item"){
        treeView->moveDown();
        emit taskListChanged();
    }else if(buttonName == "btnTaskParam"){
        const auto index = treeView->current_index();
        if(index.isValid()){
            auto model = (ITreeTaskModel*)treeView->get_model();
            auto task = model->object(index);
            auto str = arcirk::byte_array_to_string(task.param);
            json param;
            if(json::accept(str)){
                param = json::parse(str);
            }
            if(param.empty()){
                param = json::object();
                param["columns"] = json::array({"key", "value"});
                param["rows"] = json::array();
            }
            auto dlg = plugins::PropertyDialog(param, this);
            if(dlg.exec()){
                task.param = arcirk::string_to_byte_array(dlg.result().dump());
                model->set_struct(task, index);
                emit taskListChanged();
            }
        }

    }else if(buttonName == "btnTaskStart"){
        const auto index = treeView->current_index();
        if(index.isValid()){
            auto model = (ITreeTaskModel*)treeView->get_model();
            auto task = model->object(index);
            emit startTask(task);
        }
    }else if(buttonName == "btnGetTimeUntil"){
        auto model = (ITreeTaskModel*)treeView->get_model();
        auto arr = model->array(QModelIndex(), true);

        //libcron::Cron<libcron::LocalClock, libcron::Locker> cron;

        std::time_t now = std::time(0);
        foreach (auto itr, arr) {
//            auto res = cron.add_schedule(itr.name, itr.cron_string, [](auto&) {});
//            qDebug() << res;
            auto cron = cron::make_cron(itr.cron_string);
            for (int y = 0; y < 5; ++y) {
                std::time_t next = cron::cron_next(cron, now);
                //std::cout << ctime(&next) << std::endl;
                qDebug() << "Следующее выполение: " << ctime(&next) ;
                now = next;
            }

        }
//        cron.tick();
//        std::vector<std::tuple<std::string,
//                               std::chrono::system_clock::duration>> status;
//        auto tp1 = std::chrono::system_clock::now();
//        cron.get_time_until_expiry_for_tasks(status);

//        auto t = time(0);
//        qDebug() << "Следующее выполение: " << ctime(&t) << std::get<1>(status[0]).count();

//        for (int i = 0; i < status.size(); ++i) {

//            time_t t = std::chrono::system_clock::to_time_t(tp1 + std::get<1>(status[i]));
//            qDebug() << "Следующее выполение: " << ctime(&t);
////            auto next = tp1 + std::get<1>(status[i]);
//////            t = std::chrono::system_clock::to_time_t(next);
//////            qDebug() << "test: " << ctime(&t);
//            auto next = cron.get_clock().now() +  std::get<1>(status[i]);
//            auto secs = std::chrono::seconds(8 * 60 * 60);
//            const auto offset = duration_cast<std::chrono::system_clock::duration>(secs);
//            for (int y = 0; y < 5; ++y) {
//                cron.tick();
//                cron.get_time_until_expiry_for_tasks(status, next);
////                tp1 = next - offset;
////                time_t t = std::chrono::system_clock::to_time_t(tp1 + std::get<1>(status[i]));
////                qDebug() << "Следующее выполение: " << ctime(&t);

//                next = next + std::get<1>(status[i]);
//                tp1 = next - offset;
//                time_t t = std::chrono::system_clock::to_time_t(tp1 + std::get<1>(status[i]));
//                qDebug() << "Следующее выполение: " << ctime(&t) << std::get<1>(status[i]).count();
//            }


//        }
    }

}

void TaskListsWidget::onTreeItemDoubleClicked(const QModelIndex &index, const QString &item_name)
{
    if(!index.isValid())
        return;

    auto model = (ITreeTaskModel)treeView->get_model();
    auto task = model.object(index);
    auto dlg = TaskItem(task, this);
    if(dlg.exec()){
        model.set_struct(task, index);
        emit taskListChanged();
    }
}

