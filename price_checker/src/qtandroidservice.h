#ifndef QTANDROIDSERVICE_H
#define QTANDROIDSERVICE_H
#include <QObject>

#ifdef IS_OS_ANDROID
#include <QtCore/private/qandroidextras_p.h>

class QtAndroidService : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString deviceId READ androidId NOTIFY deviceIdChanged)
public:
    QtAndroidService(QObject *parent = nullptr);

    static QtAndroidService *instance() { return m_instance; }
    Q_INVOKABLE void sendToService(const QString &name);

    Q_INVOKABLE void startUpdate(const QString& file_path);

    Q_INVOKABLE QString getUuid();

signals:
    void messageFromService(const QString &message);
    void deviceIdChanged();

private:
    void registerNatives();
    void registerBroadcastReceiver();
    QString androidId();

    static QtAndroidService *m_instance;



};
#else
class QtAndroidService : public QObject
{
    Q_OBJECT
public:
    QtAndroidService(QObject *parent = nullptr);
signals:
    void messageFromService(const QString &message);
};

#endif
#endif // QTANDROIDSERVICE_H
