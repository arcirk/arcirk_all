#ifndef TREEITEMCHECKBOX_H
#define TREEITEMCHECKBOX_H

#ifndef IS_OS_ANDROID

#include <QObject>
#include <QWidget>
#include "treeitemwidget.h"
#include <QCheckBox>

namespace arcirk::tree::widgets {
    class TreeItemCheckBox : public TreeItemWidget
    {
        Q_OBJECT
    public:
        explicit TreeItemCheckBox(int row, int column, QWidget *parent = nullptr);

        bool isChecked() override;
        void setChecked(bool value) override;
        void setText(const QString& text) override;
        QString text() const override;
        QVariant currentState() const override{return QVariant();};
        void setCurrentState(const QVariant& state) override {Q_UNUSED(state);};
        void setRole(tree_editor_inneer_role role) override {m_role = role;};

    private:
        QCheckBox* m_check;

    private slots:
        void onCheckBoxClicked(bool state);

    signals:
        void checkBoxClicked(int row, int column, bool state);
    };
}

#endif
#endif // TREEITEMCHECKBOX_H
