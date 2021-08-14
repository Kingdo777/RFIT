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
        captureStdout = getEnvVar("CAPTURE_STDOUT", "on");

        listenPort = (uint32_t) strtoul(getEnvVar("LISTEN_PORT", "8080").c_str(), nullptr, 10);
        reactorThreadCount = (uint32_t) strtoul(getEnvVar("REACTOR_THREAD_COUNT", "1").c_str(), nullptr, 10);
        workerThreadCount = (uint32_t) strtoul(getEnvVar("WORKER_THREAD_COUNT", "1").c_str(), nullptr, 10);

        overrideCpuCount = (uint32_t) strtoul(getEnvVar("OVERRIDE_CPU_COUNT", "0").c_str(), nullptr, 10);

        maxFuncConcurrency = (uint32_t) strtoul(getEnvVar("MAX_FUNC_CONCURRENCY", "1000").c_str(), nullptr, 10);
        memAllocGranularity = (uint32_t) strtoul(getEnvVar("MEM_ALLOC_GRANULARITY", "16").c_str(), nullptr, 10);

        entrySuffix = getEnvVar("ENTRY_SUFFIX", "_mainEntry");

        // Filesystem storage
        std::string faasmLocalDir =
                getEnvVar("FAASM_LOCAL_DIR", "/home/kingdo/CLionProjects/RFIT/Function/wasm");
        functionDir = fmt::format("{}/{}", faasmLocalDir, "function");
        objectFileDir = fmt::format("{}/{}", faasmLocalDir, "object");
        runtimeFilesDir = fmt::format("{}/{}", faasmLocalDir, "runtime_root");
        sharedFilesDir = fmt::format("{}/{}", faasmLocalDir, "shared");
        sharedFilesStorageDir = fmt::format("{}/{}", faasmLocalDir, "shared_store");
    }

    void SystemConfig::reset() {
        this->initialise();
    }

    void SystemConfig::print() const {
        const std::shared_ptr<spdlog::logger> &logger = getLogger();
        logger->info("--------------- System ---------------");
        logger->info("LOG_LEVEL                       {}", logLevel);
        logger->info("LOG_FILE                        {}", logFile);
        logger->info("CAPTURE_STDOUT                  {}", captureStdout);
        logger->info("listenPort                      {}", listenPort);
        logger->info("reactorThreadCount              {}", reactorThreadCount);
        logger->info("workerThreadCount               {}", workerThreadCount);
        logger->info("overrideCpuCount                {}", overrideCpuCount);
        logger->info("maxFuncConcurrency              {}", maxFuncConcurrency);
        logger->info("memAllocGranularity             {}", memAllocGranularity);
        logger->info("entrySuffix                     {}", entrySuffix);
        logger->info("-------------- Storage --------------");
        logger->info("FUNC_DIR                  {}", functionDir);
        logger->info("OBJ_DIR                   {}", objectFileDir);
        logger->info("RUNTIME_FILES_DIR         {}", runtimeFilesDir);
        logger->info("SHARED_FILES_DIR          {}", sharedFilesDir);
        logger->info("SHARED_FILES_STORAGE_DIR  {}", sharedFilesStorageDir);
        logger->info("--------------- Config ---------------");
    }
}