//
// Created by kingdo on 2021/7/8.
//

#include "utils/lru.h"
#include "RFIT/TaskPool.h"

namespace RFIT_NS {

    void TaskPool::shutdown() {
        // TODO
    }

    void TaskPool::dispatch(T::InvokeEntry &&invokeEntry) {
        auto r = invokeEntry.instance->getR();
        auto f = invokeEntry.instance->getF();
        R_list.access(r, r->getHash());
        r->getFList().access(f, f->getFuncName());
        shared_ptr<T> t;
        if (!f->getT(t)) {
            // TODO ，此处需要从LRU中寻找空闲的T
            t = newT();
            f->newT(t);
        }
        t->IQueue.push(std::move(invokeEntry));
    }
}