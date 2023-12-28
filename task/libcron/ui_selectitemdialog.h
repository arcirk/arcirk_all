/********************************************************************************
** Form generated from reading UI file 'selectitemdialog.ui'
**
** Created by: Qt User Interface Compiler version 6.5.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SELECTITEMDIALOG_H
#define UI_SELECTITEMDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_SelectItemDialog
{
public:
    QGridLayout *gridLayout;
    QDialogButtonBox *buttonBox;
    QVBoxLayout *verticalLayout;

    void setupUi(QDialog *SelectItemDialog)
    {
        if (SelectItemDialog->objectName().isEmpty())
            SelectItemDialog->setObjectName("SelectItemDialog");
        SelectItemDialog->resize(400, 300);
        gridLayout = new QGridLayout(SelectItemDialog);
        gridLayout->setObjectName("gridLayout");
        buttonBox = new QDialogButtonBox(SelectItemDialog);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout->addWidget(buttonBox, 1, 0, 1, 1);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName("verticalLayout");

        gridLayout->addLayout(verticalLayout, 0, 0, 1, 1);


        retranslateUi(SelectItemDialog);
        QObject::connect(buttonBox, &QDialogButtonBox::accepted, SelectItemDialog, qOverload<>(&QDialog::accept));
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, SelectItemDialog, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(SelectItemDialog);
    } // setupUi

    void retranslateUi(QDialog *SelectItemDialog)
    {
        SelectItemDialog->setWindowTitle(QCoreApplication::translate("SelectItemDialog", "Dialog", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SelectItemDialog: public Ui_SelectItemDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SELECTITEMDIALOG_H
