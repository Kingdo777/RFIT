//
// Created by kingdo on 2021/7/8.
//

#include <WAVM/WASM/WASM.h>
#include "RFIT/RFIT.h"

#define MAX_TABLE_SIZE 500000

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
        tp.shutdown();
    }

    shared_ptr<R> RFIT::createR(Resource res) {
        if (RMap.find(res.getHash()) == end(RMap)) {
            auto r = std::make_shared<R>(move(res));
            RMap.emplace(r->getHash(), r);
            return r;
        }
        return RMap[res.getHash()];
    }

    void sendResponse(const FunctionRegisterMsg &m, const shared_ptr<RFIT::FunctionRegisterEntry> &func) {
        if (m.status()) {
            func->response.send(Pistache::Http::Code::Ok, "OK");
        } else {
            func->response.send(Pistache::Http::Code::Bad_Request, "Register Failed:" + m.message());
        }
        func->deferred.resolve();
    }


    void
    RFIT::registerF(FunctionRegisterMsg &msg, dlResult &dl, const boost::filesystem::path &p) {
        CpuResource cr(CPU_DEFAULT_SHARES * (int(msg.coreration() * 10)),
                       (uint64_t) (msg.coreration() * CPU_DEFAULT_CFS_PERIOD_US),
                       CPU_DEFAULT_CFS_PERIOD_US);
        MemResource mr(msg.memsize(), msg.memsize());
        Resource resource(cr, mr);
        auto r = createR(resource);
        auto f = make_shared<F>(msg.user(), msg.funcname(), r, dl, msg.concurrency());
        if (!f->pingFunc()) {
            close_remove_DL(p, dl.handle);
            msg.set_status(false);
            msg.set_message("Ping Failed");
        } else {
            FMap.emplace(f->getFuncStr(), f);
            msg.set_status(true);
            msg.set_message("OK");
        }
    }

    void RFIT::handleNativeFuncRegister(const shared_ptr<FunctionRegisterEntry> &func) {
        const string &dlPath = utils::makeDL(func->msg.funcname(),
                                             func->msg.dldata().c_str(),
                                             func->msg.dldata().size());
        pair<dlResult, std::string> res = utils::getFuncEntry(dlPath,
                                                              func->msg.funcname() + config.entrySuffix);
        if (res.first.handle == nullptr || res.first.addr == nullptr || !res.second.empty()) {
            const string &message = res.second.empty() ? (func->msg.funcname() + "_main" + "is NULL type")
                                                       : res.second;
            func->msg.set_status(false);
            func->msg.set_message(message);
        } else {
            registerF(func->msg, res.first, dlPath);
        }
        sendResponse(func->msg, func);
    }


    void
    RFIT::registerF(FunctionRegisterMsg &msg, WAVM::Runtime::ModuleRef &module) {
        CpuResource cr(CPU_DEFAULT_SHARES * (int(msg.coreration() * 10)),
                       (uint64_t) (msg.coreration() * CPU_DEFAULT_CFS_PERIOD_US),
                       CPU_DEFAULT_CFS_PERIOD_US);
        MemResource mr(msg.memsize(), msg.memsize());
        Resource resource(cr, mr);
        auto r = createR(resource);
        auto f = make_shared<F>(msg.user(), msg.funcname(), r, module, msg.concurrency());
        if (!f->pingFunc()) {
            msg.set_status(false);
            msg.set_message("Ping Failed");
        } else {
            FMap.emplace(f->getFuncStr(), f);
            msg.set_status(true);
            msg.set_message("OK");
        }
    }

    /// 1、检查是wasm二进制文件
    /// 2、检查hash值  #TODO
    /// 3、写入到指定的路径: PRO_ROOT/Function/wasm/function/##name##/function.wasm
    /// 4、编译，并存储到执行路径： PRO_ROOT/Function/wasm/function/##name##/function.wasm.o
    /// 5、注册F
    void RFIT::handleWasmFuncRegister(const shared_ptr<FunctionRegisterEntry> &func) {
        auto conf = systemConfig;
        //// 写入wasm文件
        const std::string &fileBody = func->msg.dldata();
        assert(!fileBody.empty());
        assert(utils::isWasm(fileBody));
        auto wasmFile = boost::filesystem::path(conf.functionDir).
                append(func->msg.user()).
                append(func->msg.funcname()).
                append("/function.wasm");
        writeStringToFile(wasmFile, fileBody);

        //// 生成Module，并将obj写入文件
        WAVM::IR::Module moduleIR;
        moduleIR.featureSpec.simd = true;
        WAVM::WASM::LoadError loadError;
        if (!WAVM::WASM::loadBinaryModule(reinterpret_cast<const WAVM::U8 *>(fileBody.data()), fileBody.size(),
                                          moduleIR, &loadError))
            throw std::runtime_error("Failed to parse wasm binary");
        auto wasmObjFile = boost::filesystem::path(conf.objectFileDir).
                append(func->msg.user()).
                append(func->msg.funcname()).
                append("function.wasm.o");
        // Compile the module to object code
        WAVM::Runtime::ModuleRef module;
        std::vector<uint8_t> newHash = hashBytes(fileBody.data(), fileBody.size());
        std::vector<uint8_t> oldHash = readFileToBytes(wasmObjFile.parent_path().append("function.wasm.o.md5"));
        if (newHash != oldHash) {
            module = WAVM::Runtime::compileModule(moduleIR);
            writeBytesToFile(wasmObjFile.parent_path().append("function.wasm.o.md5"), newHash);
        } else {
            auto objectCode = readFileToBytes(wasmObjFile);
            module = std::make_shared<WAVM::Runtime::Module>(WAVM::IR::Module(moduleIR), std::move(objectCode));
        }
        std::vector<uint8_t> objBytes = WAVM::Runtime::getObjectCode(module);
        writeBytesToFile(wasmObjFile, objBytes);
        /// 修改table的最大值，这样在dl时，可以扩展table
        // Force maximum size
        module->ir.tables.defs[0].type.size.max = (WAVM::U64) MAX_TABLE_SIZE;

        //// 注册F
        registerF(func->msg, module);
        sendResponse(func->msg, func);
    }

    void RFIT::handlerFuncRegisterQueue() {
        for (;;) {
            shared_ptr<FunctionRegisterEntry> func = funcRegisterQueue.popSafe();
            if (!func)
                break;
            const std::string &funcName = F::makeFuncStr(func->msg.user(), func->msg.funcname());
            if (existF(funcName)) {
                func->response.send(Pistache::Http::Code::Bad_Request,
                                    "Register Failed:" + funcName + " is exist!");
                func->deferred.resolve();
                continue;
            }
            try {
                if (func->isWasm()) {
                    handleWasmFuncRegister(func);
                } else {
                    handleNativeFuncRegister(func);
                }
            } catch (exception &e) {
                string s;
                if (e.what() != nullptr)
                    s = e.what();
                func->response.send(Pistache::Http::Code::Bad_Request,
                                    "Register Wrong: " + s);
                func->deferred.reject(e);
            }
        }
    }

    Pistache::Async::Promise<void>
    RFIT::handlerNewFuncRegister(FunctionRegisterMsg &&msg_, Pistache::Http::ResponseWriter &&response_) {
        default_logger->info("handlerNewFuncRegister" + utils::messageToJson(msg_));
        return Pistache::Async::Promise<void>(
                [&](Pistache::Async::Deferred<void> deferred) {
                    FunctionRegisterEntry func(std::move(deferred), std::move(msg_), std::move(response_));
                    this->funcRegisterQueue.push(std::move(func));
                });
    }

    Pistache::Async::Promise<void> RFIT::handlerFuncInvoke(Message &&msg, Pistache::Http::ResponseWriter &&response) {
        default_logger->info("handlerFuncInvoke" + utils::messageToJson(msg));
        return Pistache::Async::Promise<void>(
                [&](Pistache::Async::Deferred<void> deferred) {
                    const std::string &funcName = F::makeFuncStr(msg.user(), msg.funcname());
                    auto f = getF(funcName);
                    if (!f) {
                        response.send(Http::Code::Bad_Request, "No Function Found For " + funcName);
                        deferred.resolve();
                    } else {
                        auto instance = make_shared<I>(std::move(msg), f->getR(), f);
                        T::InvokeEntry invokeEntry(std::move(deferred), std::move(instance), std::move(response));
                        tp.dispatch(std::move(invokeEntry));
                    }
                });
    }

    shared_ptr<F> RFIT::getF(const string &funcStr) {
        auto it = FMap.find(funcStr);
        if (it == FMap.end())
            return nullptr;
        return it->second;
    }

    bool RFIT::existF(const string &funcStr) {
        return FMap.find(funcStr) != FMap.end();
    }


    RFIT &getRFIT() {
        static RFIT rfit;
        return rfit;
    }

}