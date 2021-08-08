//
// Created by kingdo on 2021/7/8.
//

#ifndef RFIT_RFIT_H
#define RFIT_RFIT_H

#include <WAVM/IR/Module.h>
#include <WAVM/Runtime/Runtime.h>
#include <utility>
#include "TaskPool.h"
#include "utils/config.h"
#include "utils/dl.h"
#include "utils/logging.h"
#include "utils/json.h"


#define RFIT_GLOBAL RFIT_NS::getRFIT()

namespace RFIT_NS {

    class RFIT {
    public:

        explicit RFIT();

        RFIT(RFIT &) = delete;

        RFIT &operator=(RFIT &) = delete;

        RFIT(RFIT &&) = delete;

        RFIT &operator=(RFIT &&) = delete;

        void shutdown();

        Pistache::Async::Promise<void>
        handlerNewFuncRegister(FunctionRegisterMsg &&msg_, Pistache::Http::ResponseWriter &&response_);

        Pistache::Async::Promise<void>
        handlerFuncInvoke(Message &&msg_, Pistache::Http::ResponseWriter &&response_);

        struct FunctionRegisterEntry {
            FunctionRegisterEntry(Pistache::Async::Deferred<void> deferred_,
                                  FunctionRegisterMsg msg_,
                                  Pistache::Http::ResponseWriter response_) :
                    deferred(move(deferred_)),
                    msg(std::move(msg_)),
                    response(std::move(response_)) {}

            Pistache::Async::Deferred<void> deferred;
            FunctionRegisterMsg msg;
            Pistache::Http::ResponseWriter response;
        public:
            bool isWasm() const {
                return msg.type() == "wasm";
            }
        };

        Pistache::Polling::Epoll poller;

        Pistache::PollableQueue<FunctionRegisterEntry> funcRegisterQueue;

        std::unordered_map<uint64_t, std::shared_ptr<R>> RMap;
        std::unordered_map<string, std::shared_ptr<F>> FMap;

        utils::SystemConfig &config;

        thread rfitThread;
        bool running = false;

        Pistache::NotifyFd shutdownFd;

        TaskPool tp;

    private:
        void run();

        shared_ptr<R> createR(Resource r);

        void registerF(FunctionRegisterMsg &msg, dlResult &dl, const boost::filesystem::path &p);

        void registerF(FunctionRegisterMsg &msg, WAVM::Runtime::ModuleRef &module);

        void handleNativeFuncRegister(const shared_ptr<FunctionRegisterEntry> &func);

        void handleWasmFuncRegister(const shared_ptr<FunctionRegisterEntry> &func);

        void handlerFuncRegisterQueue();

        shared_ptr<F> getF(const string &funcName);

        bool existF(const string &funcName);
    };

    RFIT &getRFIT();
}

#endif //RFIT_RFIT_H
