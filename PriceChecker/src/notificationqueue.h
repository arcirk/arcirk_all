#ifndef NOTIFICATIONQUEUE_H
#define NOTIFICATIONQUEUE_H

#include <QObject>
#include <QQueue>

class NotificationQueue : public QObject
{
    Q_OBJECT
public:
    NotificationQueue(QObject *parent = nullptr);

    Q_INVOKABLE void add(const QString& message);
    Q_INVOKABLE QString get();
    Q_INVOKABLE bool isEmpty() const;

private:
    QQueue<QString> m_queue;
};

#endif // NOTIFICATIONQUEUE_H
