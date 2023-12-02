#ifndef BANKARHIVE_H
#define BANKARHIVE_H

#include "bankarhiveinterface.h"
#include <QList>
#include <QByteArray>
#include <QFile>
#include <QUuid>
#include <QFileInfo>
#include <QDateTime>

#define WS_RESULT_SUCCESS "success"
#define WS_RESULT_ERROR "error"

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

struct task_param{
    QString host;
    QString token;
    QString destantion;
    QString destantion_bnk;
};

struct file_details{

    file_details(){};

    QString date;
    QString pay;

    void parse(const QString& file_name){
        if(file_name.indexOf("Выписка за") ==-1)
            return;
        //QString str = file_name.right(file_name.length() - QString("Выписка за").length() - 1);
        QStringList lst = file_name.split(" ");
        if(lst.size() == 6){
            date = lst[3];
            pay = lst[5];
        }
    }
};

class BankArhive : public QObject, public IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "ru.arcirk.plugins.BankArhiveInterface/1.0")
    Q_INTERFACES( IPlugin )
public:
    ~BankArhive();

    void setParam(const QString& param);
    bool isValid();
    bool accept();
    bool prepare();
    QByteArray param() const;

private:
    bool is_valid = false;
    QJsonObject m_param;
    task_param m_task_param;

    QByteArray readData() const;
    QJsonObject http_get(const QString& command, const QString& param);


private:


};

#endif // BANKARHIVE_H
