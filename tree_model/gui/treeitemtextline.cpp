#ifndef IS_OS_ANDROID
#include "treeitemtextline.h"
#include "texteditdialog.h"
#include <QHBoxLayout>


using namespace arcirk::tree::widgets;

TreeItemTextLine::TreeItemTextLine(int row, int column, QWidget *parent) :
    TreeItemWidget(row, column, parent)
{

    is_select = false;
    is_clear = false;
    is_border = false;
    m_autoMarkIncomplete = false;

    hbox = new QHBoxLayout(this);
    hbox->setContentsMargins(0,0,0,0);
    hbox->setSpacing(0);
    m_text_line = new TextBox(this);
    //m_text_line->setStyleSheet("border: 1px  solid rgb(255, 255, 255); border-radius: 0px;");
    m_button = new QToolButton(this);
    m_button->setText("...");
    m_button->setObjectName("SelectButton");
    m_button->setVisible(is_select);
    m_clear_button = new QToolButton(this);
    m_clear_button->setObjectName("ClearButton");
    m_clear_button->setIcon(QIcon(":/img/clear.ico"));
    m_clear_button->setVisible(is_clear);
    hbox->addWidget(m_text_line);
    hbox->addWidget(m_button);
    hbox->addWidget(m_clear_button);

    setLayout(hbox);

    connect(m_button, &QToolButton::clicked, this, &TreeItemTextLine::onButtonClicked);
    connect(m_clear_button, &QToolButton::clicked, this, &TreeItemTextLine::onButtonClicked);
    connect(m_text_line, &TextBox::textChanged, this, &TreeItemTextLine::onTextChanged);
}

void TreeItemTextLine::setText(const QString &text)
{
    m_text_line->setText(text);
}

QString TreeItemTextLine::text() const
{
    return m_text_line->text();
}

void TreeItemTextLine::isSelectButton(bool value)
{
    is_select = value;
    m_button->setVisible(is_select);
}

void TreeItemTextLine::isClearButton(bool value)
{
    is_clear = value;
    m_clear_button->setVisible(is_clear);
}

void TreeItemTextLine::setDefaultValue(const QString &value)
{
    m_default = value;
}

void TreeItemTextLine::isBorder(bool value)
{
    is_border = value;
    if(!value)
        m_text_line->setStyleSheet("border: 1px  solid rgb(255, 255, 255); border-radius: 0px;");
    else
        m_text_line->setStyleSheet("");
}

void TreeItemTextLine::isReadOnly(bool value)
{
    m_text_line->setReadOnly(value);
}

void TreeItemTextLine::setSpacing(int value)
{
    hbox->setSpacing(value);
}

void TreeItemTextLine::setAutoMarkIncomplete(bool value)
{
    m_autoMarkIncomplete = value;
    if(!value)
        isBorder(is_border);
    else{
        if(m_text_line->text().isEmpty()){
//           m_text_line->setStyleSheet("border-top: 1px;"
//                "border-bottom: 1px solid red");
            m_text_line->setAutoMarkIncomplete(value);
        }else{
           isBorder(is_border);
        }
    }
}

void TreeItemTextLine::onButtonClicked()
{
    auto btn = qobject_cast<QToolButton*>(sender());

    if(btn){
        if(btn->objectName() == "SelectButton"){
            auto dlg = TextEditDialog(this);
            dlg.setText(m_text_line->text());
            if(dlg.exec() == QDialog::Accepted){
                m_text_line->setText(dlg.text());
            }
        }else if(btn->objectName() == "ClearButton"){
            m_text_line->setText(m_default);
        }
    }

}

void TreeItemTextLine::onTextChanged(const QVariant &value)
{
//    if(m_autoMarkIncomplete){
//        if(m_text_line->text().isEmpty()){
////            m_text_line->setStyleSheet("");
////            m_text_line->setStyleSheet("border: 1px solid blue; border-radius: 2px;"
////                                       "border-bottom: 1px solid red");
////            QPainter* painter = m_text_line->paintEngine()->painter();
////            //painter.drawText(QPoint(rect().center().x() - 10, rect().center().y()), "Label");

////            painter->setPen(QPen(QBrush(Qt::red), 5));
////            painter->drawLine(rect().topLeft(), rect().bottomRight());

//        }else{
//            isBorder(is_border);
//        }
//    }

    emit textChanged(value);
}
#endif
