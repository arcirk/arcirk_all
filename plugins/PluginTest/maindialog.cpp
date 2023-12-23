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
#include <QPdfDocument>
#include <QPdfSelection>

MainDialog::MainDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MainDialog)
{
    ui->setupUi(this);

    connect(ui->btnCreate, &QPushButton::clicked, this, &MainDialog::onBtnCreateClicked);
    connect(ui->btnSelect, &QToolButton::clicked, this, &MainDialog::onSelectPlugin);
    connect(ui->lineEdit, &QLineEdit::textChanged, this, &MainDialog::onTextChanged);
    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &MainDialog::onDialogBoxClicked);
    connect(ui->btnParam, &QToolButton::clicked, this, &MainDialog::onBtnParamClicked);

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
    try {
        QFile f(ui->lineEdit->text());
        if(!f.exists()){
            QMessageBox::critical(this, "Ошибка", "Файл плагина не существует!");
            return;
        }
        auto loader = new QPluginLoader(f.fileName(), this);
        QObject *obj = loader->instance();
        IAIPlugin* plugin
            = qobject_cast<IAIPlugin*>(obj);
        if(plugin){
            if(!plugin->accept())
                QMessageBox::critical(this, "Ошибка", plugin->lastError());
            else
                QMessageBox::information(this, "Ошибка", "Обработка успешно завершена!");
            loader->unload();
        }
        delete loader;
    } catch (const std::exception& e) {
        qCritical() << e.what();
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


void MainDialog::onBtnParamClicked()
{
    using namespace arcirk::plugins;
    try {
        QFile f(ui->lineEdit->text());
        if(!f.exists()){
            QMessageBox::critical(this, "Ошибка", "Файл плагина не существует!");
            return;
        }
        auto loader = new QPluginLoader(f.fileName(), this);
        QObject *obj = loader->instance();
        IAIPlugin* plugin
            = qobject_cast<IAIPlugin*>(obj);
        if(plugin){
            if(plugin->editParam(this)){
                loader->unload();
            }
        }
        delete loader;
    } catch (const std::exception& e) {
        qCritical() << e.what();
    }

}


void MainDialog::on_btnSelectPdf_clicked()
{
    auto result = QFileDialog::getOpenFileName(this, "Выбор файла...", "", "Файлы pdf(*.pdf)");
    if(!result.isEmpty())
        ui->txtPdfFile->setText(result);
}


void MainDialog::on_btnReadPdf_clicked()
{
    auto pdf = QPdfDocument(this);
    auto res = pdf.load(ui->txtPdfFile->text());
    qDebug() << res;

    auto ba = pdf.getAllText(0);

    auto text = ba.text();

    if(text.indexOf("40702810218350004297") != -1){
        qDebug() << "Это выписка";
    }else
        qDebug() << "Счет не найден!";

}

void MainDialog::on_btnHash_clicked()
{
    QString concatenated = "IIS_1C:LbyFvj1";
    QByteArray data = concatenated.toLocal8Bit().toBase64();
    qDebug() << data;
}


void MainDialog::on_pushButton_clicked()
{
    using namespace arcirk::plugins;
    try {
        QFile f(ui->lineEdit->text());
        if(!f.exists()){
            QMessageBox::critical(this, "Ошибка", "Файл плагина не существует!");
            return;
        }
        auto loader = new QPluginLoader(f.fileName(), this);
        QObject *obj = loader->instance();
        IAIPlugin* plugin
            = qobject_cast<IAIPlugin*>(obj);
        if(plugin){
            qDebug() << qPrintable(plugin->param());
            loader->unload();
        }
        delete loader;
    } catch (const std::exception& e) {
        qCritical() << e.what();
    }
}

