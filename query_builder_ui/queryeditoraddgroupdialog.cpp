#include "queryeditoraddgroupdialog.h"
#include "ui_queryeditoraddgroupdialog.h"

using namespace arcirk::query_builder_ui;

QueryEditorAddGroupDialog::QueryEditorAddGroupDialog(sqlite_functions_info& info, const QString& parent_name, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QueryEditorAddGroupDialog),
    info_(info)
{
    ui->setupUi(this);
    ui->txtParent->setText(parent_name);
    ui->txtName->setText(info_.name.c_str());
}

QueryEditorAddGroupDialog::~QueryEditorAddGroupDialog()
{
    delete ui;
}

void QueryEditorAddGroupDialog::accept()
{
    info_.name = ui->txtName->text().toStdString();
    return QDialog::accept();
}
