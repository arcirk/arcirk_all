#ifndef FACELIB_H
#define FACELIB_H
#include <QString>
#include <QVariant>
#include <QtPlugin>

namespace arcirk::plugins {
class IAIPlugin {
public:
    virtual ~IAIPlugin() { }

    //virtual QByteArray readData() const = 0;

    virtual void setParam(const QString& param) = 0;
    virtual bool isValid() = 0;
    virtual bool accept() = 0;
    virtual bool prepare() = 0;
    virtual QByteArray param() const = 0;

    //    virtual void setParam(const QString& param) = 0;
};
}

Q_DECLARE_INTERFACE( arcirk::plugins::IAIPlugin, "ru.arcirk.plugins.PluginInterface/1.0" )
#endif // FACELIB_H
