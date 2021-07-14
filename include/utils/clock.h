#pragma once

#include <chrono>

namespace RFIT_NS::utils {
    typedef std::chrono::steady_clock::time_point TimePoint;

    class Clock {
    public:
        Clock() = default;

        static TimePoint now() {
            return std::chrono::steady_clock::now();
        }

        static long epochMillis() {
            long millis = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count();

            return millis;
        }

        static long timeDiff(const TimePoint &t1, const TimePoint &t2) {
            long age =
                    std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t2).count();
            return age;
        }

        static long timeDiffMicro(const TimePoint &t1, const TimePoint &t2) {
            long age =
                    std::chrono::duration_cast<std::chrono::microseconds>(t1 - t2).count();
            return age;
        }

        static long timeDiffNano(const TimePoint &t1, const TimePoint &t2) {
            long age =
                    std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t2).count();
            return age;
        }
    };
}