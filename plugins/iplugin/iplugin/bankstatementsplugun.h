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

    struct param_t
    {
        param_t() {}
        param_t(const QByteArray data) {
            fromRaw(data);
        }

        QList<QByteArray> files;
        QList<QString> names;
        QList<QDateTime> creates;
        QList<QString> suffix;
        QList<QString> loc_paths;

        void read(const QString& file){
            QFile f(file);
            if(f.open(QIODevice::ReadOnly)){
                files.push_back(f.readAll());
                f.close();
                QFileInfo fs(file);
                auto name = fs.fileName().left(fs.fileName().length() - fs.suffix().length() - 1);
                names.push_back(name);
                suffix.push_back(fs.suffix());
                creates.push_back(fs.lastModified());
                loc_paths.push_back(fs.fileName());
            }


        }

        QByteArray toRaw() const{
            QByteArray ba;
            QDataStream stream {&ba, QIODevice::WriteOnly};
            stream << files;
            stream << names;
            stream << creates;
            stream << suffix;
            stream << loc_paths;
            return ba;
        }

        void fromRaw(const QByteArray& data){
            if(data.size() == 0)
                return;
            QDataStream stream {data};
            stream >> files;
            stream >> names;
            stream >> creates;
            stream >> suffix;
            stream >> loc_paths;

        }

        QString generateName(int index){
            Q_ASSERT(index < files.size());
            Q_ASSERT(index>=0);
            QString result;
            if(names[index].indexOf("Выписка за") != -1){
                result = creates[index].toString("dd.MM.yy.hh.mm.ss");
            }
            return result;
        }
    };
    struct file_details{

        file_details(){};

        QString date;
        QString pay;

        void parse(const QString& file_name){
            if(file_name.indexOf("Выписка за") ==-1)
                return;
            QStringList lst = file_name.split(" ");
            if(lst.size() == 6){
                date = lst[3];
                pay = lst[5];
            }
        }
    };

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
        bool editParam(QWidget* parent);
        QString lastError() const;

    private:
        bool is_valid = false;
        json m_param;
        QString m_last_error;
        task_param m_task_param;

        json default_param() const;
        json read_param() const;
        void write_param();

        QByteArray readData() const;
        QJsonObject http_get(const QString& command, const QString& param);

    };

}


#endif // BANKSTATEMENTSPLUGUN_H
