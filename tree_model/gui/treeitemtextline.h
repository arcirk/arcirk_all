#ifndef TREEITEMTEXTLINE_H
#define TREEITEMTEXTLINE_H

#ifndef IS_OS_ANDROID

#include "treeitemwidget.h"
#include <QToolButton>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QPainter>
#include <QPaintEngine>

namespace arcirk::tree::widgets {

    class TextBox : public QLineEdit
    {
        Q_OBJECT

    private:
        bool m_autoMarkIncomplete = false;
    public:
        TextBox(QWidget *parent = 0): QLineEdit(parent){}
        ~TextBox(){}
        void setAutoMarkIncomplete(bool value){
            m_autoMarkIncomplete = value;
        }

        bool autoMarkIncomplete(){return m_autoMarkIncomplete;}

    protected:
        virtual void paintEvent(QPaintEvent * e){
            if(m_autoMarkIncomplete){
                if(this->text().isEmpty()){
                    QLineEdit::paintEvent(e);
                    QPainter painter(this);
                    painter.setPen(QPen(QBrush(Qt::red), 1));
                    auto rc(rect());
                    rc.setHeight(rc.height() - 2);
                    rc.setWidth(rc.width() - 3);
                    rc.setLeft(rc.left() + 2);
                    painter.drawLine(rc.bottomLeft(), rc.bottomRight());
                }else
                    return QLineEdit::paintEvent(e);
            }else
                return QLineEdit::paintEvent(e);
        }
    };

    class TreeItemTextLine : public TreeItemWidget
    {
        Q_OBJECT
    public:
        explicit TreeItemTextLine(int row, int column, QWidget *parent = nullptr);

        void setText(const QString& text) override;
        QString text() const override;
        bool isChecked() override {return false;};
        void setChecked(bool value) override {Q_UNUSED(value);};
        QVariant currentState() const override{return QVariant();};
        void setCurrentState(const QVariant& state) override {Q_UNUSED(state);};
        void setRole(tree_editor_inneer_role role) override {
            m_role = role;
            if(role == tree_editor_inneer_role::widgetColor ||
                role == tree_editor_inneer_role::widgetDirectoryPath ||
                role == tree_editor_inneer_role::widgetFilePath){
                isSelectButton(true);
            }
        };

        void isSelectButton(bool value);
        void isClearButton(bool value);

        void setDefaultValue(const QString& value);

        void isBorder(bool value);

        void isReadOnly(bool value);

        void setSpacing(int value);

        void setAutoMarkIncomplete(bool value);
        bool autoMarkIncomplete(){return m_text_line->autoMarkIncomplete();}

        void setSynonim(const QString& value);
        void setValue(const QString& value);
        QString synonim() const;
        QString value() const;

        void enableClearBottom(bool value);

    private:
        QToolButton* m_button;
        QToolButton* m_clear_button;
        TextBox* m_text_line;
        QString m_default;
        bool is_select;
        bool is_clear;
        QHBoxLayout* hbox;
        bool is_border;
        bool m_autoMarkIncomplete;
        QString m_synonim;
        QString m_value;
        bool is_synonim;

    private slots:
        void onButtonClicked();
        void onTextChanged(const QVariant& value = "");
    signals:
        void textChanged(const QVariant& value = "");
        void valueChanged(const QVariant& value = "");
    };
} // namespace arcirk::tree::widgets

#endif
#endif // TREEITEMTEXTLINE_H
