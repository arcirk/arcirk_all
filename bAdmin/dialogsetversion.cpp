#include "dialogsetversion.h"
#include "ui_dialogsetversion.h"

DialogSetVersion::DialogSetVersion(const arcirk::client::version_application& ver,  QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSetVersion)
{
    ui->setupUi(this);

    ui->spMajor->setValue(ver.major);
    ui->spMinor->setValue(ver.minor);
    ui->spPath->setValue(ver.path);
}

DialogSetVersion::~DialogSetVersion()
{
    delete ui;
}

arcirk::client::version_application DialogSetVersion::getResult()
{
    return arcirk::client::version_application(ui->spMajor->value(), ui->spMinor->value(), ui->spPath->value());
}
