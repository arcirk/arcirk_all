#include "cronworker.h"
#include <QDebug>

CronWorker::CronWorker(const arcirk::tasks::task_options& task)
    : task_(task)
{
    m_timer = nullptr;
    //setAutoDelete(false);
}

CronWorker::~CronWorker()
{
    qDebug() << "CronWorker exit";
}

void CronWorker::run()
{
//    libcron::Cron<libcron::LocalClock, libcron::Locker> cron;
//    m_timer = new QTimer();
//    //m_timer->setSingleShot(true);
////    //connect(m_timer,SIGNAL(timeout()),this,SLOT(onVerify()));
//    auto timerTimeout = [&cron]()->void {
//        qDebug() << "cron.tick()";
//        cron.tick();
//    };

//    //m_timer->connect(m_timer,SIGNAL(timeout()), &timerTimeout);
//    QObject::connect(m_timer, &QTimer::timeout, timerTimeout);

//    cron.add_schedule(task_.name, task_.cron_string, [=](auto& i) {
//        //emit activate(task_);
//        qDebug() << i.get_name().c_str();
//    });

//    m_timer->start(1000);
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1500ms);

}

bool CronWorker::isRunning()
{
//    if(m_timer)
//        return m_timer->isActive();
//    else
        return false;
}

//void CronWorker::onVerify()
//{
//    cron.tick();
//}

//void CronWorker::onStop()
//{
//    cron.remove_schedule(task_.name);
//    m_timer->stop();
//}
