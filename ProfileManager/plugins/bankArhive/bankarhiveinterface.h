#ifndef BANKARHIVEINTERFACE_H
#define BANKARHIVEINTERFACE_H
#include <QString>
#include <QVariant>
#include <QtPlugin>

class IPlugin {
public:
    virtual ~IPlugin() { }

    //virtual QByteArray readData() const = 0;

    virtual void setParam(const QString& param) = 0;
    virtual bool isValid() = 0;
    virtual bool accept() = 0;
    virtual bool prepare() = 0;
    virtual QByteArray param() const = 0;

//    virtual void setParam(const QString& param) = 0;
};

Q_DECLARE_INTERFACE( IPlugin, "ru.arcirk.plugins.BankArhiveInterface/1.0" )

#endif // BANKARHIVEINTERFACE_H
