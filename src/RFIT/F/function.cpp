//
// Created by kingdo on 10/17/20.
//

#include "RFIT/F/function.h"
#include "utils/logging.h"

namespace RFIT_NS {
    F::F(const string &funcName_, shared_ptr<R> r_, utils::dlResult dr_, boost::filesystem::path p,
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
}