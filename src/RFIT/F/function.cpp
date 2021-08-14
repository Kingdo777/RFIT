//
// Created by kingdo on 10/17/20.
//

#include <RFIT/core.h>

#include <utility>
#include <WAVM/Runtime/Runtime.h>


namespace RFIT_NS {
    F::F(string user_,
         string funcName_,
         shared_ptr<R> r_,
         uint32_t concurrency) :
            user(std::move(user_)),
            funcName(std::move(funcName_)),
            r(std::move(r_)),
            concurrency(concurrency),
            TList(concurrency) {
    }

    F::F(string user_,
         string funcName_,
         shared_ptr<R> r_,
         utils::dlResult dr_,
         uint32_t concurrency) :
            user(std::move(user_)),
            funcName(std::move(funcName_)),
            native({dr_}),
            r(std::move(r_)),
            concurrency(concurrency),
            TList(concurrency) {
        isWasm_ = false;
    }

    F::F(string user_,
         string funcName_,
         shared_ptr<R> r_,
         const WAVM::Runtime::ModuleRef &module_,
         uint32_t concurrency
    ) :
            user(std::move(user_)),
            funcName(std::move(funcName_)),
            wasm({wasm::WAVMWasmModule(user, funcName, module_)}),
            r(std::move(r_)),
            concurrency(concurrency),
            TList(concurrency) {
        isWasm_ = true;
    }

    void F::invoke(Message &msg) {
        msg.set_isping(false);
        if (!isWasm_) {
            ((FuncType) (native.dr.addr))(msg);
        } else {
            wasm.module.execute(msg);
        }
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

    bool F::pingFunc() {
        Message m{};
        m.set_user(user);
        m.set_funcname(funcName);
        m.set_isping(true);
        if (!isWasm_) {
            ((FuncType) native.dr.addr)(m);
            return m.outputdata() == "PONG";
        } else {
            m.set_funcptr("_rfit_ping_func");
            return wasm.module.execute(m);
        }
    }
}