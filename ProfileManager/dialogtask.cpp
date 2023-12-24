#include "dialogtask.h"
#include "ui_dialogtask.h"
#include <QStringListModel>
#include <QFileDialog>
#include <QUuid>
#include <QFileInfo>
#include <QMessageBox>
#include "facelib.h"

DialogTask::DialogTask(arcirk::services::task_options& task_data, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogTask),
    task_data_(task_data)
{
    ui->setupUi(this);

    if(task_data_.uuid.empty()){
        task_data_.uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    }
    ui->name->setText(task_data.name.c_str());
    ui->synonum->setText(task_data.synonum.c_str());
    ui->allowed->setCheckState(task_data.allowed ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->predefined->setCheckState(task_data.predefined ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->days_of_week->setText(task_data.days_of_week.c_str());
    ui->interval->setValue(task_data.interval);
    ui->uuid->setText(task_data.uuid.c_str());
    ui->script->setText(task_data.script_synonum.c_str());
    ui->txtComment->setText(task_data.comment.c_str());

    ui->name->setEnabled(!task_data.predefined);
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
    connect(ui->btnSavePlugin, &QToolButton::clicked, this, &DialogTask::onBtnSavePluginClicked);
    connect(ui->cmbScriptType, &QComboBox::currentIndexChanged, this, &DialogTask::onCmbScriptTypeCurrentIndexChanged);
    connect(ui->btnPluginParam, &QToolButton::clicked, this, &DialogTask::onOpenParamDialog);
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
    if(!m_plugin_file.isEmpty())
        task_data_.reset_version = true;

    QDialog::accept();
}


void DialogTask::onBtnSelectPluginClicked()
{
    if(task_data_.type_script == 1){
        auto result = QFileDialog::getOpenFileName(this, "Выбор файла", "", "Биллиотека (*.dll)");
        if(!result.isEmpty()){
            task_data_.script = ByteArray();
            m_plugin_file = result;
            QFileInfo f(result);
            ui->script->setText(f.fileName());
        }
    }
}

void DialogTask::onBtnSavePluginClicked()
{
//    if(task_data_.type_script == 1 && task_data_.script.size() > 0){
//        auto result = QFileDialog::getSaveFileName(this, "Выбор файла", "", "Биллиотека (*.dll)");
//        if(!result.isEmpty()){
//            arcirk::write_file(result.toStdString(), task_data_.script);
//        }
//    }
}


void DialogTask::onCmbScriptTypeCurrentIndexChanged(int index)
{
    task_data_.type_script = index;
}

void DialogTask::onOpenParamDialog()
{
    openParamDialog();
}

void DialogTask::openParamDialog()
{
    if(task_data_.type_script == 1){
        if(ui->script->text().isEmpty())
            return;
        QPath p(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
        p /= "plugins";
        p /= QString(task_data_.uuid.c_str());

        QDir dir(p.path());
        if(!dir.exists())
            dir.mkpath(p.path());

        p /= ui->script->text();



        if(!p.exists()){
            if(QMessageBox::question(this, "Параметры плагина", "Для настройки параметров требуется установить плагин на текущий компьютер. Продолжить?") == QMessageBox::No)
                return;
            QPath path(QString("html\\client_data\\plugins"));
            path /= QString(task_data_.uuid.c_str());
            path /= ui->script->text();

            auto param = json::object({
                {"file_name", path.path().toStdString()}
            });

            emit doInstallPlugin(param, task_data_.uuid);
        }else{
            using namespace arcirk::plugins;
            try {
                auto loader = new QPluginLoader(p.path(), this);
                QObject *obj = loader->instance();
                IAIPlugin* plugin
                    = qobject_cast<IAIPlugin*>(obj);
                if(plugin){
                    if(task_data_.param.size() > 0){
                        plugin->setParam(arcirk::byte_array_to_string(task_data_.param).c_str());
                    }
                    if(plugin->editParam(this)){
                        auto param_t = arcirk::string_to_byte_array(plugin->param().toStdString());
                        task_data_.param = ByteArray(param_t.size());
                        std::copy(param_t.begin(), param_t.end(), task_data_.param.begin());
                    }
                    loader->unload();
                }
                delete loader;
            } catch (const std::exception& e) {
                qCritical() << e.what();
            }

        }
    }
}

void DialogTask::generate_cron_string()
{
    QString hour_interval;
    QString sec_interval;
    auto start = ui->start_task->time();
    auto end = ui->end_task->time();
    if(start.isValid() && end.isValid()){
        if(start > end){
            ui->lblCron->setText("<Время начала не может быть больше чем время окончания!>");
            return;
        }
    }
    if(start.isValid())
        hour_interval = QString::number(start.hour());
    if(end.isValid()){
        hour_interval.append("-");
        hour_interval.append(QString::number(start.hour()));
    }
    //auto start_date = ui->start_task->
}

void DialogTask::onEndInstallPlugin(const QString& file_name)
{
    using namespace arcirk::plugins;
    try {
        auto loader = new QPluginLoader(file_name, this);
        QObject *obj = loader->instance();
        IAIPlugin* plugin
            = qobject_cast<IAIPlugin*>(obj);
        if(plugin){
            if(plugin->editParam(this)){
                auto param_t = arcirk::string_to_byte_array(plugin->param().toStdString());
                task_data_.param = ByteArray(param_t.size());
                std::copy(param_t.begin(), param_t.end(), task_data_.param.begin());
            }
            loader->unload();
        }
        delete loader;
    } catch (const std::exception& e) {
        qCritical() << e.what();
    }
}

