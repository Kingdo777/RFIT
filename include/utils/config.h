#pragma once

#include <string>

#define systemConfig  RFIT_NS::utils::getSystemConfig()

namespace RFIT_NS::utils {
    class SystemConfig {

    public:
        std::string logLevel;
        std::string logFile;

        uint32_t listenPort = 8080;
        uint32_t reactorThreadCount = 1;
        uint32_t workerThreadCount = 1;

        uint32_t overrideCpuCount = 0;

        uint32_t maxFuncConcurrency = 100;

        uint32_t memAllocGranularity = 16;

        SystemConfig();

        void print() const;

        void reset();

    private:
        void initialise();
    };

    SystemConfig &getSystemConfig();
}