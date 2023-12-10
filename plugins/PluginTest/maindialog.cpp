#include "maindialog.h"
#include "ui_maindialog.h"
#include <QPluginLoader>
#include "facelib.h"
#include <QPushButton>
#include <QToolButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QLineEdit>
#include <QDialogButtonBox>

MainDialog::MainDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MainDialog)
{
    ui->setupUi(this);

    connect(ui->btnCreate, &QPushButton::clicked, this, &MainDialog::onBtnCreateClicked);
    connect(ui->btnSelect, &QToolButton::clicked, this, &MainDialog::onSelectPlugin);
    connect(ui->lineEdit, &QLineEdit::textChanged, this, &MainDialog::onTextChanged);
    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &MainDialog::onDialogBoxClicked);

    QSettings settings("TestPlugin", "Star Test");

    auto v = settings.value("plugin_path");
    if(v.isValid()){
        ui->lineEdit->setText(v.toString());
    }

}

MainDialog::~MainDialog()
{
    delete ui;
}


void MainDialog::onBtnCreateClicked()
{
    using namespace arcirk::plugins;
    QFile f(ui->lineEdit->text());
    if(!f.exists()){
        QMessageBox::critical(this, "Ошибка", "Файл плагина не существует!");
        return;
    }
    QPluginLoader loader(f.fileName());
    QObject *obj = loader.instance();
    IAIPlugin* plugin
        = qobject_cast<IAIPlugin*>(obj);
    if(plugin){
        if(plugin->editParam(this)){
            loader.unload();
        }
    }
}

void MainDialog::onSelectPlugin()
{
    auto result = QFileDialog::getOpenFileName(this, "Выбор файла...", "", "Файлы dll(*.dll)");
    if(!result.isEmpty())
        ui->lineEdit->setText(result);
}

void MainDialog::onTextChanged(const QString &value)
{
    QSettings settings("TestPlugin", "Star Test");
    settings.setValue("plugin_path", value);
}

void MainDialog::onDialogBoxClicked(QAbstractButton *button)
{
    if(button == ui->buttonBox->button(QDialogButtonBox::Close))
        QApplication::exit();
}

