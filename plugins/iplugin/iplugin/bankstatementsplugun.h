#ifndef BANKSTATEMENTSPLUGUN_H
#define BANKSTATEMENTSPLUGUN_H

#include "facelib.h"
#include <QObject>

namespace arcirk::plugins {

    struct task_param{
        QString host;
        QString token;
        QString destantion;
        QString destantion_bnk;
    };

    class BankStatementsPlugun : public QObject, public IAIPlugin
    {
        Q_OBJECT
        Q_PLUGIN_METADATA(IID "ru.arcirk.plugins.PluginInterface/1.0")
        Q_INTERFACES( arcirk::plugins::IAIPlugin )
    public:
        ~BankStatementsPlugun();

        void setParam(const QString& param);
        bool isValid();
        bool accept();
        bool prepare();
        QByteArray param() const;

        void editParam(QWidget* parent);


    private:
        bool is_valid = false;
        QJsonObject m_param;
        task_param m_task_param;

//        QByteArray readData() const;
//        QJsonObject http_get(const QString& command, const QString& param);
    };

}


#endif // BANKSTATEMENTSPLUGUN_H
