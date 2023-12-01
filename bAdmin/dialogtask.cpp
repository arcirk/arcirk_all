#include "dialogtask.h"
#include "ui_dialogtask.h"
#include <QStringListModel>
#include <QFileDialog>
#include <QUuid>
#include <QFileInfo>

DialogTask::DialogTask(arcirk::services::task_options& task_data, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogTask),
    task_data_(task_data)
{
    ui->setupUi(this);
    if(task_data_.uuid.empty()){
        task_data_.uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
        //task_data_.ref = task_data_.uuid;
    }
    ui->name->setText(task_data.name.c_str());
    ui->synonum->setText(task_data.synonum.c_str());
    //ui->start_task->set
    //ui->end_task->setText(task_data.name.c_str());
    ui->allowed->setCheckState(task_data.allowed ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->predefined->setCheckState(task_data.predefined ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->days_of_week->setText(task_data.days_of_week.c_str());
    ui->interval->setValue(task_data.interval);
    ui->uuid->setText(task_data.uuid.c_str());
    ui->script->setText(task_data.script_synonum.c_str());
    ui->txtComment->setText(task_data.comment.c_str());

    ui->name->setEnabled(!task_data.predefined);
    //ui->uuid->setEnabled(!task_data.predefined);
    auto buttons = ui->buttonBox->buttons();
    foreach (auto btn, buttons) {
        if(btn->text() == "Cancel")
            btn->setText("Отмена");
    }

    if(!task_data.name.empty())
        setWindowTitle(QString("Служба (%1)").arg(task_data.name.c_str()));

    ui->cmbScriptType->setModel(new QStringListModel(QStringList{"Не используется", "Плагин", "Модуль Python", "Модуль OneScript", "Модуль JavaScript"}, ui->cmbScriptType));
    ui->cmbScriptType->setCurrentIndex(task_data_.type_script);

    connect(ui->btnSelectPlugin, &QToolButton::clicked, this, &DialogTask::onBtnSelectPluginClicked);
    connect(ui->cmbScriptType, &QComboBox::currentIndexChanged, this, &DialogTask::onCmbScriptTypeCurrentIndexChanged);
}

DialogTask::~DialogTask()
{
    delete ui;
}

void DialogTask::accept()
{
    task_data_.name = ui->name->text().trimmed().toStdString();
    task_data_.synonum = ui->synonum->text().trimmed().toStdString();
    task_data_.days_of_week = ui->days_of_week->text().trimmed().toStdString();
    task_data_.uuid = ui->uuid->text().trimmed().toStdString();
    task_data_.script_synonum = ui->script->text().trimmed().toStdString();
    task_data_.interval = ui->interval->value();
    task_data_.allowed = ui->allowed->isChecked();
    task_data_.comment = ui->txtComment->toPlainText().toStdString();
    task_data_.type_script = ui->cmbScriptType->currentIndex();
    QDialog::accept();
}


void DialogTask::onBtnSelectPluginClicked()
{
    if(task_data_.type_script == 1){
        auto result = QFileDialog::getOpenFileName(this, "Выбор файла", "", "Биллиотека (*.dll)");
        if(!result.isEmpty()){
//            ui->script->setText(result);
            ByteArray res{};
            arcirk::read_file(result.toStdString(), task_data_.script);
//            auto test = pre::json::to_json(task_data_);
//            qDebug() << arcirk::type_string(test["script"]).c_str();
            QFileInfo f(result);
            ui->script->setText(f.fileName());
        }
    }
}


void DialogTask::onCmbScriptTypeCurrentIndexChanged(int index)
{
    task_data_.type_script = index;
}

