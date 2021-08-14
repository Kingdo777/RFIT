#include <utils/logging.h>
#include <utils/timing.h>
#include <string>

using namespace std;
using namespace RFIT_NS::utils;
namespace RFIT_NS::utils {
    std::unordered_map<std::string, std::atomic<long>> timerTotals;
    std::unordered_map<std::string, std::atomic<int>> timerCounts;

    TimePoint startTimer() {
        return Clock::now();
    }

    double getTimeDiffMillis(const TimePoint &begin) {
        long micros = Clock::timeDiffMicro(Clock::now(), begin);
        return ((double) micros) / 1000;
    }

    void logEndTimer(const std::string &label,
                     const TimePoint &begin) {
        double millis = getTimeDiffMillis(begin);
        const std::shared_ptr<spdlog::logger> &logger = getLogger("time statistics");
        logger->trace("TIME = {:.3f}ms ({})", millis, label);

        // Record microseconds total
        timerTotals[label] += long(millis * 1000L);
        timerCounts[label]++;
    }


    void printTimerTotals() {
        // Switch the pairs so we can use std::sort
        std::vector<std::pair<long, std::string>> totals;
        totals.reserve(timerTotals.size());
        for (auto &p : timerTotals) {
            totals.emplace_back(p.second, p.first);
        }

        std::sort(totals.begin(), totals.end());

        printf("---------- TIMER TOTALS ----------\n");
        printf("Total (ms)  Avg (ms)   Count  Label\n");
        for (auto &p : totals) {
            double millis = double(p.first) / 1000.0;
            int count = timerCounts[p.second];
            double avg = millis / count;
            printf(
                    "%-11.2f %-10.3f %5i  %s\n", millis, avg, count, p.second.c_str());
        }
    }

    uint64_t timespecToNanos(struct timespec *nativeTimespec) {
        uint64_t nanos = nativeTimespec->tv_sec * 1000000000;
        nanos += nativeTimespec->tv_nsec;

        return nanos;
    }

    void nanosToTimespec(uint64_t nanos, struct timespec *nativeTimespec) {
        nativeTimespec->tv_sec = nanos / 1000000000;
        nativeTimespec->tv_nsec = nanos % 1000000000;
    }
}