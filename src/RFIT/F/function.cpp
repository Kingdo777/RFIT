//
// Created by kingdo on 10/17/20.
//

#include <RFIT/core.h>

#include <utility>
#include <WAVM/Runtime/Runtime.h>


namespace RFIT_NS {
    F::F(string funcName_,
         shared_ptr<R> r_,
         uint32_t concurrency) :
            funcName(std::move(funcName_)),
            r(std::move(r_)),
            concurrency(concurrency),
            TList(concurrency) {
    }

    void F::invoke(Message &msg) {
        auto func = (FuncType) (native.dr.addr);
        msg.set_isping(false);
        func(msg);
    }

    bool F::getIdleT(shared_ptr<T> &t) {
        return TList.takeIdleOne(t);
    }

    bool F::getT(shared_ptr<T> &t) {
        return TList.takeOne(t);
    }

    void F::newT(const shared_ptr<T> &t, bool take) {
        TList.newOne(t, take);
    }

    void F::shutdownAllT() {
        TList.shutdown();
        TList.flush();
    }

    std::vector<shared_ptr<T>> F::getAllT() {
        return TList.getSortedItem();
    }

}