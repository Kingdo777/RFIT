//
// Created by kingdo on 2021/7/8.
//

#include <pistache/http.h>
#include "RFIT/RFIT.h"

namespace RFIT_NS {

    RFIT::RFIT() :
            poller(),
            config(systemConfig) {
        funcRegisterQueue.bind(poller);
        createR(Resource());
        run();
    }

    void RFIT::run() {
        if (running)
            return;
        config.print();

        if (!shutdownFd.isBound())
            shutdownFd.bind(poller);

        rfitThread = thread([this]() {
            vector<Pistache::Polling::Event> events;
            for (;;) {
                int ready_fds = poller.poll(events);
                if (ready_fds == -1) {
                    throw runtime_error("Polling");
                }
                for (const auto &e:events) {
                    if (e.tag == shutdownFd.tag())
                        return;
                    else if (e.tag == funcRegisterQueue.tag()) {
                        handlerFuncRegisterQueue();
                    }
                }
            }
        });
    }

    void RFIT::shutdown() {
        if (shutdownFd.isBound())
            shutdownFd.notify();
        rfitThread.join();
    }

    shared_ptr<R> RFIT::createR(Resource res) {
        if (RMap.find(res.getHash()) == end(RMap)) {
            auto r = std::make_shared<R>(move(res));
            RMap.emplace(r->getHash(), r);
            WriteLock lock(RIQueueLock);
            RIQueue.emplace(r->getHash(), std::make_shared<Pistache::PollableQueue<I>>());
            RIQueue[r->getHash()]->bind(poller);
            return r;
        }
        return RMap[res.getHash()];
    }

    void makeResponseMsgFromRequest(const FunctionRegisterMsg &msg, FunctionRegisterResponseMsg &responseMsg) {
        responseMsg.set_funcname(msg.funcname());
        responseMsg.set_concurrency(msg.concurrency());
        responseMsg.set_coreration(msg.coreration());
        responseMsg.set_memsize(msg.memsize());
    }

    void RFIT::handlerFuncRegisterQueue() {
        for (;;) {
            auto func = funcRegisterQueue.popSafe();
            if (!func)
                break;
            boost::filesystem::path funPath(FUNC_PATH);
            funPath.append(func->msg.funcname());
            if (!boost::filesystem::exists(funPath))
                boost::filesystem::create_directories(funPath);
            string outputFile = funPath.append("function.so").string();
            try {
                std::ofstream out(outputFile, ios_base::out | ios_base::binary);
                out.write(func->msg.dldata().c_str(), func->msg.dldata().size()).flush();
                out.close();
                FunctionRegisterResponseMsg msg;
                makeResponseMsgFromRequest(func->msg, msg);
                msg.set_status(true);
                msg.set_message("OK");
                func->deferred.resolve(std::move(msg));
            } catch (exception &e) {
                func->deferred.reject(current_exception());
            }
        }
    }

    Pistache::Async::Promise<FunctionRegisterResponseMsg>
    RFIT::handlerNewFuncRegister(FunctionRegisterMsg &&msg_) {
        return Pistache::Async::Promise<FunctionRegisterResponseMsg>(
                [&](Pistache::Async::Deferred<FunctionRegisterResponseMsg> deferred) {
                    FunctionRegisterEntry func(std::move(deferred), msg_);
                    this->funcRegisterQueue.push(std::move(func));
                });
    }


    RFIT &getRFIT() {
        static RFIT rfit;
        return rfit;
    }
}