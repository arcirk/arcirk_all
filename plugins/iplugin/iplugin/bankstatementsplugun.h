#ifndef BANKSTATEMENTSPLUGUN_H
#define BANKSTATEMENTSPLUGUN_H

#include "facelib.h"
#include <QObject>
#include "arcirk.hpp"

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::plugins), plugin_param,
    (std::string, key)
    (ByteArray, value)
    (int, is_group)
    (std::string, ref)
    (std::string, parent)
)

#define PLUGIN_NAME "BankStatements"

namespace arcirk::plugins {

//    struct task_param{
//        QString host;
//        QString token;
//        QString destantion;
//        QString destantion_bnk;
//    };

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
        bool editParam(QWidget* parent);


    private:
        bool is_valid = false;
        json m_param;
        QString m_last_error;

        json default_param() const;
        json read_param() const;
        void write_param();

    };

}


#endif // BANKSTATEMENTSPLUGUN_H
