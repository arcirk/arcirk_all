/********************************************************************************
** Form generated from reading UI file 'taskitem.ui'
**
** Created by: Qt User Interface Compiler version 6.5.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TASKITEM_H
#define UI_TASKITEM_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QTimeEdit>
#include <QtWidgets/QToolButton>

QT_BEGIN_NAMESPACE

class Ui_TaskItem
{
public:
    QGridLayout *gridLayout_2;
    QGridLayout *gridLayout;
    QTimeEdit *start_task;
    QCheckBox *allowed;
    QLabel *label_8;
    QLabel *label;
    QLabel *label_3;
    QLabel *lblCronString;
    QLineEdit *name;
    QLineEdit *uuid;
    QLabel *label_2;
    QLabel *label_7;
    QHBoxLayout *horizontalLayout;
    QSpinBox *interval;
    QLabel *label_6;
    QLabel *label_4;
    QComboBox *cmbScriptType;
    QHBoxLayout *horizontalLayout_2;
    QLineEdit *script;
    QToolButton *btnSelectPlugin;
    QToolButton *btnPluginParam;
    QToolButton *btnSave;
    QLineEdit *synonum;
    QLabel *label_9;
    QLabel *label_5;
    QLabel *label_10;
    QLabel *label_13;
    QTimeEdit *end_task;
    QLineEdit *days_of_week;
    QLabel *label_12;
    QCheckBox *predefined;
    QLabel *label_11;
    QTextEdit *txtComment;
    QComboBox *cmbCronType;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *TaskItem)
    {
        if (TaskItem->objectName().isEmpty())
            TaskItem->setObjectName("TaskItem");
        TaskItem->resize(487, 467);
        gridLayout_2 = new QGridLayout(TaskItem);
        gridLayout_2->setObjectName("gridLayout_2");
        gridLayout = new QGridLayout();
        gridLayout->setSpacing(3);
        gridLayout->setObjectName("gridLayout");
        start_task = new QTimeEdit(TaskItem);
        start_task->setObjectName("start_task");
        start_task->setMaximumSize(QSize(60, 16777215));

        gridLayout->addWidget(start_task, 5, 1, 1, 1);

        allowed = new QCheckBox(TaskItem);
        allowed->setObjectName("allowed");

        gridLayout->addWidget(allowed, 14, 1, 1, 1);

        label_8 = new QLabel(TaskItem);
        label_8->setObjectName("label_8");

        gridLayout->addWidget(label_8, 11, 0, 1, 1);

        label = new QLabel(TaskItem);
        label->setObjectName("label");

        gridLayout->addWidget(label, 0, 0, 1, 1);

        label_3 = new QLabel(TaskItem);
        label_3->setObjectName("label_3");

        gridLayout->addWidget(label_3, 5, 0, 1, 1);

        lblCronString = new QLabel(TaskItem);
        lblCronString->setObjectName("lblCronString");
        lblCronString->setMinimumSize(QSize(0, 24));
        lblCronString->setMaximumSize(QSize(16777215, 24));
        lblCronString->setFrameShape(QFrame::StyledPanel);

        gridLayout->addWidget(lblCronString, 9, 1, 1, 1);

        name = new QLineEdit(TaskItem);
        name->setObjectName("name");

        gridLayout->addWidget(name, 0, 1, 1, 1);

        uuid = new QLineEdit(TaskItem);
        uuid->setObjectName("uuid");
        uuid->setEnabled(false);

        gridLayout->addWidget(uuid, 12, 1, 1, 1);

        label_2 = new QLabel(TaskItem);
        label_2->setObjectName("label_2");

        gridLayout->addWidget(label_2, 1, 0, 1, 1);

        label_7 = new QLabel(TaskItem);
        label_7->setObjectName("label_7");

        gridLayout->addWidget(label_7, 8, 0, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        interval = new QSpinBox(TaskItem);
        interval->setObjectName("interval");
        interval->setMaximumSize(QSize(100, 16777215));
        interval->setMaximum(999999999);

        horizontalLayout->addWidget(interval);

        label_6 = new QLabel(TaskItem);
        label_6->setObjectName("label_6");

        horizontalLayout->addWidget(label_6);


        gridLayout->addLayout(horizontalLayout, 7, 1, 1, 1);

        label_4 = new QLabel(TaskItem);
        label_4->setObjectName("label_4");
        label_4->setWordWrap(true);

        gridLayout->addWidget(label_4, 6, 0, 1, 1);

        cmbScriptType = new QComboBox(TaskItem);
        cmbScriptType->setObjectName("cmbScriptType");

        gridLayout->addWidget(cmbScriptType, 10, 1, 1, 1);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        script = new QLineEdit(TaskItem);
        script->setObjectName("script");

        horizontalLayout_2->addWidget(script);

        btnSelectPlugin = new QToolButton(TaskItem);
        btnSelectPlugin->setObjectName("btnSelectPlugin");
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/img/link.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnSelectPlugin->setIcon(icon);
        btnSelectPlugin->setAutoRaise(true);

        horizontalLayout_2->addWidget(btnSelectPlugin);

        btnPluginParam = new QToolButton(TaskItem);
        btnPluginParam->setObjectName("btnPluginParam");
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/img/options.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnPluginParam->setIcon(icon1);
        btnPluginParam->setAutoRaise(true);

        horizontalLayout_2->addWidget(btnPluginParam);

        btnSave = new QToolButton(TaskItem);
        btnSave->setObjectName("btnSave");
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/img/save.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnSave->setIcon(icon2);
        btnSave->setAutoRaise(true);

        horizontalLayout_2->addWidget(btnSave);


        gridLayout->addLayout(horizontalLayout_2, 11, 1, 1, 1);

        synonum = new QLineEdit(TaskItem);
        synonum->setObjectName("synonum");

        gridLayout->addWidget(synonum, 1, 1, 1, 1);

        label_9 = new QLabel(TaskItem);
        label_9->setObjectName("label_9");

        gridLayout->addWidget(label_9, 12, 0, 1, 1);

        label_5 = new QLabel(TaskItem);
        label_5->setObjectName("label_5");

        gridLayout->addWidget(label_5, 7, 0, 1, 1);

        label_10 = new QLabel(TaskItem);
        label_10->setObjectName("label_10");

        gridLayout->addWidget(label_10, 13, 0, 1, 1);

        label_13 = new QLabel(TaskItem);
        label_13->setObjectName("label_13");

        gridLayout->addWidget(label_13, 2, 0, 1, 1);

        end_task = new QTimeEdit(TaskItem);
        end_task->setObjectName("end_task");
        end_task->setMaximumSize(QSize(60, 16777215));

        gridLayout->addWidget(end_task, 6, 1, 1, 1);

        days_of_week = new QLineEdit(TaskItem);
        days_of_week->setObjectName("days_of_week");

        gridLayout->addWidget(days_of_week, 8, 1, 1, 1);

        label_12 = new QLabel(TaskItem);
        label_12->setObjectName("label_12");

        gridLayout->addWidget(label_12, 9, 0, 1, 1);

        predefined = new QCheckBox(TaskItem);
        predefined->setObjectName("predefined");
        predefined->setEnabled(false);

        gridLayout->addWidget(predefined, 15, 1, 1, 1);

        label_11 = new QLabel(TaskItem);
        label_11->setObjectName("label_11");

        gridLayout->addWidget(label_11, 10, 0, 1, 1);

        txtComment = new QTextEdit(TaskItem);
        txtComment->setObjectName("txtComment");
        txtComment->setMaximumSize(QSize(16777215, 63));
        txtComment->setLineWrapMode(QTextEdit::WidgetWidth);

        gridLayout->addWidget(txtComment, 13, 1, 1, 1);

        cmbCronType = new QComboBox(TaskItem);
        cmbCronType->setObjectName("cmbCronType");

        gridLayout->addWidget(cmbCronType, 2, 1, 1, 1);


        gridLayout_2->addLayout(gridLayout, 0, 0, 1, 1);

        buttonBox = new QDialogButtonBox(TaskItem);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout_2->addWidget(buttonBox, 1, 0, 1, 1);


        retranslateUi(TaskItem);
        QObject::connect(buttonBox, &QDialogButtonBox::accepted, TaskItem, qOverload<>(&QDialog::accept));
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, TaskItem, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(TaskItem);
    } // setupUi

    void retranslateUi(QDialog *TaskItem)
    {
        TaskItem->setWindowTitle(QCoreApplication::translate("TaskItem", "Dialog", nullptr));
        allowed->setText(QCoreApplication::translate("TaskItem", "\320\240\320\260\320\267\321\200\320\265\321\210\320\270\321\202\321\214 \320\262\321\213\320\277\320\276\320\273\320\275\320\265\320\275\320\270\320\265", nullptr));
        label_8->setText(QCoreApplication::translate("TaskItem", "\320\244\320\260\320\271\320\273 \321\201\320\272\321\200\320\270\320\277\321\202\320\260/\320\277\320\273\320\260\320\263\320\270\320\275\320\260:", nullptr));
        label->setText(QCoreApplication::translate("TaskItem", "\320\235\320\260\320\270\320\274\320\265\320\275\320\276\320\262\320\260\320\275\320\270\320\265:", nullptr));
        label_3->setText(QCoreApplication::translate("TaskItem", "\320\235\320\260\321\207\320\270\320\275\320\260\321\202\321\214 \320\262\321\213\320\277\320\276\320\273\320\275\320\265\320\275\320\270\320\265:", nullptr));
        lblCronString->setText(QString());
        label_2->setText(QCoreApplication::translate("TaskItem", "\320\237\321\200\320\265\320\264\321\201\321\202\320\260\320\262\320\273\320\265\320\275\320\270\320\265:", nullptr));
        label_7->setText(QCoreApplication::translate("TaskItem", "\320\222\321\213\320\277\320\276\320\273\320\275\321\217\321\202\321\214 \320\277\320\276 \320\264\320\275\321\217\320\274 \320\275\320\265\320\264\320\265\320\273\320\270:", nullptr));
        label_6->setText(QCoreApplication::translate("TaskItem", "\321\201\320\265\320\272.", nullptr));
        label_4->setText(QCoreApplication::translate("TaskItem", "\320\236\321\201\321\202\320\260\320\275\320\260\320\262\320\273\320\270\320\262\320\260\321\202\321\214 \320\262\321\213\320\277\320\276\320\273\320\275\320\265\320\275\320\270\320\265 \320\277\320\276\321\201\320\273\320\265:", nullptr));
        btnSelectPlugin->setText(QCoreApplication::translate("TaskItem", "...", nullptr));
        btnPluginParam->setText(QCoreApplication::translate("TaskItem", "...", nullptr));
        btnSave->setText(QCoreApplication::translate("TaskItem", "...", nullptr));
        label_9->setText(QCoreApplication::translate("TaskItem", "\320\230\320\264\320\265\320\275\321\202\320\270\321\204\320\270\320\272\320\260\321\202\320\276\321\200:", nullptr));
        label_5->setText(QCoreApplication::translate("TaskItem", "\320\222\321\213\320\277\320\276\320\273\320\275\321\217\321\202\321\214 \321\201 \320\270\320\275\321\202\320\265\321\200\320\262\320\260\320\273\320\276\320\274:", nullptr));
        label_10->setText(QCoreApplication::translate("TaskItem", "\320\232\320\276\320\274\320\274\320\265\320\275\321\202\320\260\321\200\320\270\320\271:", nullptr));
        label_13->setText(QCoreApplication::translate("TaskItem", "\320\242\320\270\320\277 \321\200\320\260\321\201\320\277\320\270\321\201\320\260\320\275\320\270\321\217:", nullptr));
        label_12->setText(QCoreApplication::translate("TaskItem", "Cron:", nullptr));
        predefined->setText(QCoreApplication::translate("TaskItem", "\320\237\321\200\320\265\320\264\320\276\320\277\321\200\320\265\320\264\320\265\320\273\320\265\320\275\320\275\321\213\320\271", nullptr));
        label_11->setText(QCoreApplication::translate("TaskItem", "\320\242\320\270\320\277 \321\201\320\272\321\200\320\270\320\277\321\202\320\260:", nullptr));
    } // retranslateUi

};

namespace Ui {
    class TaskItem: public Ui_TaskItem {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TASKITEM_H
