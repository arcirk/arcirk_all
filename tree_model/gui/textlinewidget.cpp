#include "textlinewidget.h"
#include "ui_textlinewidget.h"
//#include <QToolButton>

using namespace arcirk::tree_model;

TextLineWidget::TextLineWidget(int row, int column, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TextLineWidget)
{
    ui->setupUi(this);

    m_row = row;
    m_column = column;

    connect(ui->btnEditText, &QToolButton::clicked, this, &TextLineWidget::onEditTextDialog);
}

TextLineWidget::~TextLineWidget()
{
    delete ui;
}

void TextLineWidget::setText(const QString &text)
{
    ui->lineEdit->setText(text);
}

QString TextLineWidget::text() const
{
    return ui->lineEdit->text();
}

void TextLineWidget::setStyleSheet(const QString &style)
{
    ui->lineEdit->setStyleSheet(style);
}

void TextLineWidget::onLineEditEditingFinished()
{

}

void TextLineWidget::onEditTextDialog()
{
    emit toolButtonClicked(m_row, m_column, ui->lineEdit->text());
}
