#ifndef DIALOGTASK_H
#define DIALOGTASK_H

#include <QDialog>
#include <shared_struct.hpp>

namespace Ui {
class DialogTask;
}

class DialogTask : public QDialog
{
    Q_OBJECT

public:
    explicit DialogTask(arcirk::services::task_options& task_data, QWidget *parent = nullptr);
    ~DialogTask();

    void accept() override;

    QString currentScript() const {return m_plugin_file;}

private slots:
    void onBtnSelectPluginClicked();
    void onBtnSavePluginClicked();
    void onCmbScriptTypeCurrentIndexChanged(int index);
    void onOpenParamDialog();

private:
    Ui::DialogTask *ui;
    arcirk::services::task_options& task_data_;

    QString m_plugin_file;

    void openParamDialog();
    void openParam(const QString& file_name);

signals:
    void doInstallPlugin(const json &param, const std::string& ref);
    void installPrivatePlugin(const QString &file_name, const QString &task_ref);

public slots:
    void onEndInstallPlugin(const QString& file_name);

};

#endif // DIALOGTASK_H
