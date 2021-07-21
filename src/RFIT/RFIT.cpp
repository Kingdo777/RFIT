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
            for (;;) {
                vector<Pistache::Polling::Event> events;
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
    RFIT::registerF(FunctionRegisterMsg &msg, dlResult &dl, const boost::filesystem::path &p) {
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
    makeFuncRegisterResponseMsg(const FunctionRegisterMsg &msg,
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

    void sendResponse(const FunctionRegisterResponseMsg &m, const shared_ptr<RFIT::FunctionRegisterEntry> &func) {
        if (m.status()) {
            func->response.send(Pistache::Http::Code::Ok, "OK");
        } else {
            func->response.send(Pistache::Http::Code::Bad_Request, "Register Failed:" + m.message());
        }
        func->deferred.resolve();
    }

    void RFIT::handlerFuncRegisterQueue() {
        for (;;) {
            shared_ptr<FunctionRegisterEntry> func = funcRegisterQueue.popSafe();
            if (!func)
                break;

            if (existF(func->msg.funcname())) {
                func->response.send(Pistache::Http::Code::Bad_Request,
                                    "Register Failed:" + func->msg.funcname() + " is exist!");
                func->deferred.resolve();
                break;
            }
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
                    makeFuncRegisterResponseMsg(func->msg, msg, false, message);
                } else {
                    auto info = registerF(func->msg, res.first, dlPath);
                    makeFuncRegisterResponseMsg(func->msg, msg, info.first, info.second);
                }
                sendResponse(msg, func);
            } catch (exception &e) {
                string s;
                if (e.what() != nullptr)
                    s = e.what();
                func->response.send(Pistache::Http::Code::Bad_Request,
                                    "Register Wrong: " + s);
                func->deferred.reject(s);
            }
        }
    }

    Pistache::Async::Promise<void>
    RFIT::handlerNewFuncRegister(FunctionRegisterMsg &&msg_, Pistache::Http::ResponseWriter &&response_) {
        return Pistache::Async::Promise<void>(
                [&](Pistache::Async::Deferred<void> deferred) {
                    FunctionRegisterEntry func(std::move(deferred), std::move(msg_), std::move(response_));
                    this->funcRegisterQueue.push(std::move(func));
                });
    }

    Pistache::Async::Promise<void> RFIT::handlerFuncInvoke(Message &msg) {
        default_logger->info("handlerFuncInvoke" + utils::messageToJson(msg));
        return Pistache::Async::Promise<void>([&](Pistache::Async::Deferred<void> deferred) {
            const std::string &funcName = msg.funcname();
            auto f = getF(funcName);
            if (f) {
                f->invoke(msg);
                deferred.resolve();
            } else {
                deferred.reject("No Function Found For " + funcName);
            }
        });
    }

    shared_ptr<F> RFIT::getF(const string &funcName) {
        auto it = FMap.find(funcName);
        if (it == FMap.end())
            return nullptr;
        return it->second;
    }

    bool RFIT::existF(const string &funcName) {
        return FMap.find(funcName) != FMap.end();
    }


    RFIT &getRFIT() {
        static RFIT rfit;
        return rfit;
    }

}