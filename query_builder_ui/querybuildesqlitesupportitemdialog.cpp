#include "querybuildesqlitesupportitemdialog.h"
#include "ui_querybuildesqlitesupportitemdialog.h"
#include "qsourcehighliter.h"
#include <QToolButton>

using namespace arcirk::query_builder_ui;
using namespace source_highlite;

QueryBuildeSqliteSupportItemDialog::QueryBuildeSqliteSupportItemDialog(sqlite_functions_info& info, const QString& parent_name, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QueryBuildeSqliteSupportItemDialog),
    info_(info)
{
    ui->setupUi(this);

    ui->txtParent->setText(parent_name);
    ui->txtName->setText(info_.name.c_str());
    ui->txtTemplate->setPlainText(QByteArray::fromBase64(info_.fun.c_str()).data());
    ui->txtDesc->setPlainText(QByteArray::fromBase64(info_.desc.c_str()).data());

    auto highlighter = new QSourceHighliter(ui->txtTemplate->document());
    highlighter->setCurrentLanguage(QSourceHighliter::CodeSQL);

    connect(ui->btnToUpper, &QToolButton::clicked, this, &QueryBuildeSqliteSupportItemDialog::onBtnToUpperClicked);
}

QueryBuildeSqliteSupportItemDialog::~QueryBuildeSqliteSupportItemDialog()
{
    delete ui;
}

void QueryBuildeSqliteSupportItemDialog::accept()
{
    info_.name = ui->txtName->text().toStdString();
    info_.desc = ui->txtDesc->toPlainText().toUtf8().toBase64().toStdString();
    info_.fun = ui->txtTemplate->toPlainText().toUtf8().toBase64().toStdString();
    return QDialog::accept();
}

void QueryBuildeSqliteSupportItemDialog::onBtnToUpperClicked()
{
    ui->txtName->setText(ui->txtName->text().toUpper());
}

