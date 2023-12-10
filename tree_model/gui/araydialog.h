#ifndef ARAYDIALOG_H
#define ARAYDIALOG_H
#ifndef IS_OS_ANDROID
#include <QDialog>
#include "tree_model.h"
#include "gui/treeviewwidget.h"
#include "gui/tabletoolbar.h"

namespace Ui {
class ArrayDialog;
}

namespace arcirk::tree_widget {
    class ArrayDialog : public QDialog
    {
        Q_OBJECT
    public:
        explicit ArrayDialog(const json& data, QWidget *parent = nullptr);
        ~ArrayDialog();

        json result() const;

        void accept() override;

    private:
        Ui::ArrayDialog *ui;
        json m_data;
        TreeViewWidget* m_tree;
        TableToolBar* m_toolbar;

    private slots:
        void onToolBarItemClicked(const QString& buttonName);
    };
}
#endif
#endif // ARAYDIALOG_H

