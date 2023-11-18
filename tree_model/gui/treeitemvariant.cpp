#ifndef IS_OS_ANDROID
#include "treeitemvariant.h"
//#include "qbitarray.h"
#include <QHBoxLayout>
#include <QToolButton>
#include <QAction>
#include <QMenu>

using namespace arcirk::tree::widgets;
TreeItemVariant::TreeItemVariant(int row, int column, QWidget *parent)
    : TreeItemWidget{row, column, parent}
{

    Q_INIT_RESOURCE(resurce);

    auto hbox = new QHBoxLayout(this);
    hbox->setContentsMargins(0,0,0,0);
    hbox->setSpacing(0);

    m_label = new QLabel(this);
    m_label->setObjectName("libel");
    hbox->addWidget(m_label);
    m_text = new QLineEdit(this);
    m_text->setObjectName("lineEdit");
    hbox->addWidget(m_text);
    m_text->setVisible(false);
    m_integer = new QSpinBox(this);
    m_integer->setObjectName("numberEdit");
    hbox->addWidget(m_integer);
    m_integer->setMaximum(9999);
    m_integer->setAlignment(Qt::AlignRight);
    m_integer->setVisible(false);

    m_sel_type= new QToolButton(this);
    m_sel_type->setObjectName("selectType");
    m_sel_type->setIcon(QIcon(":/img/select_type.png"));
    m_sel_type->setPopupMode(QToolButton::InstantPopup);
    auto mnu = new QMenu(m_sel_type);
    auto action = new QAction("Текст", this);
    action->setObjectName("mnuText");
    connect(action, &QAction::triggered, this, &TreeItemVariant::onMenuItemClicked);
    mnu->addAction(action);
    action = new QAction("Число", this);
    action->setObjectName("mnuInteger");
    connect(action, &QAction::triggered, this, &TreeItemVariant::onMenuItemClicked);
    mnu->addAction(action);
    action = new QAction("Список значений", this);
    action->setObjectName("mnuArray");
    connect(action, &QAction::triggered, this, &TreeItemVariant::onMenuItemClicked);
    mnu->addAction(action);
    action = new QAction("Файл", this);
    action->setObjectName("mnuBinary");
    connect(action, &QAction::triggered, this, &TreeItemVariant::onMenuItemClicked);
    mnu->addAction(action);
    m_sel_type->setMenu(mnu);
    hbox->addWidget(m_sel_type);

    m_selelect = new QToolButton(this);
    m_selelect->setText("...");
    m_selelect->setObjectName("selectButton");
    hbox->addWidget(m_selelect);
    m_selelect->setVisible(false);
    m_erase = new QToolButton(this);
    m_erase->setObjectName("eraseButton");
    m_erase->setIcon(QIcon(":/img/delete.png"));
    hbox->addWidget(m_erase);
    m_erase->setVisible(false);
    setLayout(hbox);

    connect(m_selelect, &QToolButton::clicked, this, &TreeItemVariant::onSelectClicked);
    connect(m_erase, &QToolButton::clicked, this, &TreeItemVariant::onEraseClicked);
    connect(m_integer, &QSpinBox::valueChanged, this, &TreeItemVariant::onSpinChanged);
    connect(m_text, &QLineEdit::textChanged, this, &TreeItemVariant::onTextChanged);

    setCurrentState("null");
}

bool TreeItemVariant::isChecked()
{
    return false;
}

void TreeItemVariant::setChecked(bool value)
{
    Q_UNUSED(value);
}

void TreeItemVariant::setText(const QString &text)
{
    Q_UNUSED(text);
    m_text->setText(text);
}

QString TreeItemVariant::text() const
{
    QString value = "";
    if(m_current_state == "mnuText"){
        value = m_text->text();
    }else if(m_current_state == "mnuArray"){
        value = m_list.join(", ");
    }else if(m_current_state == "mnuBinary"){
        value = "<бинарные данные>";
    }else if(m_current_state == "mnuInteger"){
        value = QString::number(m_integer->value());
    }
    return value;
}

void TreeItemVariant::setCurrentState(const QVariant &state)
{
    //qDebug() << __FUNCTION__;
    m_current_state = state;
    reset_state(m_current_state.toString());
}

void TreeItemVariant::setData(const QVariant &data)
{
    //qDebug() << __FUNCTION__;

    m_current_value = data;

//    if(m_current_state != "null" && m_current_state != "")
//        //m_current_value = data;
//    else
//    if(m_current_state == "mnuText"){
//        m_text->setText(data.toString());
//    }else if(m_current_state == "mnuArray"){
//        m_list = data.toStringList();
//    }else if(m_current_state == "mnuBinary"){
//        m_label->setText("<бинарные данные>");
//    }
//    else if(m_current_state == "mnuInteger"){
//        m_integer->setValue(data.toInt());
//    }
}

QVariant TreeItemVariant::data()
{

//    if(m_current_state == "mnuText"){
//        m_current_value = QVariant(m_text->text());
//    }else if(m_current_state == "mnuArray"){
//        m_current_value = QVariant(m_list);
//    }else if(m_current_state == "mnuBinary"){
//        //value = "<бинарные данные>";
//    }else if(m_current_state == "mnuInteger"){
//        int val = m_integer->value();
//        m_current_value = val;
//    }

    //qDebug() << __FUNCTION__ << m_current_value;
    return m_current_value;
}

void TreeItemVariant::reset_state(const QString &state)
{
    if(state == "mnuText"){
        m_label->setVisible(false);
        m_text->setVisible(true);
        m_sel_type->setVisible(false);
        m_selelect->setVisible(true);
        m_erase->setVisible(true);
        m_integer->setVisible(false);
        //m_text->setText(m_current_value.toString());
    }else if(state == "mnuArray"){
        m_label->setVisible(true);
        m_text->setVisible(false);
        m_sel_type->setVisible(false);
        m_selelect->setVisible(true);
        m_erase->setVisible(true);
        m_integer->setVisible(false);
        m_list = m_current_value.toStringList();
    }else if(state == "mnuBinary"){
        m_label->setVisible(true);
        m_text->setVisible(false);
        m_sel_type->setVisible(false);
        m_selelect->setVisible(true);
        m_erase->setVisible(true);
        m_integer->setVisible(false);
        m_label->setText("<бинарные данные>");
    }else if(state == "mnuInteger"){
        m_label->setVisible(false);
        m_text->setVisible(false);
        m_sel_type->setVisible(false);
        m_selelect->setVisible(false);
        m_erase->setVisible(true);
        m_integer->setVisible(true);
        m_integer->setValue(m_current_value.toInt());
    }else{
        m_label->setVisible(true);
        m_text->setVisible(false);
        m_sel_type->setVisible(true);
        m_selelect->setVisible(false);
        m_erase->setVisible(false);
        m_integer->setVisible(false);
        m_label->setText("");
    }

    m_current_state = state;
    //qDebug() << __FUNCTION__ << state;
}

void TreeItemVariant::onMenuItemClicked()
{
    auto mnu = qobject_cast<QAction*>(sender());
    if(mnu){
        reset_state(mnu->objectName());
    }
}

void TreeItemVariant::onSelectClicked()
{

}

void TreeItemVariant::onEraseClicked()
{
    m_label->setVisible(true);
    m_text->setVisible(false);
    m_sel_type->setVisible(true);
    m_selelect->setVisible(false);
    m_erase->setVisible(false);
    m_integer->setVisible(false);
}

void TreeItemVariant::onSpinChanged(int value)
{
    m_current_value = value;
}

void TreeItemVariant::onTextChanged(const QString &value)
{
    m_current_value = value;
}

#endif
