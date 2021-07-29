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
            r->getFList().flush();
        }
        R_list.flush();
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

    TaskPool::RFT_LIST TaskPool::getRFT() {
        RFT_LIST rftList;
        utils::UniqueLock lock(mutex);
        auto Rs = R_list.getSortedItem();
        int i_r = 0, i_f = 0;
        for (const auto &r:Rs) {
            rftList.r.push_back(r);
            rftList.f.emplace_back();
            rftList.t.emplace_back();
            auto Fs = r->getFList().getSortedItem();
            for (const auto &f:Fs) {
                rftList.f[i_r].push_back(f);
                rftList.t[i_r].emplace_back();
                auto Ts = f->getAllT();
                for (const auto &t:Ts) {
                    rftList.t[i_r][i_f].push_back(t);
                }
                i_f++;
            }
            i_r++;
        }
        return rftList;
    }
}