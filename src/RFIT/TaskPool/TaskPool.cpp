//
// Created by kingdo on 2021/7/8.
//

#include "utils/lru.h"
#include "utils/locks.h"
#include "RFIT/TaskPool.h"

namespace RFIT_NS {

    void TaskPool::shutdown() {
        utils::UniqueLock lock(mutex);
        auto Rs = R_list.getSortedItem();
        for (const auto &r:Rs) {
            auto Fs = r->getFList().getSortedItem();
            for (const auto &f:Fs) {
                f->shutdownAllT();
            }
        }
    }

    void TaskPool::dispatch(T::InvokeEntry &&invokeEntry) {
        auto r = invokeEntry.instance->getR();
        auto f = invokeEntry.instance->getF();
        R_list.access(r, r->getHash());
        r->getFList().access(f, f->getFuncName());
        shared_ptr<T> t;
        /// 这里上了一把锁
        utils::UniqueLock lock(mutex);
        if (!f->getT(t)) {
            if (!getTFromOtherFInSameR(f, t))
                if (!getTFromOtherR(r, t))
                    t = newT();
            /// 如果是上述三种情况下拿到的T都要进行 f->newT(t)操作
            f->newT(t);
        }
        lock.unlock();
        t->IQueue.push(std::move(invokeEntry));
    }

    bool TaskPool::getTFromOtherFInSameR(const shared_ptr<F> &selfF, shared_ptr<T> &t) {
        auto r = selfF->getR();
        auto Fs = r->getFList().getSortedItem();
        for (const auto &f:Fs) {
            if (f == selfF)
                continue;
            if (f->getIdleT(t))
                return true;
        }
        return false;
    }

    bool TaskPool::getTFromOtherR(const shared_ptr<R> &selfR, shared_ptr<T> &t) {
        auto Rs = R_list.getSortedItem();
        for (const auto &r:Rs) {
            if (r == selfR)
                continue;
            auto Fs = r->getFList().getSortedItem();
            for (const auto &f:Fs) {
                if (f->getIdleT(t))
                    return true;
            }
        }
        return false;
    }
}