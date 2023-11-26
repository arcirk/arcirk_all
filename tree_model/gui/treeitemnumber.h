#ifndef TREEITEMNUMBER_H
#define TREEITEMNUMBER_H

#ifndef IS_OS_ANDROID

#include "treeitemwidget.h"
#include <QWidget>
#include <QSpinBox>

namespace arcirk::tree::widgets {
    class TreeItemNumber : public TreeItemWidget
    {
        Q_OBJECT
    public:
        explicit TreeItemNumber(int row, int column, QWidget *parent = nullptr);

        void setText(const QString& text) override {Q_UNUSED(text);};
        QString text() const override {return "";};
        bool isChecked() override {return false;};
        void setChecked(bool value) override {Q_UNUSED(value);};
        QVariant currentState() const override{return QVariant();};
        void setCurrentState(const QVariant& state) override {Q_UNUSED(state);};
        void setRole(tree_editor_inneer_role role) override {m_role = role;};

        int value();
        void setValue(int value);

    private:
        QSpinBox * m_spin;

    };
}

#endif
#endif // TREEITEMNUMBER_H
