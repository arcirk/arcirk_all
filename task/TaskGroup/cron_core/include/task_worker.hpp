#ifndef TASK_WORKER_HPP
#define TASK_WORKER_HPP

#include <iostream>
#include <functional>
#include <string>
#include <ctime>
#include <vector>
#include <map>
//#include <sstream>
#include <string_view>
#include <fstream>

#ifdef _WINDOWS
#include <sdkddkver.h>
#include <boost/asio.hpp>
#include <windows.h>
#include <boost/locale.hpp>
#else
#include <boost/asio.hpp>
#endif //_WINDOWS

#include <boost/filesystem.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/adapted/struct/define_struct.hpp>
#include <boost/fusion/include/define_struct.hpp>

#include "boost/date_time/posix_time/posix_time.hpp"

#include "thread_pool.hpp"
#include "croncpp.hpp"

typedef unsigned char BYTE;
typedef std::vector<BYTE> ByteArray;

#define NIL_STRING_UUID "00000000-0000-0000-0000-000000000000"
//static constexpr time_t const NULL_TIME = -1;

//namespace net = boost::asio;

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::services), task_options,
    (std::string, uuid)
    (std::string, name)
    (std::string, synonum)
    (bool, predefined)
    (int, start_task)
    (int, end_task)
    (int, interval)
    (bool, allowed)
    (std::string, days_of_week)
    (ByteArray, script)
    (std::string, script_synonum)
    (std::string, comment)
    (ByteArray, param)
    (bool, reset_version)
    (std::string, cron_string)
);

typedef std::function<void(const arcirk::services::task_options&)> cron_callback;
namespace arcirk{

    tm current_date() {
        using namespace std;
        tm current{};
        time_t t = time(nullptr);
    #ifdef _WINDOWS
        localtime_s(&current, &t);
    #else
        localtime_r(&t, &current);
    #endif
        return current;
    }

    std::string to_utf(const std::string& source, const std::string& charset = "Windows-1251"){
    #ifdef _WINDOWS
        return boost::locale::conv::to_utf<char>(source, charset);
    #else
        return source;
    #endif
    }
    std::string from_utf(const std::string& source, const std::string& charset = "Windows-1251"){
    #ifdef BOOST_WINDOWS
        return boost::locale::conv::from_utf(source, charset);
    #else
        return source;
    #endif
    }

    static inline void log(const std::string& what, const std::string& message, bool conv = true, const std::string& log_folder = "logs"){
        const std::tm tm = arcirk::current_date();
        char cur_date[100];
        std::strftime(cur_date, sizeof(cur_date), "%c", &tm);

        std::string res = std::string(cur_date);
        res.append(" " + what + ": ");
        if(conv)
            res.append(arcirk::from_utf(message));
        else
            res.append(message);

        std::cout << res << std::endl;

        if(log_folder.empty())
            return;

        namespace fs = boost::filesystem;

        fs::path log_dir(log_folder);
        log_dir /= "days";
        if(!fs::exists(log_dir)){
            try {
                fs::create_directories(log_dir);
            } catch (const std::exception &e) {
                std::cerr << e.what() << std::endl;
                return;
            }
        }
        char date_string[100];
        strftime(date_string, sizeof(date_string), "%u_%m_%Y", &tm);

        fs::path file = log_dir / (std::string(date_string) + ".log");
        std::ofstream out;			// поток для записи
        out.open(file.string(), std::ios::app); 		// открываем файл для записи
        if (out.is_open())
        {
            out << res  << '\n';
        }
        out.close();
    };

    namespace uuids{

        inline bool is_valid_uuid(std::string const& maybe_uuid, boost::uuids::uuid& result) {
            using namespace boost::uuids;

            try {
                result = string_generator()(maybe_uuid);
                return result.version() != uuid::version_unknown;
            } catch(...) {
                return false;
            }
        }

        inline boost::uuids::uuid random_uuid(){
            return boost::uuids::random_generator()();
        }

        inline boost::uuids::uuid string_to_uuid(const std::string& sz_uuid, bool random_uuid = false) {

            if (sz_uuid == NIL_STRING_UUID && random_uuid)
                return boost::uuids::random_generator()();

            boost::uuids::uuid uuid{};

            if (is_valid_uuid(sz_uuid, uuid)){
                return uuid;
            }

            if (!random_uuid)
                return boost::uuids::nil_uuid();
            else
                return boost::uuids::random_generator()();
        }
    }
}
namespace arcirk::services{

class task_worker : boost::noncopyable
{
    std::mutex output_mutex;
public:

    task_worker(boost::asio::io_service& io
                , const task_options& opt
                , const cron_callback& callback_
                , std::shared_ptr<thread_pool>& workers)
        : ioService(io)
        , timer(io)
        , workers_(workers)
        , task_param(opt)
    {
        callback = callback_;
        name = opt.name;
        ioService.post(boost::bind(&task_worker::start, this));
    }

    void execute(boost::system::error_code const& e)
    {
        if (e != boost::asio::error::operation_aborted) {
            count++;
            callback(task_param);
            try
            {
                auto cron = cron::make_cron(task_param.cron_string);
                std::time_t now = std::time(0);
                std::time_t next = cron::cron_next(cron, now);
                std::cout << ctime(&next);
                timer.expires_at(boost::posix_time::from_time_t(next));
                start_wait();
            }
            catch (cron::bad_cronexpr const & ex)
            {
                std::cerr << ex.what() << '\n';
            }

        }
    }

    void start()
    {

        //std::cout << __FUNCTION__ << std::endl;

        workers_->wait();
        try
        {
            auto cron = cron::make_cron(task_param.cron_string);
            std::time_t now = std::time(0);
            std::time_t next = cron::cron_next(cron, now);
            timer.expires_at(boost::posix_time::from_time_t(next));
            start_wait();
        }
        catch (cron::bad_cronexpr const & ex)
        {
            std::cerr << ex.what() << '\n';
        }
    }

    void start_now(const int custom_interval = 1)
    {
        workers_->wait();
        timer.expires_from_now(boost::posix_time::seconds(custom_interval));
        start_wait();
    }

    void stop(){
        timer.cancel();
    };

private:
    void start_wait()
    {
        timer.async_wait(boost::bind(&task_worker::execute
                                     , this
                                     , boost::asio::placeholders::error));
    }

private:
    boost::asio::io_service& ioService;
    boost::asio::deadline_timer timer;
    cron_callback callback;
    std::string name;
    std::shared_ptr<thread_pool>& workers_;
    const task_options& task_param;
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
            arcirk::log(__FUNCTION__,"service already started");
            return;
        }
        arcirk::log(__FUNCTION__,"run all tasks");
        is_started_ = true;
        io_service.run();
    }

    void stop(){
        arcirk::log(__FUNCTION__,"stop all tasks");
        is_started_ = false;
        io_service.stop();
    }

    void clear(){
        if(is_started_)
        {
            arcirk::log(__FUNCTION__,"service already started");
            return;
        }
        workers_.reset();
        tasks.clear();
    }

    void stop_task(const boost::uuids::uuid& uuid){
        auto itr = tasks.find(uuid);
        if(itr != tasks.end())
            itr->second->stop();
    }

    void start_task(const boost::uuids::uuid& uuid){
        auto itr = tasks.find(uuid);
        if(itr != tasks.end())
            itr->second->start();
    }

    void start_task_now(const boost::uuids::uuid& uuid, int custom_interval){
        auto itr = tasks.find(uuid);
        if(itr != tasks.end())
            itr->second->start_now(custom_interval);
    }

    void add_task(const task_options& opt
                  , cron_callback const& callback)
    {
        tasks.insert(std::pair<boost::uuids::uuid, std::unique_ptr<task_worker>>(arcirk::uuids::string_to_uuid(opt.uuid), std::make_unique<task_worker>(std::ref(io_service)
                                                                                                                                                        , opt, callback, workers_)));
    }

    bool is_started(){
        return is_started_;
    }
private:
    bool is_started_ = false;
    boost::asio::io_service io_service;
    std::map<boost::uuids::uuid, std::unique_ptr<task_worker>> tasks;
    std::size_t threads;
    std::size_t capacity;
};

}


#endif // TASK_WORKER_HPP
