#include "btasks.h"
#include <iostream>

BTasks::BTasks()
{
    libcron::Cron cron;
    cron.add_schedule("Hello from Cron", "* * * * * ?", [=](auto&) {
        std::cout << "Hello from libcron!" << std::endl;
    });
}
