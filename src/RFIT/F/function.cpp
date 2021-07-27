//
// Created by kingdo on 10/17/20.
//

#include <RFIT/core.h>


namespace RFIT_NS {
    F::F(string &funcName_, shared_ptr<R> r_, utils::dlResult dr_, boost::filesystem::path p,
         uint32_t concurrency) :
            funcName(funcName_),
            dr(dr_),
            dlPath(std::move(p)),
            r(std::move(r_)),
            concurrency(concurrency) {
    }

    void F::invoke(Message &msg) {
        auto func = (FuncType) (dr.addr);
        msg.set_isping(false);
        func(msg);
    }

    bool F::getT(shared_ptr<T> &t) {
        return TList.takeOne(t, concurrency);
    }

    void F::newT(const shared_ptr<T> &t, bool take) {
        TList.newOne(t, take);
    }

}