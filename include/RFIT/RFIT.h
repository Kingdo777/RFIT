//
// Created by kingdo on 2021/7/8.
//

#ifndef RFIT_RFIT_H
#define RFIT_RFIT_H

#include <pistache/mailbox.h>
#include <pistache/async.h>
#include <pistache/os.h>
#include <utility>
#include <boost/filesystem.hpp>
#include "RFIT/TaskPool/TaskPool.h"
#include "RFIT/R/resource.h"
#include "RFIT/F/function.h"
#include "RFIT/I/instance.h"
#include "RFIT/T/task.h"
#include "utils/config.h"

#define FUNC_PATH "/home/kingdo/CLionProjects/RFIT/Function/lib"

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

        Pistache::Async::Promise<FunctionRegisterResponseMsg> handlerNewFuncRegister(FunctionRegisterMsg &&msg_);

    private:
        struct FunctionRegisterEntry {
            FunctionRegisterEntry() = delete;

            FunctionRegisterEntry(Pistache::Async::Deferred<FunctionRegisterResponseMsg> deferred_,
                                  FunctionRegisterMsg msg_) :
                    deferred(move(deferred_)),
                    msg(std::move(msg_)) {}

            Pistache::Async::Deferred<FunctionRegisterResponseMsg> deferred;
            FunctionRegisterMsg msg;
        };

        Pistache::Polling::Epoll poller;

        Pistache::PollableQueue<FunctionRegisterEntry> funcRegisterQueue;

        std::unordered_map<uint64_t, std::shared_ptr<R>> RMap;
        std::unordered_map<string, std::shared_ptr<F>> FMap;

        std::unordered_map<uint64_t, std::shared_ptr<Pistache::PollableQueue<I>>> RIQueue;
        std::shared_mutex RIQueueLock;

        utils::SystemConfig &config;

        thread rfitThread;
        bool running = false;

        Pistache::NotifyFd shutdownFd;

    private:
        void run();

        shared_ptr<R> createR(Resource r);

        void handlerFuncRegisterQueue();
    };

    RFIT &getRFIT();
}

#endif //RFIT_RFIT_H
