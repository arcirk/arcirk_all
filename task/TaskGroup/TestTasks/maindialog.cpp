#include "maindialog.h"
#include "ui_maindialog.h"
#include <iostream>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QTime>

MainDialog::MainDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MainDialog)
{
    ui->setupUi(this);

//    using namespace std::chrono_literals;

//    libcron::Cron cron;
//    cron.add_schedule("Cron", "0 */1 * * * ?", [](auto& i) {
//        qDebug() << i.get_name().c_str() << QDateTime::currentDateTime().toString();
//    });

//    while(true)
//    {
//        cron.tick();
//        std::this_thread::sleep_for(1000ms);
//    }

//    auto cron = libcron::CronData::create(schedule);

    m_task_list = new tasks::TaskListsWidget(this);
    ui->verticalLayout->addWidget(m_task_list);

    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("Отмена");
    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &MainDialog::onButtonBoxClicked);

    m_task_list->load();

    connect(m_task_list, &tasks::TaskListsWidget::taskListChanged, this, &MainDialog::onTaskListChanged);
    connect(m_task_list, &tasks::TaskListsWidget::startTask, this, &MainDialog::onStartTask);
}

MainDialog::~MainDialog()
{
    delete ui;
}

void MainDialog::onButtonBoxClicked(QAbstractButton *button)
{
    if(button == ui->buttonBox->button(QDialogButtonBox::Cancel))
        QApplication::quit();
}

void MainDialog::onTaskListChanged()
{
    m_task_list->save();
}

void MainDialog::onStartTask(const arcirk::tasks::task_options &task)
{

//    auto uuid = QUuid::fromString(task.uuid.c_str());
//    if(m_runtables.find(uuid) == m_runtables.end()){
//        auto r_task = new CronWorker(task);
//        //connect(this, &MainDialog::taskStop, r_task, &CronWorker::onStop);
//        //connect(r_task, &CronWorker::activate, this, &MainDialog::onTaskActivate);
//        QThreadPool::globalInstance()->start(r_task);
//        //m_runtables.insert(uuid, r_task);
//    }else{
//        auto itr = m_runtables[uuid];
//        if(itr){
//            if(itr->isRunning())
//                QMessageBox::critical(this, "Ошибка", "Задача уже запущена!");
//        }
//    }

//    using namespace std::chrono_literals;
//    while(true)
//    {
//        cron.tick();
//        std::this_thread::sleep_for(500ms);
//    }
//    QString s = task.cron_string.c_str();
//    QStringList ss = s.split(" ");
//    ss[ss.size()-1] = "?";

//    std::map<std::string, std::string> name_schedule_map;
//    for (int i = 0; i < 10; ++i) {
//        ss[1] = QString("*/%1").arg(QString::number(1+i));
//        name_schedule_map.insert(std::pair<std::string, std::string>(QString("Задача %1").arg(QString::number(i)).toStdString(),
//                                                                     ss.join(" ").toStdString()));
//    }
//    auto cron = std::make_shared<libcron::Cron<libcron::LocalClock, libcron::Locker> >();
//    cron->add_schedule(name_schedule_map, [=](auto& i) {
//        //emit activate(task_);
//        qDebug() << i.get_name().c_str();
//        auto r_task = new CronWorker(task);
//        QThreadPool::globalInstance()->start(r_task);
//    });

//    auto cron = std::make_shared<libcron::Cron<libcron::LocalClock, libcron::Locker> >();
//    cron->add_schedule(task.name, ss.join(" ").toStdString(), [=](auto& i) {
//        //emit activate(task_);
//        qDebug() << i.get_name().c_str();
//        auto r_task = new CronWorker(task);
//        QThreadPool::globalInstance()->start(r_task);
//    });

//    auto m_timer = new QTimer(this);
//    auto timerTimeout = [cron]()->void {


////        qDebug() << "cron.tick()";
////        try {
//        cron->tick();
////        } catch (const std::exception& e) {
////            qDebug() << e.what();
////        }

//    };

//    QObject::connect(m_timer, &QTimer::timeout, timerTimeout);

//    m_timer->start(1000);
}

void MainDialog::onTaskActivate(const arcirk::tasks::task_options &task)
{
    qDebug() << QDateTime::currentDateTime().toString() << "task:" << task.name.c_str();
}
