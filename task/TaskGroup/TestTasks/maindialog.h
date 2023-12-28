#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QDialog>
#include "iface/iface.hpp"
#include "tasklistswidget.h"

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

private:
    Ui::MainDialog *ui;
    tasks::TaskListsWidget * m_task_list;
};
#endif // MAINDIALOG_H
