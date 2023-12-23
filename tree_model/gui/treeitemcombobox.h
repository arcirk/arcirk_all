#ifndef TREEITEMCOMBOBOX_H
#define TREEITEMCOMBOBOX_H

#ifndef IS_OS_ANDROID

#include "treeitemwidget.h"
#include <QWidget>
#include <QComboBox>
#include "pairmodel.h"

namespace arcirk::tree::widgets {
    class TreeItemComboBox : public TreeItemWidget
    {
        Q_OBJECT
    public:
        explicit TreeItemComboBox(int row, int column, QWidget *parent = nullptr);

        bool isChecked() override;
        void setChecked(bool value) override;
        void setText(const QString& text) override;
        QString text() const override;
        QVariant currentState() const override{return QVariant();};
        void setCurrentState(const QVariant& state) override {Q_UNUSED(state);};
        void setRole(tree_editor_inner_role role) override {m_role = role;};

        void setModel(PairModel* model);
        void addItems(const QStringList& lst);

        void setCurrentIndex(int index);

        QVariant data(int index) const;

    private:
        QComboBox* m_combo;

    private slots:
        void onCurrentIndexChanged(int index);
    signals:
        void currentIndexChanged(int row, int col, int index);
    };
}

#endif
#endif // TREEITEMCOMBOBOX_H
