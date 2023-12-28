/********************************************************************************
** Form generated from reading UI file 'araydialog.ui'
**
** Created by: Qt User Interface Compiler version 6.5.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ARAYDIALOG_H
#define UI_ARAYDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_ArrayDialog
{
public:
    QGridLayout *gridLayout;
    QDialogButtonBox *buttonBox;
    QVBoxLayout *verticalLayout;

    void setupUi(QDialog *ArrayDialog)
    {
        if (ArrayDialog->objectName().isEmpty())
            ArrayDialog->setObjectName("ArrayDialog");
        ArrayDialog->resize(330, 308);
        gridLayout = new QGridLayout(ArrayDialog);
        gridLayout->setObjectName("gridLayout");
        buttonBox = new QDialogButtonBox(ArrayDialog);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setOrientation(Qt::Vertical);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout->addWidget(buttonBox, 0, 1, 1, 1);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName("verticalLayout");

        gridLayout->addLayout(verticalLayout, 0, 0, 1, 1);


        retranslateUi(ArrayDialog);
        QObject::connect(buttonBox, &QDialogButtonBox::accepted, ArrayDialog, qOverload<>(&QDialog::accept));
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, ArrayDialog, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(ArrayDialog);
    } // setupUi

    void retranslateUi(QDialog *ArrayDialog)
    {
        ArrayDialog->setWindowTitle(QCoreApplication::translate("ArrayDialog", "\320\241\320\277\320\270\321\201\320\276\320\272", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ArrayDialog: public Ui_ArrayDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ARAYDIALOG_H
