//
// Created by kingdo on 2020/10/23.
//

#include <RFIT/core.h>


namespace RFIT_NS {
    I::I(Message msg_, shared_ptr<R> r_, shared_ptr<F> f_) :
            msg(std::move(msg_)),
            r(std::move(r_)),
            f(std::move(f_)) {
        module = f->getModule();
    }

    const Message &I::getMsg() const {
        return msg;
    }

    const shared_ptr<R> &I::getR() const {
        return r;
    }

    const shared_ptr<F> &I::getF() const {
        return f;
    }

    void I::invoke() {
        assert(f->isWasm());
        bool success;
        success = module.execute(msg);
        if (!success) {
            module.printDebugInfo();
            default_logger->error("Execution failed");
        }
        if (!msg.outputdata().empty()) {
            default_logger->info("Output: {}", msg.outputdata());
        }
    }
}