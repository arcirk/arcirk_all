#ifndef COMMON_HPP
#define COMMON_HPP

#include "arcirk.hpp"

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::tasks), task_options,
    (std::string, uuid)
    (std::string, name)
    (std::string, synonum)
    (bool, predefined)
    (int, start_task)
    (int, end_task)
    (int, interval)
    (int, type_script)
    (bool, allowed)
    (std::string, days_of_week)
    (ByteArray, script)
    (std::string, script_synonum)
    (std::string, comment)
    (ByteArray, param)
    (bool, reset_version)
    (std::string, cron_string)
)


#endif // COMMON_HPP
