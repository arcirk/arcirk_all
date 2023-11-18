#ifndef IS_OS_ANDROID
#include "treeitemnumber.h"
#include <QHBoxLayout>

using namespace arcirk::tree::widgets;

TreeItemNumber::TreeItemNumber(int row, int column, QWidget *parent) :
    TreeItemWidget(row, column, parent)
{
    auto hbox = new QHBoxLayout(this);
    hbox->setContentsMargins(0,0,0,0);
    hbox->setSpacing(0);
    m_spin = new QSpinBox(this);
    m_spin->setMaximum(99999);
    m_spin->setAlignment(Qt::AlignRight);
    hbox->addWidget(m_spin);

    setLayout(hbox);
}

int TreeItemNumber::value()
{
    return m_spin->value();
}

void TreeItemNumber::setValue(int value)
{
    m_spin->setValue(value);
}
#endif
