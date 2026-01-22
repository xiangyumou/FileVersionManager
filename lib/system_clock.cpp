/**
  ___ _                 _
 / __| |__   __ _ _ __ | |_    /\/\   ___  ___
/ /  | '_ \ / _` | '_ \| __|  /    \ / _ \/ _ \
/ /___| | | | (_| | | | | |_  / /\  |  __|  __/
\____/|_| |_|\__,_|_| |_|\__| \/  \/\___|\___|

@ Author: Mu Xiangyu, Chant Mee
*/

#ifndef SYSTEM_CLOCK_CPP
#define SYSTEM_CLOCK_CPP

#include "fvm/system_clock.h"
#include <ctime>
#include <cstdio>
#include <string>

namespace fvm {
namespace core {

std::string SystemClock::get_current_time(int timezone_offset_hours) const {
    char t[100];
    time_t timep;
    time(&timep);
    struct tm* p = gmtime(&timep);
    snprintf(t, sizeof(t), "%d-%02d-%02d %02d:%02d:%02d",
             1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday,
             timezone_offset_hours + p->tm_hour, p->tm_min, p->tm_sec);
    return std::string(t);
}

long SystemClock::get_current_time_raw() const {
    time_t timep;
    time(&timep);
    return static_cast<long>(timep);
}

} // namespace core
} // namespace fvm

#endif // SYSTEM_CLOCK_CPP
