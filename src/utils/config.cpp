#include <utils/config.h>
#include <utils/logging.h>
#include <utils/environment.h>

namespace RFIT_NS::utils {

    SystemConfig &getSystemConfig() {
        static SystemConfig conf;
        return conf;
    }

    SystemConfig::SystemConfig() {
        this->initialise();
    }

    void SystemConfig::initialise() {
        // System
        logLevel = getEnvVar("LOG_LEVEL", "info");
        logFile = getEnvVar("LOG_FILE", "off");

        listenPort = (uint32_t) strtoul(getEnvVar("LISTEN_PORT", "8080").c_str(), nullptr, 10);
        reactorThreadCount = (uint32_t) strtoul(getEnvVar("REACTOR_THREAD_COUNT", "1").c_str(), nullptr, 10);
        workerThreadCount = (uint32_t) strtoul(getEnvVar("WORKER_THREAD_COUNT", "1").c_str(), nullptr, 10);

        overrideCpuCount = (uint32_t) strtoul(getEnvVar("OVERRIDE_CPU_COUNT", "0").c_str(), nullptr, 10);

        maxFuncConcurrency = (uint32_t) strtoul(getEnvVar("MAX_FUNC_CONCURRENCY", "0").c_str(), nullptr, 10);
        memAllocGranularity = (uint32_t) strtoul(getEnvVar("MEM_ALLOC_GRANULARITY", "0").c_str(), nullptr, 10);

        entrySuffix = getEnvVar("ENTRY_SUFFIX", "_mainEntry");;
    }

    void SystemConfig::reset() {
        this->initialise();
    }

    void SystemConfig::print() const {
        const std::shared_ptr<spdlog::logger> &logger = getLogger();
        logger->info("--------------- System ---------------");
        logger->info("LOG_LEVEL                       {}", logLevel);
        logger->info("LOG_FILE                        {}", logFile);
        logger->info("listenPort                      {}", listenPort);
        logger->info("reactorThreadCount              {}", reactorThreadCount);
        logger->info("workerThreadCount               {}", workerThreadCount);
        logger->info("overrideCpuCount                {}", overrideCpuCount);
        logger->info("maxFuncConcurrency              {}", maxFuncConcurrency);
        logger->info("memAllocGranularity             {}", memAllocGranularity);
        logger->info("entrySuffix                     {}", entrySuffix);
        logger->info("--------------- Config ---------------");
    }
}