#include "treecomboboxwidget.h"
#include "ui_treecomboboxwidget.h"

using namespace arcirk::tree_model;

TreeComboBoxWidget::TreeComboBoxWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TreeComboBoxWidget)
{
    ui->setupUi(this);
    connect(ui->comboBox, &QComboBox::currentIndexChanged, this, &TreeComboBoxWidget::onCurrentIndexChanged);
}

TreeComboBoxWidget::~TreeComboBoxWidget()
{
    delete ui;
}

void TreeComboBoxWidget::addItems(const QStringList& list)
{
    ui->comboBox->addItems(list);
}

void TreeComboBoxWidget::setCurrentIndex(int index)
{
    ui->comboBox->setCurrentIndex(index);
}

QString TreeComboBoxWidget::currentText() const
{
    return ui->comboBox->currentText();
}

void TreeComboBoxWidget::onCurrentIndexChanged(int index)
{
    emit currentIndexChanged(index);
}
