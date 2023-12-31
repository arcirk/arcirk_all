#ifndef TASKITEM_H
#define TASKITEM_H

#include <QDialog>
#include "common.hpp"

namespace Ui {
class TaskItem;
}

namespace arcirk::tasks {
    class TaskItem : public QDialog
    {
        Q_OBJECT

    public:
        explicit TaskItem(task_options& task, QWidget *parent = nullptr);
        ~TaskItem();

        void accept() override;

        QString currentScript() const {return m_plugin_file;}

    private slots:
        void onBtnSelectPluginClicked();
        void onBtnSavePluginClicked();
        void onCmbScriptTypeCurrentIndexChanged(int index);
        void onOpenParamDialog();
        void onNameTextChange(const QString& value);
        void onSynonimTextChange(const QString& value);

    private:
        Ui::TaskItem *ui;
        task_options &task_data_;

        QString m_plugin_file;

        void openParamDialog();
        void openParam(const QString& file_name);
        QString generateCronText() const;

    signals:
        void doInstallPlugin(const json &param, const std::string& ref);
        void installPrivatePlugin(const QString &file_name, const QString &task_ref);

    public slots:
        void onEndInstallPlugin(const QString& file_name);

    private slots:
        void onCronTypeCurrentIndexChanged(int index);
        void onStartTaskUserTimeChanged(const QTime &time);
        void onEndTaskUserTimeChanged(const QTime &time);
    };

}


#endif // TASKITEM_H
