#ifndef IS_OS_ANDROID
#include "treeitemtextedit.h"
#include "texteditdialog.h"
#include <QHBoxLayout>
#include <QFileDialog>

using namespace arcirk::tree::widgets;

TreeItemTextEdit::TreeItemTextEdit(int row, int column, QWidget *parent) :
    TreeItemWidget(row, column, parent)
{

    is_select = false;
    is_clear = false;
    is_border = false;
    m_autoMarkIncomplete = false;
    is_synonim = false;

    hbox = new QHBoxLayout(this);
    hbox->setContentsMargins(0,0,0,0);
    hbox->setSpacing(0);
    m_text_line = new TextEditBox(this);

    m_button = new QToolButton(this);
    m_button->setText("...");
    m_button->setObjectName("SelectButton");
    m_button->setVisible(is_select);
    m_clear_button = new QToolButton(this);
    m_clear_button->setObjectName("ClearButton");
    m_clear_button->setIcon(QIcon(":/img/clear.ico"));
    m_clear_button->setVisible(is_clear);
    hbox->addWidget(m_text_line);
    hbox->addWidget(m_button, 0, Qt::AlignTop);
    hbox->addWidget(m_clear_button, 0, Qt::AlignTop);

    setLayout(hbox);

    connect(m_button, &QToolButton::clicked, this, &TreeItemTextEdit::onButtonClicked);
    connect(m_clear_button, &QToolButton::clicked, this, &TreeItemTextEdit::onButtonClicked);
    connect(m_text_line, &TextEditBox::textChanged, this, &TreeItemTextEdit::onTextChanged);
}

void TreeItemTextEdit::setSynonim(const QString &value)
{
    m_synonim = value;
    is_synonim = true;
    m_text_line->setText(value);
}

void TreeItemTextEdit::setValue(const QString &value)
{
    m_value = value;
}

QString TreeItemTextEdit::synonim() const
{
    return m_synonim;
}

QString TreeItemTextEdit::value() const
{
    return m_value;
}

void TreeItemTextEdit::enableClearBottom(bool value)
{
    m_clear_button->setEnabled(value);
}

void TreeItemTextEdit::setText(const QString &text)
{
    m_text_line->setText(text);
    m_value = text;
}

QString TreeItemTextEdit::text() const
{
    return m_text_line->toPlainText();
}

void TreeItemTextEdit::isSelectButton(bool value)
{
    is_select = value;
    m_button->setVisible(is_select);
}

void TreeItemTextEdit::isClearButton(bool value)
{
    is_clear = value;
    m_clear_button->setVisible(is_clear);
}

void TreeItemTextEdit::setDefaultValue(const QString &value)
{
    m_default = value;
}

void TreeItemTextEdit::isBorder(bool value)
{
    is_border = value;
    if(!value)
        m_text_line->setStyleSheet("border: 1px  solid rgb(255, 255, 255); border-radius: 0px;");
    else
        m_text_line->setStyleSheet("");
}

void TreeItemTextEdit::isReadOnly(bool value)
{
    m_text_line->setReadOnly(value);
}

void TreeItemTextEdit::setSpacing(int value)
{
    hbox->setSpacing(value);
}

void TreeItemTextEdit::setAutoMarkIncomplete(bool value)
{
    m_autoMarkIncomplete = value;
    if(!value)
        isBorder(is_border);
    else{
        if(m_text_line->toPlainText().isEmpty()){
            m_text_line->setAutoMarkIncomplete(value);
        }else{
           isBorder(is_border);
        }
    }
}

void TreeItemTextEdit::onButtonClicked()
{
    auto btn = qobject_cast<QToolButton*>(sender());

    if(btn){
        if(btn->objectName() == "SelectButton"){
           if(m_role == widgetText){
                auto dlg = TextEditDialog(this);
                dlg.setText(m_text_line->toPlainText());
                if(dlg.exec() == QDialog::Accepted){
                    m_text_line->setText(dlg.text());
                }
           }else if(m_role == widgetFilePath){
                QString filter;
                if(m_ext_value.isValid()){
                    filter = m_ext_value.toString();
                }
                auto result = QFileDialog::getOpenFileName(this, "Выбор файла", m_text_line->toPlainText(), filter);
                if(!result.isEmpty()){
                    m_text_line->setText(result);
                    m_value = result;
                }
           }else if(m_role == widgetDirectoryPath){
                auto result = QFileDialog::getExistingDirectory(this, "Выбор каталога", m_text_line->toPlainText());
                if(!result.isEmpty()){
                    m_text_line->setText(result);
                    m_value = result;
                }
           }else if(m_role == widgetColor){

           }
        }else if(btn->objectName() == "ClearButton"){
            if(!is_synonim)
                m_text_line->setText(m_default);
            else{
                m_value = m_default;
                m_text_line->setText("");
                emit valueChanged(m_value);
            }
        }
    }

}

void TreeItemTextEdit::onTextChanged()
{
    emit textChanged();
}
#endif
