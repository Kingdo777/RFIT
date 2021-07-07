#pragma once

#include <string>

namespace RFIT::utils {
    class SystemConfig {

    public:
        std::string logLevel;
        std::string logFile;

        int listenPort = 8080;
        int reactorThreadCount = 1;
        int workerThreadCount = 1;

        int overrideCpuCount = 0;

        SystemConfig();

        void print() const;

        void reset();

    private:
        void initialise();
    };

    SystemConfig &getSystemConfig();
}