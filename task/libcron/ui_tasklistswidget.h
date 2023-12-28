/********************************************************************************
** Form generated from reading UI file 'tasklistswidget.ui'
**
** Created by: Qt User Interface Compiler version 6.5.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TASKLISTSWIDGET_H
#define UI_TASKLISTSWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TaskListsWidget
{
public:
    QGridLayout *gridLayout;
    QVBoxLayout *verticalLayout;

    void setupUi(QWidget *TaskListsWidget)
    {
        if (TaskListsWidget->objectName().isEmpty())
            TaskListsWidget->setObjectName("TaskListsWidget");
        TaskListsWidget->resize(427, 189);
        gridLayout = new QGridLayout(TaskListsWidget);
        gridLayout->setObjectName("gridLayout");
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName("verticalLayout");

        gridLayout->addLayout(verticalLayout, 0, 0, 1, 1);


        retranslateUi(TaskListsWidget);

        QMetaObject::connectSlotsByName(TaskListsWidget);
    } // setupUi

    void retranslateUi(QWidget *TaskListsWidget)
    {
        TaskListsWidget->setWindowTitle(QCoreApplication::translate("TaskListsWidget", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class TaskListsWidget: public Ui_TaskListsWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TASKLISTSWIDGET_H
