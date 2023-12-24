#ifndef BTASKS_H
#define BTASKS_H

#include "bTasks_global.h"
#include "Cron.h"

#include <future>
#include <boost/asio/post.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>

class BTASKS_EXPORT BTasks
{
public:
    BTasks();
};

#endif // BTASKS_H
