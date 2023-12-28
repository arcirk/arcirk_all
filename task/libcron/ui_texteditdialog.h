/********************************************************************************
** Form generated from reading UI file 'texteditdialog.ui'
**
** Created by: Qt User Interface Compiler version 6.5.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TEXTEDITDIALOG_H
#define UI_TEXTEDITDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QPlainTextEdit>

QT_BEGIN_NAMESPACE

class Ui_TextEditDialog
{
public:
    QGridLayout *gridLayout;
    QPlainTextEdit *plainTextEdit;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *TextEditDialog)
    {
        if (TextEditDialog->objectName().isEmpty())
            TextEditDialog->setObjectName("TextEditDialog");
        TextEditDialog->resize(317, 228);
        gridLayout = new QGridLayout(TextEditDialog);
        gridLayout->setObjectName("gridLayout");
        plainTextEdit = new QPlainTextEdit(TextEditDialog);
        plainTextEdit->setObjectName("plainTextEdit");

        gridLayout->addWidget(plainTextEdit, 0, 0, 1, 1);

        buttonBox = new QDialogButtonBox(TextEditDialog);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout->addWidget(buttonBox, 1, 0, 1, 1);


        retranslateUi(TextEditDialog);
        QObject::connect(buttonBox, &QDialogButtonBox::accepted, TextEditDialog, qOverload<>(&QDialog::accept));
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, TextEditDialog, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(TextEditDialog);
    } // setupUi

    void retranslateUi(QDialog *TextEditDialog)
    {
        TextEditDialog->setWindowTitle(QCoreApplication::translate("TextEditDialog", "\320\240\320\265\320\264\320\260\320\272\321\202\320\270\321\200\320\276\320\262\320\260\320\275\320\270\320\265 \321\202\320\265\320\272\321\201\321\202\320\260", nullptr));
    } // retranslateUi

};

namespace Ui {
    class TextEditDialog: public Ui_TextEditDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TEXTEDITDIALOG_H
