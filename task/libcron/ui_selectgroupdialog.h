/********************************************************************************
** Form generated from reading UI file 'selectgroupdialog.ui'
**
** Created by: Qt User Interface Compiler version 6.5.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SELECTGROUPDIALOG_H
#define UI_SELECTGROUPDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_SelectGroupDialog
{
public:
    QGridLayout *gridLayout;
    QVBoxLayout *verticalLayout;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *SelectGroupDialog)
    {
        if (SelectGroupDialog->objectName().isEmpty())
            SelectGroupDialog->setObjectName("SelectGroupDialog");
        SelectGroupDialog->resize(400, 300);
        gridLayout = new QGridLayout(SelectGroupDialog);
        gridLayout->setObjectName("gridLayout");
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName("verticalLayout");

        gridLayout->addLayout(verticalLayout, 0, 0, 1, 1);

        buttonBox = new QDialogButtonBox(SelectGroupDialog);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout->addWidget(buttonBox, 1, 0, 1, 1);


        retranslateUi(SelectGroupDialog);
        QObject::connect(buttonBox, &QDialogButtonBox::accepted, SelectGroupDialog, qOverload<>(&QDialog::accept));
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, SelectGroupDialog, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(SelectGroupDialog);
    } // setupUi

    void retranslateUi(QDialog *SelectGroupDialog)
    {
        SelectGroupDialog->setWindowTitle(QCoreApplication::translate("SelectGroupDialog", "\320\222\321\213\320\261\320\276\321\200 \320\263\321\200\321\203\320\277\320\277\321\213", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SelectGroupDialog: public Ui_SelectGroupDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SELECTGROUPDIALOG_H
