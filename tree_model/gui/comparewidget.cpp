#ifndef IS_OS_ANDROID
#include "comparewidget.h"
#include "ui_comparewidget.h"
#include <QToolButton>
#include <QComboBox>

using namespace arcirk::tree_model;

CompareWidget::CompareWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CompareWidget)
{
    ui->setupUi(this);
    formControl(0,0);

    connect(ui->toolButton, &QToolButton::clicked, this, &CompareWidget::onOpenArbitraryExpressionDialog);
}

CompareWidget::CompareWidget(int row, int col, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CompareWidget)
{
    ui->setupUi(this);
    formControl(row,col);
}

CompareWidget::~CompareWidget()
{
    delete ui;
}

void CompareWidget::setActivePage(int index)
{
    if(index==0){
        parse();
    }else{
        update_data();
    }

    ui->compareCtl->setVisible(index == 0);
    ui->editLayout_2->setVisible(index != 0);

}

void CompareWidget::setModelIndex(int row, int col)
{
    currentIndex = qMakePair(row,col);
}

void CompareWidget::setLeftData(const QStringList &value, const QString& table_bame)
{
    auto lst = ui->cmbLeftCompare;
    lst->clear();
    lst->addItems(value);
    leftTable = table_bame;
    ui->cmbLeftCompare->setProperty("table_name", table_bame);
}

void CompareWidget::setRightData(const QStringList &value, const QString& table_bame)
{
    auto lst = ui->cmbRightCompare;
    lst->clear();
    lst->addItems(value);
    rightTable = table_bame;
    ui->cmbRightCompare->setProperty("table_name", table_bame);
}

QString CompareWidget::text() const
{
    return ui->textEdit->toPlainText();
}

void CompareWidget::setText(const QString &text)
{
    ui->textEdit->setText(text);
    parse();
}

void CompareWidget::setTableModel(const json &model)
{
    table_model = model;
}

void CompareWidget::set_rcmb_read_only(bool value)
{
    ui->cmbRightCompare->setEditable(value);
}

void CompareWidget::onCmbLeftCompareCurrentIndexChanged(int index)
{
    emit selectionItemLeftChanged(currentIndex.first, currentIndex.second, ui->cmbLeftCompare->currentText(), index);
    update_data();
}

void CompareWidget::onCmbRightCompareCurrentIndexChanged(int index)
{
    emit selectionItemLeftChanged(currentIndex.first, currentIndex.second, ui->cmbRightCompare->currentText(), index);
    update_data();
}

void CompareWidget::formControl(int row, int col)
{
    ui->compareCtl->setVisible(false);
    currentIndex = qMakePair(row,col);

    ui->cmbCompare->addItems(QStringList{"=", "<>", "<", ">", "<=", ">="});
    ui->cmbCompare->setCurrentText("=");
    connect(ui->cmbLeftCompare, &QComboBox::currentIndexChanged, this, &CompareWidget::onCmbLeftCompareCurrentIndexChanged);
    connect(ui->cmbLeftCompare, &QComboBox::currentTextChanged, this, &CompareWidget::onCuurrentTextChanged);
    connect(ui->cmbRightCompare, &QComboBox::currentIndexChanged, this, &CompareWidget::onCmbRightCompareCurrentIndexChanged);
    connect(ui->cmbRightCompare, &QComboBox::currentTextChanged, this, &CompareWidget::onCuurrentTextChanged);
}

void CompareWidget::update_data()
{
    QString left = ui->cmbLeftCompare->currentText();
    QString right = ui->cmbRightCompare->currentText();
    QString compare = ui->cmbCompare->currentText();
    ui->textEdit->setText(leftTable + "." + left + " " + compare + " " + rightTable + "." + right);
}

void CompareWidget::parse()
{

    QString text = ui->textEdit->toPlainText();

    //int ind_compare = -1;
    QString compare;
    for (int i = 0; i < ui->cmbCompare->count(); ++i) {
        if(text.indexOf(ui->cmbCompare->itemText(i)) != -1){
            //ind_compare = text.indexOf(ui->cmbCompare->itemText(i));
            compare = ui->cmbCompare->itemText(i);
            ui->cmbCompare->setCurrentIndex(i);
            break;
        }
    }

    QStringList l_text = text.split(compare);
    if(l_text.size() != 2)
        return;
    QStringList left_full = l_text[0].trimmed().split(".");
    QStringList right_full = l_text[1].trimmed().split(".");
    if(left_full.size() == 2)
        ui->cmbLeftCompare->setCurrentText(left_full[1].trimmed());
    if(right_full.size() == 2)
        ui->cmbRightCompare->setCurrentText(right_full[1].trimmed());

    //    if(ind_compare != -1){
    //        QString leftTable = text.left(ind_compare).trimmed();
    //        QString rightTable = text.right(text.length() - ind_compare - 2).trimmed();
    //        if(ui->cmbLeftCompare->findText(leftTable) != -1){
    //            ui->cmbLeftCompare->setCurrentIndex(ui->cmbLeftCompare->findText(leftTable));
    //        }
    //        if(ui->cmbRightCompare->findText(rightTable) != -1){
    //            ui->cmbRightCompare->setCurrentIndex(ui->cmbRightCompare->findText(rightTable));
    //        }
    //    }
}

void CompareWidget::onCuurrentTextChanged(const QString &arg1)
{
    //    QComboBox* cmb = qobject_cast<QComboBox*>(sender());
    //    if(cmb){
    //        QString table_name = cmb->property("table_name").toString();
    //        cmb->setCurrentText(table_name + "." + arg1);
    //    }
}


void CompareWidget::onOpenArbitraryExpressionDialog()
{
    if(!table_model.is_array())
        return;



}

#endif
