//
// Created by kingdo on 2020/10/23.
//

#include <RFIT/core.h>


namespace RFIT_NS {
    I::I(Message msg_, shared_ptr<R> r_, shared_ptr<F> f_) :
            msg(std::move(msg_)),
            r(std::move(r_)),
            f(std::move(f_)) {}

    const Message &I::getMsg() const {
        return msg;
    }

    const shared_ptr<R> &I::getR() const {
        return r;
    }

    const shared_ptr<F> &I::getF() const {
        return f;
    }
}