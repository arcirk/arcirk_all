#ifndef IS_OS_ANDROID
#include "treeitemcombobox.h"
#include <QHBoxLayout>

using namespace arcirk::tree::widgets;

TreeItemComboBox::TreeItemComboBox(int row, int column, QWidget *parent)
    : TreeItemWidget{row, column, parent}
{
    auto hbox = new QHBoxLayout(this);
    m_combo = new QComboBox(this);
    hbox->setContentsMargins(0,0,0,0);
    hbox->setSpacing(0);
    hbox->addWidget(m_combo);
    //hbox->setAlignment(m_combo, Qt::AlignHCenter | Qt::AlignVCenter);

    this->setContentsMargins(0,0,0,0);
    setLayout(hbox);

    connect(m_combo, &QComboBox::currentIndexChanged, this, &TreeItemComboBox::onCurrentIndexChanged);

}

bool TreeItemComboBox::isChecked()
{
    return false;
}

void TreeItemComboBox::setChecked(bool value)
{
    Q_UNUSED(value);
}

void TreeItemComboBox::setText(const QString &text)
{
    Q_UNUSED(text);
}

QString TreeItemComboBox::text() const
{
    return m_combo->currentText();
}

void TreeItemComboBox::setModel(PairModel *model)
{
    m_combo->setModel(model);
}

void TreeItemComboBox::addItems(const QStringList &lst)
{
    m_combo->addItems(lst);
}

void TreeItemComboBox::setCurrentIndex(int index)
{
    m_combo->setCurrentIndex(index);
}

QVariant TreeItemComboBox::data(int index) const
{
    return m_combo->currentData();
}

void TreeItemComboBox::onCurrentIndexChanged(int index)
{
    emit currentIndexChanged(row(), column(), index);
}
#endif
