#pragma once

#include <string>

#define systemConfig  RFIT_NS::utils::getSystemConfig()

namespace RFIT_NS::utils {
    class SystemConfig {

    public:
        std::string logLevel;
        std::string logFile;
        std::string captureStdout;

        uint32_t listenPort = 8080;
        uint32_t reactorThreadCount = 12;
        uint32_t workerThreadCount = 1;

        uint32_t overrideCpuCount = 0;

        uint32_t maxFuncConcurrency = 100;

        uint32_t memAllocGranularity = 16;

        std::string entrySuffix = "_mainEntry";

        // Filesystem storage
        std::string functionDir;
        std::string objectFileDir;
        std::string runtimeFilesDir;
        std::string sharedFilesDir;
        std::string sharedFilesStorageDir;

        SystemConfig();

        void print() const;

        void reset();

    private:
        void initialise();
    };

    SystemConfig &getSystemConfig();
}