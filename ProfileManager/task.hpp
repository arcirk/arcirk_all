#ifndef  BWEBSOCKETS_TASK_HPP
#define  BWEBSOCKETS_TASK_HPP

#include "shared_struct.hpp"
#include <ctime>
#include <iostream>
#include <iomanip>
#include <functional>

#ifdef _WINDOWS
#include <sdkddkver.h>
#include <boost/asio.hpp>
#include <windows.h>
#include <boost/locale.hpp>
#else
#include <boost/asio.hpp>
#endif //_WINDOWS


namespace net = boost::asio;                    // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include "thread_pool.hpp"
#include <boost/algorithm/hex.hpp>

#include <QUuid>


namespace arcirk::services{


class task : boost::noncopyable
{
    std::mutex output_mutex;
public:
    typedef std::function<void(task_options)> handler_fn;

    task(boost::asio::io_service& ioService
         , std::string const& name
         , int interval
         , handler_fn task_fn
         , std::shared_ptr<thread_pool> workers)
        : ioService(ioService)
        , timer(ioService)
        , task_(task_fn)
    //, workers_(std::move(workers))
    {
        workers_ = workers;
        opt_.name = name;
        opt_.uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
        opt_.interval = interval;
        task_ = task_fn;
        //log_text("Create PeriodicTask '" + name + "'");
        // Schedule start to be ran by the io_service
        ioService.post(boost::bind(&task::start, this));
    }

    task(boost::asio::io_service& ioService
         , const task_options& opt
         , handler_fn task_fn
         , std::shared_ptr<thread_pool> workers)
        : ioService(ioService)
        , timer(ioService)
        , opt_(std::move(opt))
    {
        workers_ = workers;
        interval = opt.interval;
        name = opt.name;
        task_ = task_fn;
        //opt_ = opt;
        //log_text("Create PeriodicTask '" + name + "'");
        // Schedule start to be ran by the io_service
        ioService.post(boost::bind(&task::start, this));
    }

    void execute(boost::system::error_code const& e)
    {
        if (e != boost::asio::error::operation_aborted) {
            //log_text("Execute PeriodicTask '" + name + "'");
            count++;
            task_(opt_);

            timer.expires_at(timer.expires_at() + boost::posix_time::seconds(interval));
            start_wait();
        }
    }

    void start()
    {
        //arcirk::log(__FUNCTION__, "start task '" + opt_.synonum + "'");

        workers_->post([=]
                       {
                           //auto guard = std::lock_guard(output_mutex);
                           //task_(opt_);
                       });

        auto val = workers_->submit([]
                                    {
                                        return "ok!";
                                    });
        workers_->wait();

        timer.expires_from_now(boost::posix_time::seconds(interval));

        start_wait();
    }

    void start_now(const int custom_interval)
    {
        //arcirk::log(__FUNCTION__, "start task '" + opt_.synonum + "'");

        workers_->post([=]
                       {
                           //auto guard = std::lock_guard(output_mutex);
                           //task_(opt_);
                       });

        auto val = workers_->submit([]
                                    {
                                        return "ok!";
                                    });
        workers_->wait();

        if(custom_interval == -1)
            timer.expires_from_now(boost::posix_time::seconds(interval));
        else
            timer.expires_from_now(boost::posix_time::seconds(custom_interval));
        start_wait();
    }

    void stop(){
        //arcirk::log(__FUNCTION__,"stop task '" + opt_.synonum + "'");
        timer.cancel();
    };

private:
    void start_wait()
    {
        timer.async_wait(boost::bind(&task::execute
                                     , this
                                     , boost::asio::placeholders::error));
    }

private:
    boost::asio::io_service& ioService;
    boost::asio::deadline_timer timer;
    handler_fn task_;
    std::string name;
    int interval;
    std::shared_ptr<thread_pool> workers_;
    task_options opt_;
    int count = 0;
};

class task_scheduler : boost::noncopyable
{
    std::shared_ptr<thread_pool> workers_;
public:
    explicit
        task_scheduler()
    {
        threads = 4;
        capacity = 8;
        workers_ = std::make_shared<thread_pool>(threads, capacity);
    }
    void run()
    {
        if(is_started_)
        {
            //arcirk::log(__FUNCTION__,"service already started");
            return;
        }
        //arcirk::log(__FUNCTION__,"run all tasks");
        is_started_ = true;
        io_service.run();
    }

    void stop(){
        //arcirk::log(__FUNCTION__,"stop all tasks");
        is_started_ = false;
        io_service.stop();
    }

    void clear(){
        if(is_started_)
        {
            //arcirk::log(__FUNCTION__,"service already started");
            return;
        }
        workers_.reset();
        tasks.clear();
    }

    void stop_task(const QUuid& uuid){
        auto itr = tasks.find(uuid);
        if(itr != tasks.end())
            itr->second->stop();
    }

    void start_task(const QUuid& uuid){
        auto itr = tasks.find(uuid);
        if(itr != tasks.end())
            itr->second->start();
    }

    void start_task_now(const QUuid& uuid, int custom_interval){
        auto itr = tasks.find(uuid);
        if(itr != tasks.end())
            itr->second->start_now(custom_interval);
    }

    void add_task(std::string const& name
                  , const QUuid& uuid
                  , task::handler_fn const& task_fn
                  , int interval)
    {
        tasks.insert(std::pair<QUuid, std::unique_ptr<task>>(uuid, std::make_unique<task>(std::ref(io_service)
                                                                                                       , name, interval, task_fn, workers_)));
    }

    void add_task(const task_options& opt
                  , task::handler_fn const& task_fn)
    {
        tasks.insert(std::pair<QUuid, std::unique_ptr<task>>(QUuid::fromString(opt.uuid.c_str()), std::make_unique<task>(std::ref(io_service)
                                                                                                                                          , opt, task_fn, workers_)));
    }

    bool is_started(){
        return is_started_;
    }
private:
    bool is_started_ = false;
    boost::asio::io_service io_service;
    std::map<QUuid, std::unique_ptr<task>> tasks;
    std::size_t threads;
    std::size_t capacity;

};
}


#endif
