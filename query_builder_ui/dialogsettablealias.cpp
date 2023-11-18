#include "dialogsettablealias.h"
#include "ui_dialogsettablealias.h"

DialogSetTableAlias::DialogSetTableAlias(std::string& name, std::string& alias, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSetTableAlias),
    m_name(name),
    m_alias(alias)
{
    ui->setupUi(this);

    ui->lblName->setText(name.c_str());
    ui->lblAlias->setText(alias.c_str());
}

DialogSetTableAlias::~DialogSetTableAlias()
{
    delete ui;
}

void DialogSetTableAlias::accept()
{
    m_alias = ui->lblAlias->text().toStdString();
    return QDialog::accept();
}
