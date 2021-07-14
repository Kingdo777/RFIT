//
// Created by kingdo on 2021/7/8.
//

#include "RFIT/TaskPool/TaskPool.h"
namespace RFIT_NS {

    bool TaskPool::isShutdown()
    {
        return _shutdown;
    }

    void TaskPool::shutdown() {
        _shutdown = true;
    }
}