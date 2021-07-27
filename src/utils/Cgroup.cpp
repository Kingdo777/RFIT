//
// Created by kingdo on 2021/7/21.
//

#include "utils/Cgroup.h"
#include <utils/config.h>
#include <utils/logging.h>
#include <utils/timing.h>
#include <mutex>
#include <boost/filesystem.hpp>
#include <utility>
#include <syscall.h>

using namespace boost::filesystem;

namespace RFIT_NS::isolation {
    static const std::vector<std::string> controllers = {CG_CPU};

    static std::mutex groupMutex;

    CGroup::CGroup(std::string name)
            : name(std::move(name)) {
        RFIT_NS::utils::SystemConfig &conf = RFIT_NS::utils::getSystemConfig();
    }

    std::string CGroup::getName() {
        return this->name;
    }

    pid_t getCurrentTid() {
        auto tid = (pid_t) syscall(SYS_gettid);
        return tid;
    }

    void addCurrentThreadToTasks(const path &tasksPath) {
        const std::shared_ptr<spdlog::logger> &logger = RFIT_NS::utils::getLogger();

        pid_t threadId = getCurrentTid();

        std::ofstream outfile;
        outfile.open(tasksPath.string(), std::ios_base::app);
        outfile << threadId << std::endl;
        outfile.flush();

        logger->debug("Added thread id {} to {}", threadId, tasksPath.string());
    }

    void CGroup::addCurrentThread() {
        const std::shared_ptr<spdlog::logger> &logger = RFIT_NS::utils::getLogger();

        PROF_START(cGroupAdd)
        // Get lock and add to controllers
        std::scoped_lock<std::mutex> guard(groupMutex);

        for (const std::string &controller : controllers) {
            path tasksPath(BASE_DIR);
            tasksPath.append(controller);
            tasksPath.append(this->name);
            tasksPath.append("tasks");

            addCurrentThreadToTasks(tasksPath);
        }
        PROF_END(cGroupAdd)
    }
}
