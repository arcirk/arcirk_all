#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QDialog>
#include "tasklistswidget.h"
#include "cronworker.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainDialog; }
QT_END_NAMESPACE

class MainDialog : public QDialog
{
    Q_OBJECT

public:
    MainDialog(QWidget *parent = nullptr);
    ~MainDialog();

private slots:
    void onButtonBoxClicked(QAbstractButton *button);
    void onTaskListChanged();
    void onStartTask(const arcirk::tasks::task_options& task);
    void onTaskActivate(const arcirk::tasks::task_options& task);

private:
    Ui::MainDialog *ui;
    tasks::TaskListsWidget * m_task_list;
    QMap<QUuid, CronWorker*> m_runtables;

signals:
    void taskStop();

};
#endif // MAINDIALOG_H
