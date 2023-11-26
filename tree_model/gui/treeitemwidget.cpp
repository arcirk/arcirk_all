#ifndef IS_OS_ANDROID
#include "treeitemwidget.h"
#include "ui_treeitemwidget.h"

using namespace arcirk::tree::widgets;

TreeItemWidget::TreeItemWidget(int row, int column, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TreeItemWidget)
{
    ui->setupUi(this);

    m_row = row;
    m_column = column;
    m_role = widgetText;

    setContentsMargins(0,0,0,0);
}

TreeItemWidget::~TreeItemWidget()
{
    delete ui;
}

int TreeItemWidget::row() const
{
    return m_row;
}

int TreeItemWidget::column() const
{
    return m_column;
}

#endif
