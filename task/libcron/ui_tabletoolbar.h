/********************************************************************************
** Form generated from reading UI file 'tabletoolbar.ui'
**
** Created by: Qt User Interface Compiler version 6.5.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TABLETOOLBAR_H
#define UI_TABLETOOLBAR_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TableToolBar
{
public:
    QGridLayout *gridLayout;
    QHBoxLayout *horizontalLayout;
    QToolButton *btnAdd;
    QToolButton *btnEdit;
    QToolButton *btnAddGroup;
    QToolButton *btnDelete;
    QLabel *separator1;
    QToolButton *btnMoveUp;
    QToolButton *btnMoveDown;
    QToolButton *btnMoveTo;
    QLabel *separator2;
    QSpacerItem *horizontalSpacer;

    void setupUi(QWidget *TableToolBar)
    {
        if (TableToolBar->objectName().isEmpty())
            TableToolBar->setObjectName("TableToolBar");
        TableToolBar->resize(500, 25);
        gridLayout = new QGridLayout(TableToolBar);
        gridLayout->setObjectName("gridLayout");
        gridLayout->setHorizontalSpacing(0);
        gridLayout->setVerticalSpacing(3);
        gridLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(3);
        horizontalLayout->setObjectName("horizontalLayout");
        btnAdd = new QToolButton(TableToolBar);
        btnAdd->setObjectName("btnAdd");
        btnAdd->setEnabled(false);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/img/itemAdd.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnAdd->setIcon(icon);
        btnAdd->setAutoRaise(true);

        horizontalLayout->addWidget(btnAdd);

        btnEdit = new QToolButton(TableToolBar);
        btnEdit->setObjectName("btnEdit");
        btnEdit->setEnabled(false);
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/img/itemEdit.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnEdit->setIcon(icon1);
        btnEdit->setAutoRaise(true);

        horizontalLayout->addWidget(btnEdit);

        btnAddGroup = new QToolButton(TableToolBar);
        btnAddGroup->setObjectName("btnAddGroup");
        btnAddGroup->setEnabled(false);
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/img/addGroup.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnAddGroup->setIcon(icon2);
        btnAddGroup->setAutoRaise(true);

        horizontalLayout->addWidget(btnAddGroup);

        btnDelete = new QToolButton(TableToolBar);
        btnDelete->setObjectName("btnDelete");
        btnDelete->setEnabled(false);
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/img/delete.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnDelete->setIcon(icon3);
        btnDelete->setAutoRaise(true);

        horizontalLayout->addWidget(btnDelete);

        separator1 = new QLabel(TableToolBar);
        separator1->setObjectName("separator1");
        separator1->setFrameShape(QFrame::VLine);
        separator1->setFrameShadow(QFrame::Raised);

        horizontalLayout->addWidget(separator1);

        btnMoveUp = new QToolButton(TableToolBar);
        btnMoveUp->setObjectName("btnMoveUp");
        btnMoveUp->setEnabled(false);
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/img/moveup.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnMoveUp->setIcon(icon4);
        btnMoveUp->setAutoRaise(true);

        horizontalLayout->addWidget(btnMoveUp);

        btnMoveDown = new QToolButton(TableToolBar);
        btnMoveDown->setObjectName("btnMoveDown");
        btnMoveDown->setEnabled(false);
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/img/movedown.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnMoveDown->setIcon(icon5);
        btnMoveDown->setAutoRaise(true);

        horizontalLayout->addWidget(btnMoveDown);

        btnMoveTo = new QToolButton(TableToolBar);
        btnMoveTo->setObjectName("btnMoveTo");
        btnMoveTo->setEnabled(false);
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/img/move_to.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnMoveTo->setIcon(icon6);
        btnMoveTo->setAutoRaise(true);

        horizontalLayout->addWidget(btnMoveTo);

        separator2 = new QLabel(TableToolBar);
        separator2->setObjectName("separator2");
        separator2->setFrameShape(QFrame::VLine);
        separator2->setFrameShadow(QFrame::Raised);

        horizontalLayout->addWidget(separator2);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);


        gridLayout->addLayout(horizontalLayout, 0, 0, 1, 1);


        retranslateUi(TableToolBar);

        QMetaObject::connectSlotsByName(TableToolBar);
    } // setupUi

    void retranslateUi(QWidget *TableToolBar)
    {
        TableToolBar->setWindowTitle(QCoreApplication::translate("TableToolBar", "Form", nullptr));
#if QT_CONFIG(tooltip)
        btnAdd->setToolTip(QCoreApplication::translate("TableToolBar", "\320\224\320\276\320\261\320\260\320\262\320\270\321\202\321\214 \321\215\320\273\320\265\320\274\320\265\320\275\321\202", nullptr));
#endif // QT_CONFIG(tooltip)
        btnAdd->setText(QCoreApplication::translate("TableToolBar", "...", nullptr));
#if QT_CONFIG(tooltip)
        btnEdit->setToolTip(QCoreApplication::translate("TableToolBar", "\320\230\320\267\320\274\320\265\320\275\320\270\321\202\321\214 \321\215\320\273\320\265\320\274\320\265\320\275\321\202", nullptr));
#endif // QT_CONFIG(tooltip)
        btnEdit->setText(QCoreApplication::translate("TableToolBar", "...", nullptr));
#if QT_CONFIG(tooltip)
        btnAddGroup->setToolTip(QCoreApplication::translate("TableToolBar", "\320\224\320\276\320\261\320\260\320\262\320\270\321\202\321\214 \320\263\321\200\321\203\320\277\320\277\321\203", nullptr));
#endif // QT_CONFIG(tooltip)
        btnAddGroup->setText(QCoreApplication::translate("TableToolBar", "...", nullptr));
#if QT_CONFIG(tooltip)
        btnDelete->setToolTip(QCoreApplication::translate("TableToolBar", "\320\243\320\264\320\260\320\273\320\270\321\202\321\214", nullptr));
#endif // QT_CONFIG(tooltip)
        btnDelete->setText(QCoreApplication::translate("TableToolBar", "...", nullptr));
        separator1->setText(QString());
#if QT_CONFIG(tooltip)
        btnMoveUp->setToolTip(QCoreApplication::translate("TableToolBar", "\320\237\320\265\321\200\320\265\320\274\320\265\321\201\321\202\320\270\321\202\321\214 \320\262\320\262\320\265\321\200\321\205", nullptr));
#endif // QT_CONFIG(tooltip)
        btnMoveUp->setText(QString());
#if QT_CONFIG(tooltip)
        btnMoveDown->setToolTip(QCoreApplication::translate("TableToolBar", "\320\237\320\265\321\200\320\265\320\274\320\265\321\201\321\202\320\270\321\202\321\214 \320\262\320\275\320\270\320\267", nullptr));
#endif // QT_CONFIG(tooltip)
        btnMoveDown->setText(QCoreApplication::translate("TableToolBar", "...", nullptr));
#if QT_CONFIG(tooltip)
        btnMoveTo->setToolTip(QCoreApplication::translate("TableToolBar", "\320\237\320\265\321\200\320\265\320\274\320\265\321\201\321\202\320\270\321\202\321\214 \320\262 \320\263\321\200\321\203\320\277\320\277\321\203", nullptr));
#endif // QT_CONFIG(tooltip)
        btnMoveTo->setText(QCoreApplication::translate("TableToolBar", "...", nullptr));
        separator2->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class TableToolBar: public Ui_TableToolBar {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TABLETOOLBAR_H
