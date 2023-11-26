#ifndef TREEITEMTEXTEDIT_H
#define TREEITEMTEXTEDIT_H

#include "treeitemwidget.h"
#include <QToolButton>
#include <QTextEdit>
#include <QHBoxLayout>
#include <QPainter>
#include <QPaintEngine>
namespace arcirk::tree::widgets {
    class TextEditBox : public QTextEdit
    {
        Q_OBJECT

        private:
            bool m_autoMarkIncomplete = false;
        public:
            TextEditBox(QWidget *parent = 0): QTextEdit(parent){}
            ~TextEditBox(){}
            void setAutoMarkIncomplete(bool value){
                m_autoMarkIncomplete = value;
            }

            bool autoMarkIncomplete(){return m_autoMarkIncomplete;}

        protected:
            virtual void paintEvent(QPaintEvent * e){
                if(m_autoMarkIncomplete){
                    if(this->toPlainText().isEmpty()){
                        QTextEdit::paintEvent(e);
                        QPainter painter(this);
                        painter.setPen(QPen(QBrush(Qt::red), 1));
                        auto rc(rect());
                        rc.setHeight(rc.height() - 2);
                        rc.setWidth(rc.width() - 3);
                        rc.setLeft(rc.left() + 2);
                        painter.drawLine(rc.bottomLeft(), rc.bottomRight());
                    }else
                        return QTextEdit::paintEvent(e);
                }else
                    return QTextEdit::paintEvent(e);
            }
    };

    class TreeItemTextEdit : public TreeItemWidget
    {
            Q_OBJECT
        public:
            explicit TreeItemTextEdit(int row, int column, QWidget *parent = nullptr);

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
            TextEditBox* m_text_line;
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
            void onTextChanged();
        signals:
            void textChanged();
            void valueChanged(const QVariant& value = "");
        };
}
#endif // TREEITEMTEXTEDIT_H
