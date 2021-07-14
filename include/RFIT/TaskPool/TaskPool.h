//
// Created by kingdo on 2021/7/8.
//

#ifndef RFIT_TASK_POOL_H
#define RFIT_TASK_POOL_H

#include <atomic>
#include <thread>
#include <vector>

namespace RFIT_NS {
    class TaskPool {
    public:
        bool isShutdown();

        void shutdown();

    private:
        std::atomic<bool> _shutdown = false;

        // 用于创建县城池的派生线程
        std::thread poolThread;
        // 线程池
        std::vector<std::thread> poolThreads;

    };
}


#endif //RFIT_TASK_POOL_H
