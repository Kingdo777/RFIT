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

    bool pingFunc(FuncType func) {
        Message m{};
        m.set_isping(true);
        func(m);
        return m.outputdata() == "PONG";
    }

    pair<bool, string>
    RFIT::registerF(FunctionRegisterResponseMsg &msg, dlResult &dl, const boost::filesystem::path &p) {
        if (!pingFunc((FuncType) dl.addr)) {
            close_remove_DL(p, dl.handle);
            return pair<bool, string>(false, "Ping Failed");
        }
        CpuResource cr(CPU_DEFAULT_SHARES,
                       (uint64_t) (msg.coreration() * CPU_DEFAULT_CFS_PERIOD_US),
                       CPU_DEFAULT_CFS_PERIOD_US);
        MemResource mr(msg.memsize(), msg.memsize());
        Resource resource(cr, mr);
        auto r = createR(resource);
        auto f = make_shared<F>(msg.funcname(), r, dl, p, msg.concurrency());
        FMap.emplace(f->getFuncName(), f);
        return pair<bool, string>(true, "OK");
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
                    const string &message = res.second.empty() ? (func->msg.funcname() + "_main" + "is NULL type")
                                                               : res.second;
                    makeResponseMsgFromRequest(func->msg, msg, false, message);
                } else {
                    auto info = registerF(msg, res.first, dlPath);
                    makeResponseMsgFromRequest(func->msg, msg, info.first, info.second);
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