/********************************************************************************
** Form generated from reading UI file 'propertydialog.ui'
**
** Created by: Qt User Interface Compiler version 6.5.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PROPERTYDIALOG_H
#define UI_PROPERTYDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_PropertyDialog
{
public:
    QGridLayout *gridLayout;
    QDialogButtonBox *buttonBox;
    QVBoxLayout *verticalLayout;

    void setupUi(QDialog *PropertyDialog)
    {
        if (PropertyDialog->objectName().isEmpty())
            PropertyDialog->setObjectName("PropertyDialog");
        PropertyDialog->resize(391, 235);
        gridLayout = new QGridLayout(PropertyDialog);
        gridLayout->setObjectName("gridLayout");
        buttonBox = new QDialogButtonBox(PropertyDialog);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout->addWidget(buttonBox, 1, 0, 1, 1);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName("verticalLayout");

        gridLayout->addLayout(verticalLayout, 0, 0, 1, 1);


        retranslateUi(PropertyDialog);
        QObject::connect(buttonBox, &QDialogButtonBox::accepted, PropertyDialog, qOverload<>(&QDialog::accept));
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, PropertyDialog, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(PropertyDialog);
    } // setupUi

    void retranslateUi(QDialog *PropertyDialog)
    {
        PropertyDialog->setWindowTitle(QCoreApplication::translate("PropertyDialog", "Dialog", nullptr));
    } // retranslateUi

};

namespace Ui {
    class PropertyDialog: public Ui_PropertyDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PROPERTYDIALOG_H
