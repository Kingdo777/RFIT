//
// Created by kingdo on 2021/7/7.
//

#include "endpoint/RFITEndpointHandler.h"
#include "utils/json.h"
#include "utils/gids.h"

namespace RFIT_NS::endpoint {
    void
    RFITEndpointHandler::onRequest(const Pistache::Http::Request &request, Pistache::Http::ResponseWriter response) {
        if (request.method() == Http::Method::Put) {
            FunctionRegisterMsg msg;
            if (requestToMsg(request, msg) == -1) {
                response.send(Http::Code::Bad_Request,
                              "Request Format Wrong，Url Must Be：/register/functionName/concurrency/core/mem; Method Must Be Put ");
            } else {
                rfit.handlerNewFuncRegister(std::move(msg), std::move(response)).then(
                        [] {},
                        PrintException());
            }
        } else if (request.method() == Http::Method::Post) {
            Message msg;
            if (requestToMsg(request, msg) == -1) {
                response.send(Http::Code::Bad_Request,
                              "Request Format Wrong，Url Must Be：/invoke/funcName; Method Must Be POST ");
            } else {
                rfit.handlerFuncInvoke(std::move(msg), std::move(response)).then(
                        [] {},
                        PrintException());
            }
        } else if (request.method() == Http::Method::Get) {
            handleGetRequest(request, std::move(response));
        } else {
            response.send(Http::Code::Bad_Request).then(
                    [](ssize_t bytes) { std::cout << bytes << " bytes have been sent\n"; },
                    Async::NoExcept
            );
        }
    }

    void
    RFITEndpointHandler::onTimeout(const Pistache::Http::Request &request, Pistache::Http::ResponseWriter response) {
        response
                .send(Http::Code::Request_Timeout, "Timeout")
                .then([=](ssize_t) {}, PrintException());
    }

    enum RegisterPara {
        register_rp = 0,
        type_rp,
        user_rp,
        functionName_rp,
        concurrency_rp,
        core_rp,
        mem_rp,
        RegisterParaSize
    };

    int
    RFITEndpointHandler::requestToMsg(const Http::Request &request, FunctionRegisterMsg &msg) const {
        // 标准格式 ： /register/type/user/functionName/concurrency/core/mem
        // type 只能是native或者是wasm
        // concurrency 正整数 1表示不并发，大于1表示并发度
        // core 正整数 表示对CPU占用的百分比(可以大于100%)
        // mem  正整数 是16的正倍数，表示内存占用
        // CPU/MEM 都是硬限制
        vector<string> para;
        utils::split(request.resource(), para, "/");
        if (!((para.size() == 4 || para.size() == 5 || para.size() == 7) &&
              para[register_rp] == "register") &&
            (para[type_rp] == "native" || para[type_rp] == "wasm"))
            return -1;
        msg.set_type(para[type_rp]);
        msg.set_user(para[user_rp]);
        msg.set_funcname(std::move(para[functionName_rp]));
        msg.set_concurrency(1);
        msg.set_coreration(DEFAULT_CPU_RATE);
        msg.set_memsize(MEM_DEFAULT_HARD_LIMIT);
        msg.set_dldata(request.body());
        if (para.size() > 4) {
            uint32_t concurrency = strtoul(para[concurrency_rp].data(), nullptr, 10);
            concurrency = concurrency > config.maxFuncConcurrency ? config.maxFuncConcurrency : concurrency;
            msg.set_concurrency(concurrency == 0 ? 1 : concurrency);
        }
        if (para.size() > 5) {
            double core = strtod(para[core_rp].data(), nullptr);
            if (core > 0) {
                core = core > getAllCores() ? getAllCores() : core;
                msg.set_coreration(core);
            }
            uint32_t mem = strtoul(para[mem_rp].data(), nullptr, 10);
            if (mem > 0) {
                if (mem % 16)
                    mem += 16;
                mem /= config.memAllocGranularity;
                mem *= config.memAllocGranularity;
                msg.set_memsize(mem * 1024 * 1024);
            }
        }
        return 0;
    }

    enum InvokePara {
        invoke_ip = 0,
        user_ip,
        functionName_ip,
        InvokeParaSize
    };

    int
    RFITEndpointHandler::requestToMsg(const Http::Request &request, Message &msg) {
        vector<string> para;
        utils::split(request.resource(), para, "/");
        if (para.size() == InvokeParaSize && para[invoke_ip] != "invoke")
            return -1;
        msg.set_id(utils::generateGid());
        msg.set_user(para[user_ip]);
        msg.set_funcname(para[functionName_ip]);
        msg.set_timestamp(RFIT_NS::utils::Clock::epochMillis());
        msg.set_inputdata(request.body());
        msg.set_isping(false);
        return 0;
    }

    void RFITEndpointHandler::getRFTInfo(string &content) {
        content += "R-F-T List Info:\n";
        auto rftList = rfit.tp.getRFT();
        int r_index = 0;
        for (const auto &r : rftList.r) {
            content += r->toString();
            content += "\n";
            int f_index = 0;
            for (const auto &f:rftList.f[r_index]) {
                content += "\t";
                content += f->toString();
                content += "\n";
                int i = 1;
                for (const auto &t:rftList.t[r_index][f_index]) {
                    content += "\t\t";
                    content += (to_string(i++) + ". " + t->toString());
                    content += "\n";
                }
                f_index++;
            }
            r_index++;
        }
    }

    void RFITEndpointHandler::handleGetRequest(const Http::Request &request, Pistache::Http::ResponseWriter response) {
        assert(request.method() == Http::Method::Get);
        std::string content;
        if (request.resource() == "/rft/info") {
            getRFTInfo(content);
        }
        response.send(Http::Code::Ok, content);
    }

}