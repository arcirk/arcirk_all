#include "notificationqueue.h"

NotificationQueue::NotificationQueue(QObject *parent)
: QObject{parent}
{

}

void NotificationQueue::add(const QString &message)
{
    m_queue.enqueue(message);
}

QString NotificationQueue::get()
{
    if(!m_queue.isEmpty())
        return m_queue.dequeue();

    return "";
}

bool NotificationQueue::isEmpty() const
{
    return m_queue.isEmpty();
}
