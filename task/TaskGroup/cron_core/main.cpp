#include <iostream>
#include <memory>
#include <functional>
#include <boost/thread.hpp>
#include "include/task_worker.hpp"

#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>

using namespace std;

void exec_server_task(const arcirk::services::task_options &details){
    arcirk::log(__FUNCTION__ , details.name);
}

int main()
{
    setlocale(LC_ALL, "Russian");

//    auto task_manager = std::make_shared<arcirk::services::task_scheduler>();
//    auto one_task = arcirk::services::task_options();
//    one_task.uuid = boost::to_string(arcirk::uuids::random_uuid());
//    one_task.allowed = true;
//    one_task.name = "one_task !!!!!";
//    one_task.cron_string = "0 */5 */2 * * ?";
//    auto two_task = arcirk::services::task_options();
//    two_task.uuid = boost::to_string(arcirk::uuids::random_uuid());
//    two_task.allowed = true;
//    two_task.name = "two_task";
//    two_task.cron_string = "*/10 * * * * ?";
//
//    task_manager->add_task(one_task, std::bind(&exec_server_task, std::placeholders::_1));
//    task_manager->add_task(two_task, std::bind(&exec_server_task, std::placeholders::_1));
//
//    auto tr = boost::thread([&](){
//        task_manager->run();
//    });
//
//    tr.detach();
//
//    std::string line;
//    while (getline(std::cin, line)) {
//        if (line.empty()) {
//            continue;
//        }else if (line == "stop")
//        {
//            task_manager->stop();
//        }
//    }

    // Trivial logging: all log records are written into a file
    BOOST_LOG_TRIVIAL(trace) << "A trace severity message";
    BOOST_LOG_TRIVIAL(debug) << "A debug severity message";
    BOOST_LOG_TRIVIAL(info) << "An informational severity message";
    BOOST_LOG_TRIVIAL(warning) << "A warning severity message";
    BOOST_LOG_TRIVIAL(error) << "An error severity message";
    BOOST_LOG_TRIVIAL(fatal) << "A fatal severity message";

    // Filtering can also be applied
    using namespace boost::log;

    core::get()->set_filter
            (
                    trivial::severity >= trivial::info
            );

    // Now the first two lines will not pass the filter
    BOOST_LOG_TRIVIAL(trace) << "A trace severity message";
    BOOST_LOG_TRIVIAL(debug) << "A debug severity message";
    BOOST_LOG_TRIVIAL(info) << "An informational severity message";
    BOOST_LOG_TRIVIAL(warning) << "A warning severity message";
    BOOST_LOG_TRIVIAL(error) << "An error severity message";
    BOOST_LOG_TRIVIAL(fatal) << "A fatal severity message";

    return 0;
}
