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

    void
    makeResponseMsgFromRequest(const FunctionRegisterMsg &msg,
                               FunctionRegisterResponseMsg &responseMsg,
                               bool status = true,
                               const string &message = "OK") {
        responseMsg.set_funcname(msg.funcname());
        responseMsg.set_concurrency(msg.concurrency());
        responseMsg.set_coreration(msg.coreration());
        responseMsg.set_memsize(msg.memsize());
        responseMsg.set_status(status);
        responseMsg.set_message(message);
    }

    void RFIT::handlerFuncRegisterQueue() {
        for (;;) {
            auto func = funcRegisterQueue.popSafe();
            if (!func)
                break;
            FunctionRegisterResponseMsg msg;
            try {
                const string &dlPath = utils::makeDL(func->msg.funcname(),
                                                     func->msg.dldata().c_str(),
                                                     func->msg.dldata().size());
                pair<dlResult, std::string> res = utils::getFuncEntry(dlPath,
                                                                      func->msg.funcname() + config.entrySuffix);
                if (res.first.handle == nullptr || res.first.addr == nullptr || !res.second.empty()) {
                    if (res.second.empty())
                        makeResponseMsgFromRequest(func->msg, msg, false,
                                                   func->msg.funcname() + "_main" + "is NULL type");
                    else
                        makeResponseMsgFromRequest(func->msg, msg, false, res.second);
                    dlclose(res.first.handle);
                    boost::filesystem::remove(dlPath);
                } else {
                    auto function = (FuncType) res.first.addr;
                    Message m{};
                    m.set_isping(true);
                    function(m);
                    if (m.outputdata() == "PONG")
                        makeResponseMsgFromRequest(func->msg, msg, true, "OK");
                    else {
                        makeResponseMsgFromRequest(func->msg, msg, false, "Ping Failed");
                        dlclose(res.first.handle);
                        boost::filesystem::remove(dlPath);
                    }
                }
                func->deferred.resolve(std::move(msg));
            } catch (exception &e) {
                string s;
                if (e.what() != nullptr)
                    s = e.what();
                func->deferred.reject(s);
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