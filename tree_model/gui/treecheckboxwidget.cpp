#include "treecheckboxwidget.h"
#include "ui_treecheckboxwidget.h"
#include <QHBoxLayout>

using namespace arcirk::tree_model;

TreeCheckBoxWidget::TreeCheckBoxWidget(Qt::Alignment align, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TreeCheckBoxWidget)
{
    ui->setupUi(this);
    auto hbox = new QHBoxLayout(this);
    hbox->setContentsMargins(0,0,0,0);
    hbox->setSpacing(0);
    m_check_box = new QCheckBox(this);
    //m_check_box->setStyleSheet("margin-left:50%; margin-right:50%;");
    hbox->addWidget(m_check_box);
    hbox->setAlignment(m_check_box, align);

    setLayout(hbox);

    current_index = qMakePair(0,0);

    connect(m_check_box, &QCheckBox::stateChanged, this, &TreeCheckBoxWidget::onCheckBoxStateChanged);
}

TreeCheckBoxWidget::~TreeCheckBoxWidget()
{
    delete ui;
}

void TreeCheckBoxWidget::setChecked(bool value)
{
    m_check_box->setChecked(value);
}

bool TreeCheckBoxWidget::isChecked() const
{
    return m_check_box->isChecked();
}

void TreeCheckBoxWidget::setCurrentIndex(int row, int col)
{
    current_index = qMakePair(row, col);
}

void TreeCheckBoxWidget::onCheckBoxStateChanged(int arg1)
{
    Q_UNUSED(arg1);
    int row = current_index.first;
    int col = current_index.second;
    emit stateChanged(row, col, m_check_box->isChecked());
}

