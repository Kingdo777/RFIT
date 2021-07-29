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

        struct RFT_LIST {
            std::vector<shared_ptr<R>> r;
            std::vector<std::vector<shared_ptr<F>>> f;
            std::vector<std::vector<std::vector<shared_ptr<T>>>> t;
        };

        RFT_LIST getRFT();

    private:

        LRU<shared_ptr<R>, uint64_t> R_list;
        std::mutex mutex;

    private:

        static bool getTFromOtherFInSameR(const shared_ptr<F> &selfF, shared_ptr<T> &t);

        bool getTFromOtherR(const shared_ptr<R> &selfR, shared_ptr<T> &t);

        static shared_ptr<T> newT() {
            auto t = make_shared<T>();
            return t;
        }
    };
}


#endif //RFIT_TASK_POOL_H
