#include "taskitem.h"
#include "ui_taskitem.h"
#include <QStringListModel>
#include <QUuid>
#include <QFileDialog>
#include <QFileInfo>
#include <QPluginLoader>
#include <QStandardPaths>
#include <QMessageBox>
#include "facelib.h"
#include "global/arcirk_qt.hpp"
#include <QPushButton>
#include <QDialogButtonBox>
#include "gui/pairmodel.h"
#include <QTimeEdit>

using namespace arcirk::tasks;

TaskItem::TaskItem(task_options& task, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TaskItem),
    task_data_(task)
{
    ui->setupUi(this);

    auto pmpdel = new PairModel(this);
    pmpdel->setContent(QList{
        DataPair{"Пользовательский", "users"},
        DataPair{"Eжегодно", "@yearly"},
        DataPair{"Eжемесячно", "@monthly"},
        DataPair{"Eженедельно", "@weekly"},
        DataPair{"Eжедневно", "@daily"},
        DataPair{"Ежечасно", "@hourly"}
    });
    ui->cmbCronType->setModel(pmpdel);
    connect(ui->cmbCronType, &QComboBox::currentIndexChanged, this, &TaskItem::onCronTypeCurrentIndexChanged);

    if(task_data_.uuid.empty()){
        task_data_.uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
        task_data_.name = "Новая задача";
    }
    ui->name->setText(task_data_.name.c_str());
    ui->synonum->setText(task_data_.synonum.c_str());

    ui->allowed->setCheckState(task_data_.allowed ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->predefined->setCheckState(task_data_.predefined ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->days_of_week->setText(task_data_.days_of_week.c_str());
    ui->interval->setValue(task_data_.interval);
    ui->uuid->setText(task_data_.uuid.c_str());
    ui->script->setText(task_data_.script_synonum.c_str());
    ui->txtComment->setText(task_data_.comment.c_str());

    ui->name->setEnabled(!task_data_.predefined);

    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("Отмена");

    if(!task_data_.name.empty())
        setWindowTitle(QString("Служба (%1)").arg(task_data_.name.c_str()));

    ui->cmbScriptType->setModel(new QStringListModel(QStringList{"Не используется", "Плагин", "Модуль Python", "Модуль OneScript", "Модуль JavaScript"}, ui->cmbScriptType));
    ui->cmbScriptType->setCurrentIndex(task_data_.type_script);

    connect(ui->btnSelectPlugin, &QToolButton::clicked, this, &TaskItem::onBtnSelectPluginClicked);
    connect(ui->btnSave, &QToolButton::clicked, this, &TaskItem::onBtnSavePluginClicked);
    connect(ui->cmbScriptType, &QComboBox::currentIndexChanged, this, &TaskItem::onCmbScriptTypeCurrentIndexChanged);
    connect(ui->btnPluginParam, &QToolButton::clicked, this, &TaskItem::onOpenParamDialog);

    const auto list = QStringList{
        "users", "@yearly", "@monthly", "@weekly", "@daily", "@hourly"
    };
    auto text = QString(task_data_.cron_string.c_str());
    int i = 0;
    foreach (const auto itr, list) {
        if(text.indexOf(itr) != -1){
            ui->cmbCronType->setCurrentIndex(i);
            break;
        }
        i++;
    }

    ui->lblCronString->setText(task_data_.cron_string.c_str());

    connect(ui->name, &QLineEdit::textChanged, this, &TaskItem::onNameTextChange);
    connect(ui->synonum, &QLineEdit::textChanged, this, &TaskItem::onSynonimTextChange);
    connect(ui->start_task, &QTimeEdit::userTimeChanged, this, &TaskItem::onStartTaskUserTimeChanged);
    connect(ui->end_task, &QTimeEdit::userTimeChanged, this, &TaskItem::onEndTaskUserTimeChanged);

}

TaskItem::~TaskItem()
{
    delete ui;
}

void TaskItem::accept()
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
    task_data_.cron_string = generateCronText().toStdString();
    QDialog::accept();
}

void TaskItem::onBtnSelectPluginClicked()
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

void TaskItem::onBtnSavePluginClicked()
{

}


void TaskItem::onCmbScriptTypeCurrentIndexChanged(int index)
{
    task_data_.type_script = index;
}

void TaskItem::onOpenParamDialog()
{
    openParamDialog();
}

void TaskItem::onNameTextChange(const QString &value)
{
    if(task_data_.name == task_data_.synonum){
        task_data_.name = value.toStdString();
        task_data_.synonum = task_data_.name;
        ui->synonum->setText(value);
    }
}

void TaskItem::onSynonimTextChange(const QString &value)
{
    task_data_.synonum = value.toStdString();
}

void TaskItem::openParamDialog()
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
            if(m_plugin_file.isEmpty()){
                QPath path(QString("html\\client_data\\plugins"));
                path /= QString(task_data_.uuid.c_str());
                path /= ui->script->text();

                auto param = json::object({
                    {"file_name", path.path().toStdString()}
                });

                emit doInstallPlugin(param, task_data_.uuid);
            }else{
                emit installPrivatePlugin(m_plugin_file, task_data_.uuid.c_str());
            }

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

QString TaskItem::generateCronText() const
{
    if(ui->cmbCronType->currentIndex() > 0){
        return ui->cmbCronType->currentData().toString();
    };

    QTime start_time = ui->start_task->time();
    QTime end_time = ui->end_task->time();

    int h = start_time.hour();
    int m = end_time.minute();

    QString cron_string = "*";
    if((h+m) > 0){
        cron_string.append(QString(" %1 %2").arg(QString::number(m), QString::number(h)));
    }else
        cron_string.append(" * *");

    return cron_string;
}

void TaskItem::onEndInstallPlugin(const QString& file_name)
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

void TaskItem::onCronTypeCurrentIndexChanged(int index)
{
    ui->lblCronString->setText(generateCronText());
    bool enable = index == 0;
    ui->start_task->setEnabled(enable);
    ui->end_task->setEnabled(enable);
    ui->interval->setEnabled(enable);
    ui->days_of_week->setEnabled(enable);
}


void TaskItem::onStartTaskUserTimeChanged(const QTime &time)
{
    ui->lblCronString->setText(generateCronText());
}

void TaskItem::onEndTaskUserTimeChanged(const QTime &time)
{
    ui->lblCronString->setText(generateCronText());
}

