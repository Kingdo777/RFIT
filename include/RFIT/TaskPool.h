//
// Created by kingdo on 2021/7/8.
//

#ifndef RFIT_TASK_POOL_H
#define RFIT_TASK_POOL_H

#include <atomic>
#include <thread>
#include <vector>
#include <string>
#include <utils/lru.h>
#include <RFIT/core.h>

using namespace std;
namespace RFIT_NS {
    class TaskPool {
    public:

        void shutdown();

        void dispatch(T::InvokeEntry &&invokeEntry);

    private:

        LRU<shared_ptr<R>, uint64_t> R_list;

    private:
        static shared_ptr<T> newT() {
            auto t = make_shared<T>();
            return t;
        }
    };
}


#endif //RFIT_TASK_POOL_H
