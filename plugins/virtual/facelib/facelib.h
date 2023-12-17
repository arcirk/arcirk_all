#ifndef FACELIB_H
#define FACELIB_H
#include <QString>
#include <QVariant>
#include <QtPlugin>
#include <QWidget>

namespace arcirk::plugins {
    class IAIPlugin {
    public:
        virtual ~IAIPlugin() = default;

        virtual void setParam(const QString& param) = 0;
        virtual bool editParam(QWidget* parent) = 0;
        virtual bool isValid() = 0;
        virtual bool accept() = 0;
        virtual bool prepare() = 0;
        virtual QByteArray param() const = 0;
        virtual QString lastError() const = 0;

    };
}

Q_DECLARE_INTERFACE( arcirk::plugins::IAIPlugin, "ru.arcirk.plugins.PluginInterface/1.0" )
#endif // FACELIB_H
