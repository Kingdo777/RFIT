//
// Created by kingdo on 2021/7/21.
//

#include "RFIT/core.h"
#include <utils/config.h>
#include <utils/logging.h>
#include <utils/timing.h>
#include <mutex>
#include <boost/filesystem.hpp>
#include <utility>
#include <syscall.h>

using namespace boost::filesystem;

namespace RFIT_NS {
    static const std::vector<std::string> controllers = {CG_CPU, CG_MEM};
    static std::mutex groupMutex;


    pid_t getCurrentTid() {
        auto tid = (pid_t) syscall(SYS_gettid);
        return tid;
    }

    void addCurrentThreadToTasks(const path &tasksPath) {
        pid_t threadId = getCurrentTid();
        std::ofstream outfile;
        outfile.open(tasksPath.string(), std::ios_base::app);
        outfile << threadId << std::endl;
        outfile.flush();
        default_logger->debug("Added thread id {} to {}", threadId, tasksPath.string());
    }

    void CGroup::addCurrentThread() {
        assert(!name.empty());
        PROF_START(cGroupAdd)
        // Get lock and add to controllers
        std::scoped_lock<std::mutex> guard(groupMutex);
        for (const std::string &controller : controllers) {
            path tasksPath(BASE_CGROUP_DIR);
            tasksPath.append(controller)
                    .append(BASE_CGROUP_NAME)
                    .append(this->name);
            if (!boost::filesystem::exists(tasksPath))
                boost::filesystem::create_directories(tasksPath);
            tasksPath.append("tasks");
            addCurrentThreadToTasks(tasksPath);
        }
        PROF_END(cGroupAdd)
    }

    void writeConfig(int64_t config, const path &confFilePath) {
        std::ofstream outfile;
        outfile.open(confFilePath, std::ios_base::out);
        outfile << to_string(config);
        outfile.flush();
    }

    void writeConfig(uint64_t config, const path &confFilePath) {
        std::ofstream outfile;
        outfile.open(confFilePath, std::ios_base::out);
        outfile << to_string(config);
        outfile.flush();
    }

    void CGroup::changeMemCGConfig(const MemResource &res, path &confPath) {
        writeConfig(res.impl.mem_hard_limit, confPath.append("memory.limit_in_bytes"));
        writeConfig(res.impl.mem_soft_limit, confPath.parent_path().append("memory.soft_limit_in_bytes"));
    }

    void CGroup::changeCPUCGConfig(const CpuResource &res, path &confPath) {
        writeConfig(res.impl.cpuShares, confPath.append("cpu.shares"));
        writeConfig(res.impl.cfs_quota_us, confPath.parent_path().append("cpu.cfs_quota_us"));
        writeConfig(res.impl.cfs_period_us, confPath.parent_path().append("cpu.cfs_period_us"));
    }

    void CGroup::changeConfig(const Resource &res) {
        assert(!name.empty());
        PROF_START(cGroupChange)
        if (res.impl.mem.getHash() != mem_hash) {
            path confPath(BASE_CGROUP_DIR);
            mem_hash = res.impl.mem.getHash();
            confPath.append(CG_MEM)
                    .append(BASE_CGROUP_NAME)
                    .append(this->name);
            changeMemCGConfig(res.impl.mem, confPath);
        }
        if (res.impl.cpu.getHash() != cpu_hash) {
            path confPath(BASE_CGROUP_DIR);
            cpu_hash = res.impl.cpu.getHash();
            confPath.append(CG_CPU)
                    .append(BASE_CGROUP_NAME)
                    .append(this->name);
            changeCPUCGConfig(res.impl.cpu, confPath);
        }
        PROF_END(cGroupChange)
    }

    void CGroup::setName(std::string name_) {
        name = std::move(name_);
    }

    void CGroup::destroy() {
        std::scoped_lock<std::mutex> guard(groupMutex);
        for (const std::string &controller : controllers) {
            path p(BASE_CGROUP_DIR);
            p.append(controller)
                    .append(BASE_CGROUP_NAME)
                    .append(this->name);
            boost::filesystem::remove(p);
        }
    }
}
