//
// Created by kingdo on 2021/7/7.
//

#include "endpoint/RFITEndpointHandler.h"
#include "utils//json.h"

namespace RFIT_NS::endpoint {
    void
    RFITEndpointHandler::onRequest(const Pistache::Http::Request &request, Pistache::Http::ResponseWriter response) {
        if (request.method() == Http::Method::Get) {
            response.send(Http::Code::Bad_Request).then(
                    [](ssize_t bytes) { std::cout << bytes << " bytes have been sent\n"; },
                    Async::NoExcept
            );
        } else if (request.method() == Http::Method::Put) {
            FunctionRegisterMsg msg;
            if (requestToFunctionRegisterMsg(request, msg) == -1)
                response.send(Http::Code::Bad_Request,
                              "Request Format Wrong，Must Be：/register/functionName/concurrency/core/mem");
            rfit.handlerNewFuncRegister(std::move(msg)).then(
                    [&](const FunctionRegisterResponseMsg &m) {
                        if (m.status()) {
                            response.send(Http::Code::Ok, messageToJson(m));
                        } else {
                            response.send(Http::Code::Bad_Request, "Register failed:" + messageToJson(m));
                        }
                    },
                    [&](exception_ptr &e) {
                        try {
                            rethrow_exception(e);
                        } catch (string &wrongInfo) {
                            response.send(Http::Code::Bad_Request, "Register Wrong: " + wrongInfo);
                        }
                    }
            );
        } else {
            dispatchRequest<void>(request, response);
        }
    }

    void
    RFITEndpointHandler::onTimeout(const Pistache::Http::Request &request, Pistache::Http::ResponseWriter response) {
        response
                .send(Http::Code::Request_Timeout, "Timeout")
                .then([=](ssize_t) {}, PrintException());
    }

    template<typename T>
    Async::Promise<T> RFITEndpointHandler::dispatchRequest(const Pistache::Http::Request &request,
                                                           Pistache::Http::ResponseWriter &response) {
        return Async::Promise<T>([&](Async::Deferred<T> deferred) {
            std::thread([]() {

            }).detach();
        });
    }

    int
    RFITEndpointHandler::requestToFunctionRegisterMsg(const Http::Request &request, FunctionRegisterMsg &msg) const {
        // 标准格式 ： /register/functionName/concurrency/core/mem
        // concurrency 正整数 1表示不并发，大于1表示并发度
        // core 正整数 表示对CPU占用的百分比(可以大于100%)
        // mem  正整数 是16的正倍数，表示内存占用
        // CPU/MEM 都是硬限制
        vector<string> para;
        utils::split(request.resource(), para, "/");
        if (!((para.size() == 2 || para.size() == 3 || para.size() == 5) && para[0] != "/register"))
            return -1;
        msg.set_funcname(para[1]);
        msg.set_concurrency(1);
        msg.set_coreration(DEFAULT_CPU_RATE);
        msg.set_memsize(MEM_DEFAULT_HARD_LIMIT);
        msg.set_dldata(request.body());
        if (para.size() > 2) {
            uint32_t concurrency = strtoul(para[2].data(), nullptr, 10);
            concurrency = concurrency > config.maxFuncConcurrency ? config.maxFuncConcurrency : concurrency;
            msg.set_concurrency(concurrency == 0 ? 1 : concurrency);
        }
        if (para.size() > 3) {
            double core = strtod(para[3].data(), nullptr);
            if (core > 0) {
                core = core > getAllCores() ? getAllCores() : core;
                msg.set_coreration(core);
            }
            uint32_t mem = strtoul(para[4].data(), nullptr, 10);
            if (mem > 0) {
                mem += 16;
                mem %= config.memAllocGranularity;
                msg.set_memsize(mem);
            }
        }
        return 0;
    }

}