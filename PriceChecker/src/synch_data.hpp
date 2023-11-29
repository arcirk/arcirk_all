#ifndef SYNCH_DATA_HPP
#define SYNCH_DATA_HPP

#include <QFloat16>
#include <QObject>
#include "wssettings.h"
#include <QSqlDatabase>
#include <QDebug>

namespace arcirk::synchronize{


class SynchronizeBase : public QObject{
    Q_OBJECT
public:
    explicit SynchronizeBase(QObject *parent = nullptr): QObject(parent){
            //m_settings = sett;
        };
    ~SynchronizeBase(){
        qDebug() << "~SynchronizeBase";
    };

    virtual void synchronize() = 0;

    virtual const QString connectionName() {return "SynchronizeBase";};

    virtual bool isRunning(){return is_running;};

protected:
    bool is_running;
    arcirk::Settings * m_settings;
    QSqlDatabase sql;
    void open_database();

signals:
    void endSynchronize(const QString& operationName, bool result, const nlohmann::json& details);
    void startSynchronize(const QString& operationName);
};


//Синхронизация документов
class SynchDocumentsBase : public SynchronizeBase{

public:
    explicit SynchDocumentsBase(arcirk::Settings *sett, QObject *parent = nullptr);

    void synchronize() override;
    void setComparisonOfDocuments(const nlohmann::json& resp);
    const QString connectionName() override {return "SynchDocumentsBase";};
private:
    nlohmann::json srv_resp;
    bool synchronizeBaseDocuments(nlohmann::json& sendToSrvDocuments);

};

//Первоначальное заполнение данных
class SynchInitialDataEntry : public SynchronizeBase{

public:
    explicit SynchInitialDataEntry(arcirk::Settings *sett);

    void synchronize() override;

    const QString connectionName() override {return "SynchInitialDataEntry";};

private:


};
}



#endif // SYNCH_DATA_HPP
