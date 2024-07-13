#ifndef CRONWORKER_H
#define CRONWORKER_H

#include <QRunnable>
#include <QObject>
#include "common.hpp"
#include <QTimer>


class CronWorker : public QRunnable
{
    //Q_OBJECT
public:
    explicit CronWorker(const arcirk::tasks::task_options& task);
    ~CronWorker();
    void run();
    bool isRunning();

//signals:
//    void activate(const arcirk::tasks::task_options& task);

private:
    QTimer * m_timer;

protected:
    arcirk::tasks::task_options task_;

//private slots:
//    void onVerify();

//public slots:
//    void onStop();

};

#endif // CRONWORKER_H
