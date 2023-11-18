#ifndef TREEITEMVARIANT_H
#define TREEITEMVARIANT_H

#ifndef IS_OS_ANDROID
#include "treeitemwidget.h"
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>

namespace arcirk::tree::widgets {
    class TreeItemVariant : public TreeItemWidget
    {
        Q_OBJECT
    public:
        explicit TreeItemVariant(int row, int column, QWidget *parent = nullptr);
        ~TreeItemVariant(){qDebug() << __FUNCTION__;};

        bool isChecked() override;
        void setChecked(bool value) override;
        void setText(const QString& text) override;
        QString text() const override;
        QVariant currentState() const override{return m_current_state;};
        void setCurrentState(const QVariant& state) override;

        void setData(const QVariant& data);
        QVariant data();

    private:
        QLabel* m_label;
        QLineEdit* m_text;
        QToolButton * m_sel_type;
        QToolButton * m_selelect;
        QToolButton * m_erase;
        QSpinBox * m_integer;
        QVariant m_current_state;
        QVariant m_current_value;
        QStringList m_list;


        void reset_state(const QString& state);

    private slots:
        void onMenuItemClicked();
        void onSelectClicked();
        void onEraseClicked();
        void onSpinChanged(int value);
        void onTextChanged(const QString& value);
    };
}

#endif
#endif // TREEITEMVARIANT_H
