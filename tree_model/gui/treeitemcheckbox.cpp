#ifndef IS_OS_ANDROID
#include "treeitemcheckbox.h"
#include <QHBoxLayout>

using namespace arcirk::tree::widgets;

TreeItemCheckBox::TreeItemCheckBox(int row, int column, QWidget *parent)
    : TreeItemWidget{row, column, parent}
{
    auto hbox = new QHBoxLayout(this);
    hbox->setContentsMargins(0,0,0,0);
    hbox->setSpacing(0);
    m_check = new QCheckBox(this);
    hbox->addWidget(m_check);
    hbox->setAlignment(m_check, Qt::AlignHCenter | Qt::AlignVCenter);

    this->setContentsMargins(0,0,0,0);
    setLayout(hbox);

    connect(m_check, &QCheckBox::clicked, this, &TreeItemCheckBox::onCheckBoxClicked);

}

bool TreeItemCheckBox::isChecked()
{
    return m_check->isChecked();
}

void TreeItemCheckBox::setChecked(bool value)
{
    m_check->setChecked(value);
}

void TreeItemCheckBox::setText(const QString &text)
{
    m_check->setText(text);
}

QString TreeItemCheckBox::text() const
{
    return m_check->text();
}

void TreeItemCheckBox::onCheckBoxClicked(bool state)
{
    emit checkBoxClicked(row(), column(), state);
}
#endif
