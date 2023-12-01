#include "dialogimporttodatabase.h"
#include "ui_dialogimporttodatabase.h"

DialogImportToDatabase::DialogImportToDatabase(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogImportToDatabase)
{
    ui->setupUi(this);

    auto buttons = ui->buttonBox->buttons();
    foreach (auto btn, buttons) {
        if(btn->text() == "Cancel")
            btn->setText("Отмена");
    }
}

DialogImportToDatabase::~DialogImportToDatabase()
{
    delete ui;
}
