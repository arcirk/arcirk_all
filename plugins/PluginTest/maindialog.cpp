#include "maindialog.h"
#include "ui_maindialog.h"
#include <QPluginLoader>
#include "bankstatementsplugun.h"

MainDialog::MainDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MainDialog)
{
    ui->setupUi(this);
}

MainDialog::~MainDialog()
{
    delete ui;
}


void MainDialog::on_btnCreate_clicked()
{
    using namespace arcirk::plugins;
    QPluginLoader loader("D:/src_arcirk/arcirk_all/arcirk_all/plugins/iplugin/build-bankstatementsplugun-Desktop_Qt_6_5_2_MSVC2019_64bit-Debug/debug/bankstatementsplugun.dll");
    QObject *obj = loader.instance();
    BankStatementsPlugun* plugin
        = qobject_cast<BankStatementsPlugun*>(obj);
    if(plugin){
        plugin->editParam(this);
    }
}

