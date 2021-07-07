#include <utils/config.h>
#include <utils/logging.h>
#include <utils/environment.h>

namespace RFIT::utils {

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

        listenPort = (int) strtol(getEnvVar("LISTEN_PORT", "8080").c_str(), nullptr, 10);
        reactorThreadCount = (int) strtol(getEnvVar("REACTOR_THREAD_COUNT", "1").c_str(), nullptr, 10);
        workerThreadCount = (int) strtol(getEnvVar("WORKER_THREAD_COUNT", "1").c_str(), nullptr, 10);

        overrideCpuCount=(int) strtol(getEnvVar("OVERRIDE_CPU_COUNT", "0").c_str(), nullptr, 10);
    }

    void SystemConfig::reset() {
        this->initialise();
    }

    void SystemConfig::print() const {
        const std::shared_ptr<spdlog::logger> &logger = getLogger();
        logger->info("--- System ---");
        logger->info("LOG_LEVEL                  {}", logLevel);
        logger->info("LOG_FILE                   {}", logFile);
    }
}