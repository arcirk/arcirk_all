#ifndef BANKSTATEMENTSPLUGUN_H
#define BANKSTATEMENTSPLUGUN_H

#include "facelib.h"
#include <QObject>
#include "arcirk.hpp"
#include <QDir>
#include <QFile>
#include <QFileInfo>

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::plugins), plugin_param,
    (std::string, key)
    (ByteArray, value)
    (int, is_group)
    (std::string, ref)
    (std::string, parent)
)

#define PLUGIN_FILE_NAME "BankStatements.json"

namespace arcirk::plugins {

//    struct task_param{
//        QString host;
//        QString token;
//        QString destantion;
//        QString destantion_bnk;
//    };

struct QPath
{
    QPath(const QString& p) {
        m_patch = p;
        //m_info = QFileInfo(p);
    }

    QString path() const{
        return m_patch;
    }

    const QString& operator +(const QString& v){
        if(v != QDir::separator())
            m_patch.append(QDir::separator());
        m_patch.append(v);
        return m_patch;
    }
    const QString& operator +=(const QString& v){
        m_patch.append(v);
        return m_patch;
    }

    const QString& operator /=(const QString& v){
        m_patch.append(QDir::fromNativeSeparators(QDir::separator()));
        m_patch.append(v);
        return m_patch;
    }

    const QString& operator =(const QFile& f){
        m_patch = f.fileName();
        return m_patch;
    }

    const QString& operator =(const QDir& d){
        m_patch = d.path();
        return m_patch;
    }

    const QString& operator <<(const QString& v){
        if(v != QDir::separator())
            m_patch.append(QDir::separator());
        m_patch.append(v);
        return m_patch;
    }

    bool exists() const{
        QFileInfo f(m_patch);
        return f.exists();
    }

    bool isDir(){
        QFileInfo f(m_patch);
        return f.isDir();
    }

    bool isFile(){
        QFileInfo f(m_patch);
        return f.isFile();
    }

    bool mkpath(){
        QDir d(m_patch);
        return d.mkpath(m_patch);
    }


private:
    QString m_patch;

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
