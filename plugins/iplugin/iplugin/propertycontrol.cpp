#include "propertycontrol.h"
#include "ui_propertycontrol.h"
#include <QLabel>
#include <QTextEdit>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>

PropertyControl::PropertyControl(ControlType t, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PropertyControl)
{
    ui->setupUi(this);

    m_currentWidget = nullptr;
    m_selectButton = nullptr;

    switch (t) {
    case Label:
        createLabel();
        break;
    case TextLine:
        createTextLine();
        break;
    case TextMultiLine:
        createTextBox();
        break;
    case Spin:
        createSpinBox();
        break;
    default:
        break;
    }

    m_type = t;
}

PropertyControl::~PropertyControl()
{
    delete ui;
}

void PropertyControl::setData(const QVariant &value)
{
    m_value = value;
    switch (m_type) {
    case Label:
        if(m_currentWidget){
            auto control = qobject_cast<QLabel*>(m_currentWidget);
            control->setText(value.toString());
        }
        break;
    case TextLine:
        if(m_currentWidget){
            auto control = qobject_cast<QLineEdit*>(m_currentWidget);
            control->setText(value.toString());
        }
    break;
    case TextMultiLine:
        if(m_currentWidget){
            auto control = qobject_cast<QTextEdit*>(m_currentWidget);
            control->setText(value.toString());
        }
        break;
    case Spin:
        if(m_currentWidget){
            auto control = qobject_cast<QSpinBox*>(m_currentWidget);
            control->setValue(value.toInt());
        }
        break;
    case Checked:
        if(m_currentWidget){
            auto control = qobject_cast<QCheckBox*>(m_currentWidget);
            control->setChecked(value.toBool());
        }
        break;
    default:
        break;
    }
}

void PropertyControl::setCaption(const QString &value)
{
    Q_ASSERT(m_currentWidget!=0);
    auto control = qobject_cast<QCheckBox*>(m_currentWidget);
    if(control)
        control->setText(value);

}

void PropertyControl::createLabel()
{
    m_currentWidget = new QLabel(this);
    ui->horizontalLayout->addWidget(m_currentWidget);
}

void PropertyControl::createTextLine()
{
    m_currentWidget = new QLineEdit(this);
    ui->horizontalLayout->addWidget(m_currentWidget);
}

void PropertyControl::createTextBox()
{
    m_currentWidget = new QTextEdit(this);
    ui->horizontalLayout->addWidget(m_currentWidget);
}

void PropertyControl::createSpinBox()
{
    auto control = new QSpinBox(this);
    control->setMaximum(10000);
    control->setAlignment(Qt::AlignRight);
    ui->horizontalLayout->addWidget(control);
    m_currentWidget = control;
}

void PropertyControl::createCheckBox()
{
    m_currentWidget = new QCheckBox(this);
    ui->horizontalLayout->addWidget(m_currentWidget);
}
