/********************************************************************************
** Form generated from reading UI file 'comparewidget.ui'
**
** Created by: Qt User Interface Compiler version 6.5.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_COMPAREWIDGET_H
#define UI_COMPAREWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CompareWidget
{
public:
    QGridLayout *gridLayout_2;
    QVBoxLayout *verticalLayout;
    QWidget *editLayout_2;
    QHBoxLayout *editLayout;
    QTextEdit *textEdit;
    QToolButton *toolButton;
    QWidget *compareCtl;
    QGridLayout *gridLayout;
    QHBoxLayout *horizontalLayout;
    QComboBox *cmbLeftCompare;
    QComboBox *cmbCompare;
    QComboBox *cmbRightCompare;

    void setupUi(QWidget *CompareWidget)
    {
        if (CompareWidget->objectName().isEmpty())
            CompareWidget->setObjectName("CompareWidget");
        CompareWidget->resize(279, 108);
        CompareWidget->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 255);"));
        gridLayout_2 = new QGridLayout(CompareWidget);
        gridLayout_2->setSpacing(0);
        gridLayout_2->setObjectName("gridLayout_2");
        gridLayout_2->setContentsMargins(0, 0, 0, 0);
        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName("verticalLayout");
        editLayout_2 = new QWidget(CompareWidget);
        editLayout_2->setObjectName("editLayout_2");
        editLayout = new QHBoxLayout(editLayout_2);
        editLayout->setSpacing(0);
        editLayout->setObjectName("editLayout");
        editLayout->setContentsMargins(0, 0, 0, 0);
        textEdit = new QTextEdit(editLayout_2);
        textEdit->setObjectName("textEdit");
        textEdit->setFrameShape(QFrame::NoFrame);
        textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        editLayout->addWidget(textEdit);

        toolButton = new QToolButton(editLayout_2);
        toolButton->setObjectName("toolButton");
        toolButton->setStyleSheet(QString::fromUtf8("	border: 1px solid \"gray\";\n"
"	border-radius: 0px;\n"
"color: \"black\";\n"
"background:  \"white\";"));
        toolButton->setPopupMode(QToolButton::DelayedPopup);
        toolButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
        toolButton->setAutoRaise(false);

        editLayout->addWidget(toolButton);


        verticalLayout->addWidget(editLayout_2);

        compareCtl = new QWidget(CompareWidget);
        compareCtl->setObjectName("compareCtl");
        gridLayout = new QGridLayout(compareCtl);
        gridLayout->setSpacing(0);
        gridLayout->setObjectName("gridLayout");
        gridLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName("horizontalLayout");
        cmbLeftCompare = new QComboBox(compareCtl);
        cmbLeftCompare->setObjectName("cmbLeftCompare");
        cmbLeftCompare->setAutoFillBackground(false);
        cmbLeftCompare->setStyleSheet(QString::fromUtf8("QComboBox {\n"
"	background-color: rgb(255, 255, 255);\n"
"    color: \"black\";\n"
"    border: 0px;\n"
"}"));
        cmbLeftCompare->setFrame(true);

        horizontalLayout->addWidget(cmbLeftCompare);

        cmbCompare = new QComboBox(compareCtl);
        cmbCompare->setObjectName("cmbCompare");
        cmbCompare->setMaximumSize(QSize(50, 16777215));
        cmbCompare->setStyleSheet(QString::fromUtf8("QComboBox {\n"
"	background-color: rgb(255, 255, 255);\n"
"    color: \"black\";\n"
"    border: 0px;\n"
"}"));
        cmbCompare->setFrame(true);

        horizontalLayout->addWidget(cmbCompare);

        cmbRightCompare = new QComboBox(compareCtl);
        cmbRightCompare->setObjectName("cmbRightCompare");
        cmbRightCompare->setStyleSheet(QString::fromUtf8("QComboBox {\n"
"	background-color: rgb(255, 255, 255);\n"
"    color: \"black\";\n"
"    border: 0px;\n"
"}"));
        cmbRightCompare->setFrame(true);

        horizontalLayout->addWidget(cmbRightCompare);


        gridLayout->addLayout(horizontalLayout, 0, 0, 1, 1);


        verticalLayout->addWidget(compareCtl);


        gridLayout_2->addLayout(verticalLayout, 0, 0, 1, 1);


        retranslateUi(CompareWidget);

        QMetaObject::connectSlotsByName(CompareWidget);
    } // setupUi

    void retranslateUi(QWidget *CompareWidget)
    {
        CompareWidget->setWindowTitle(QCoreApplication::translate("CompareWidget", "Form", nullptr));
        toolButton->setText(QCoreApplication::translate("CompareWidget", "...", nullptr));
    } // retranslateUi

};

namespace Ui {
    class CompareWidget: public Ui_CompareWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_COMPAREWIDGET_H
