#pragma once

#include <utils/clock.h>
#include <string>


#ifdef OPEN_TRACE
#define PROF_START(name) const RFIT::utils::TimePoint time_point_##name = RFIT::utils::startTimer();
#define PROF_END(name) RFIT::utils::logEndTimer(#name, time_point_##name);
#define PROF_SUMMARY RFIT::utils::printTimerTotals();
#else
#define PROF_BEGIN
#define PROF_START(name)
#define PROF_END(name)
#define PROF_SUMMARY
#endif
namespace RFIT_NS::utils {

    TimePoint startTimer();

    void logEndTimer(const std::string &label,
                     const TimePoint &begin);

    void printTimerTotals();
}