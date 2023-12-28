/********************************************************************************
** Form generated from reading UI file 'rowdialog.ui'
**
** Created by: Qt User Interface Compiler version 6.5.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ROWDIALOG_H
#define UI_ROWDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>

QT_BEGIN_NAMESPACE

class Ui_RowDialog
{
public:
    QGridLayout *gridLayout_2;
    QGridLayout *gridLayout;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *RowDialog)
    {
        if (RowDialog->objectName().isEmpty())
            RowDialog->setObjectName("RowDialog");
        RowDialog->resize(416, 72);
        gridLayout_2 = new QGridLayout(RowDialog);
        gridLayout_2->setObjectName("gridLayout_2");
        gridLayout = new QGridLayout();
        gridLayout->setObjectName("gridLayout");

        gridLayout_2->addLayout(gridLayout, 0, 0, 1, 1);

        buttonBox = new QDialogButtonBox(RowDialog);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setOrientation(Qt::Vertical);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout_2->addWidget(buttonBox, 0, 1, 1, 1);


        retranslateUi(RowDialog);
        QObject::connect(buttonBox, &QDialogButtonBox::accepted, RowDialog, qOverload<>(&QDialog::accept));
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, RowDialog, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(RowDialog);
    } // setupUi

    void retranslateUi(QDialog *RowDialog)
    {
        RowDialog->setWindowTitle(QCoreApplication::translate("RowDialog", "Dialog", nullptr));
    } // retranslateUi

};

namespace Ui {
    class RowDialog: public Ui_RowDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ROWDIALOG_H
