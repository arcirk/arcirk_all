/********************************************************************************
** Form generated from reading UI file 'treeitemwidget.ui'
**
** Created by: Qt User Interface Compiler version 6.5.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TREEITEMWIDGET_H
#define UI_TREEITEMWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TreeItemWidget
{
public:

    void setupUi(QWidget *TreeItemWidget)
    {
        if (TreeItemWidget->objectName().isEmpty())
            TreeItemWidget->setObjectName("TreeItemWidget");
        TreeItemWidget->resize(16, 16);

        retranslateUi(TreeItemWidget);

        QMetaObject::connectSlotsByName(TreeItemWidget);
    } // setupUi

    void retranslateUi(QWidget *TreeItemWidget)
    {
        TreeItemWidget->setWindowTitle(QCoreApplication::translate("TreeItemWidget", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class TreeItemWidget: public Ui_TreeItemWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TREEITEMWIDGET_H
