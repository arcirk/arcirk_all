#ifndef PLUGINPROPERTYDIALOG_H
#define PLUGINPROPERTYDIALOG_H

#include <QDialog>
#include <QJsonObject>
#include "propertycontrol.h"

namespace Ui {
class PluginPropertyDialog;
}

namespace arcirk::plugins {
    class PluginPropertyDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit PluginPropertyDialog(const QJsonObject& object, QWidget *parent = nullptr);
        ~PluginPropertyDialog();

    private:
        Ui::PluginPropertyDialog *ui;
        QJsonObject m_object;
        QList<PropertyControl*> m_ctrls;

        void createControls();
    };
}
#endif // PLUGINPROPERTYDIALOG_H
