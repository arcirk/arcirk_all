#ifndef IS_OS_ANDROID
#include "texteditdialog.h"
#include "ui_texteditdialog.h"

TextEditDialog::TextEditDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TextEditDialog)
{
    ui->setupUi(this);
}

TextEditDialog::~TextEditDialog()
{
    delete ui;
}

void TextEditDialog::setText(const QString &text)
{
    ui->plainTextEdit->setPlainText(text);
}

QString TextEditDialog::text() const
{
    return ui->plainTextEdit->toPlainText();
}
#endif
