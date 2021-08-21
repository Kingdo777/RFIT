//
// Created by kingdo on 2021/7/23.
//

#ifndef RFIT_CORE_H
#define RFIT_CORE_H

#include <WAVM/Inline/HashMap.h>
#include <WAVM/Runtime/Runtime.h>
#include <wavm/WAVMWasmModule.h>
#include <pistache/async.h>
#include <pistache/http.h>
#include <utils/locks.h>
#include <utils/queue.h>
#include <utils/lru.h>
#include <utils/gids.h>
#include <utils/files.h>
#include <utils/dl.h>
#include <proto/rfit.pb.h>
#include <wavm/WAVMWasmModule.h>
#include <memory>
#include <utility>
#include <vector>
#include <string>
#include <utility>
#include <list>
#include <unordered_map>
#include <thread>

#define CPU_DEFAULT_SHARES 1024
#define CPU_DEFAULT_CFS_QUOTA_US 100000
#define CPU_DEFAULT_CFS_PERIOD_US 100000
#define DEFAULT_CPU_RATE 1.0

#define MEM_DEFAULT_HARD_LIMIT (256*1024*1024)
#define MEM_DEFAULT_SOFT_LIMIT MEM_DEFAULT_HARD_LIMIT
using namespace std;
using namespace Pistache;
namespace RFIT_NS {

    class R;

    class F;

    class I;

    class T;


    struct CpuResource {
        friend class CGroup;

    public:
        CpuResource() :
                impl(Impl()) {};

        CpuResource(uint64_t cpu_shares, int64_t cfs_quota_us, int64_t cfs_period_us) :
                impl(Impl(cpu_shares, cfs_quota_us, cfs_period_us)) {};

        [[nodiscard]] uint64_t getHash() const { return impl.hash; }

        friend bool operator==(const CpuResource &left, const CpuResource &right) {
            return left.impl.hash == right.impl.hash;
        }

        friend bool operator!=(const CpuResource &left, const CpuResource &right) {
            return left.impl.hash != right.impl.hash;
        }

        [[nodiscard]] string toString() const {
            return "CpuResource::cpu_shares-" + to_string(impl.cpuShares) + " ::cfs_quota_us-" +
                   to_string(impl.cfs_quota_us) + " ::cfs_period_us-" + to_string(impl.cfs_period_us);
        }

    private:
        struct Impl {
            uint64_t hash;
            uint64_t cpuShares;
            int64_t cfs_quota_us;
            int64_t cfs_period_us;

            explicit Impl(uint64_t cpu_shares = CPU_DEFAULT_SHARES, int64_t cfs_quota_us = CPU_DEFAULT_CFS_QUOTA_US,
                          int64_t cfs_period_us = CPU_DEFAULT_CFS_PERIOD_US);
        };

        const Impl impl;
    };

    struct MemResource {
        friend class CGroup;

    public:
        MemResource() :
                impl(Impl()) {};

        MemResource(int64_t mem_soft_limit, int64_t mem_hard_limit) :
                impl(Impl(mem_soft_limit, mem_hard_limit)) {};

        [[nodiscard]] uint64_t getHash() const { return impl.hash; }

        friend bool operator==(const MemResource &left, const MemResource &right) {
            return left.impl.hash == right.impl.hash;
        }

        friend bool operator!=(const MemResource &left, const MemResource &right) {
            return left.impl.hash != right.impl.hash;
        }

        [[nodiscard]] string toString() const {
            return "MemResource::mem_soft_limit-" + to_string(impl.mem_soft_limit) + " ::mem_hard_limit-" +
                   to_string(impl.mem_hard_limit);
        }

    private:
        struct Impl {
            uint64_t hash;
            int64_t mem_soft_limit;
            int64_t mem_hard_limit;

            // 就一个构造函数,而且此构造函数，仅仅在getUniqueImpl中被调用，这样的话，所有的Impl就会保证是唯一的
            explicit Impl(int64_t mem_soft_limit = MEM_DEFAULT_SOFT_LIMIT,
                          int64_t mem_hard_limit = MEM_DEFAULT_HARD_LIMIT);
        };

        const Impl impl;
    };

    struct Resource {
        friend class CGroup;

    public:
        Resource() :
                impl(Impl()) {};

        Resource(const CpuResource &cpu,
                 const MemResource &mem) :
                impl(Impl(cpu, mem)) {};

        [[nodiscard]] uint64_t getHash() const { return impl.hash; }

        [[nodiscard]] string toString() const {
            return "Resource::{" + impl.cpu.toString() + ", " + impl.mem.toString() + "}";
        }

    private:
        struct Impl {
            uint64_t hash;
            CpuResource cpu;
            MemResource mem;

            // 就一个构造函数,而且此构造函数，仅仅在getUniqueImpl中被调用，这样的话，所有的Impl就会保证是唯一的
            explicit Impl(const CpuResource &cpu = CpuResource(), const MemResource &mem = MemResource());
        };

        const Impl impl;
    };

    class R {
        friend class F;

        friend class T;

        friend class RFIT;

    public:
        explicit R(Resource &&r) : resource(r) {};

        uint64_t getHash() {
            return resource.getHash();
        }

        LRU<shared_ptr<F>, string> &getFList() {
            return F_list;
        }

        string toString() {
            return "R: " + resource.toString();
        }

    private:
        const Resource resource;

        LRU<shared_ptr<F>, string> F_list;
    };


#define BASE_CGROUP_DIR  "/sys/fs/cgroup/"
#define BASE_CGROUP_NAME "RFIT"
#define CG_CPU  "cpu"
#define CG_MEM  "memory"

    class CGroup {
    public:
        void addCurrentThread();

        void changeConfig(const Resource &res);

        void setName(std::string name_);

        void destroy();

    private:
        std::string name;
        uint64_t cpu_hash = 0;
        uint64_t mem_hash = 0;

        static void changeCPUCGConfig(const CpuResource &res, boost::filesystem::path &confPath);

        static void changeMemCGConfig(const MemResource &res, boost::filesystem::path &confPath);
    };

    typedef void (*FuncType)(Message &);

    class I {
    public:
        friend class T;

        I(Message msg_,
          shared_ptr<R> r_,
          shared_ptr<F> f_);

        const Message &getMsg() const;

        const shared_ptr<R> &getR() const;

        const shared_ptr<F> &getF() const;

        bool invoke();

    private:
        uint64_t id;
        Message msg;
        shared_ptr<R> r;
        shared_ptr<F> f;
        wasm::WAVMWasmModule module;
    };

    //T的三种存在状态
    enum T_status {
        Idle,
        Busy,
        None,
    };

    class T {
        friend class RFIT;

        friend class TaskPool;

        struct Context {
            friend class T;

            Context() = default;;

        public:
            void set() { tid = this_thread::get_id(); }

            string toString() {
                std::ostringstream oss;
                oss << tid;
                return "{tid:" + oss.str() + "}";
            }

            string getTID_s() {
                std::ostringstream oss;
                oss << tid;
                return oss.str();
            };

            std::thread::id getTID() {
                return tid;
            };

        private:
            std::thread::id tid;
        };

        struct InvokeEntry {
            InvokeEntry(Pistache::Async::Deferred<void> deferred_,
                        shared_ptr<I> instance_,
                        Pistache::Http::ResponseWriter response_) :
                    deferred(move(deferred_)),
                    instance(std::move(instance_)),
                    response(std::move(response_)) {}

            Pistache::Async::Deferred<void> deferred;
            shared_ptr<I> instance;
            Pistache::Http::ResponseWriter response;
        };

        enum InvokeStatus {
            success,
            faild,
            wrong
        };

        struct workerInvokeDoneEntry {
            workerInvokeDoneEntry(uint64_t id_, InvokeStatus status_, string msg_) :
                    id(id_), status(status_), msg(std::move(msg_)) {};
            uint64_t id;
            InvokeStatus status;
            string msg;
        };

        struct WorkerPool;
        struct Worker;

        struct Worker {
            explicit Worker(WorkerPool *wp_);

            uint64_t getID() { return id; }

            void shutdown() {
                shutdownFd.notify();
                worker.join();
            }

            void push(shared_ptr<I> i) { IQueue.push(std::move(i)); }

            void setSelf(shared_ptr<Worker> self_) { self = std::move(self_); }

            void kill() {
                // TODO
                assert(false);
                // worker.join();
            }

        private:
            uint64_t id = utils::generateGid();
            WorkerPool *wp;
            shared_ptr<Worker> self;
            Pistache::Polling::Epoll poller;
            // 理论上来说，一个worker只能处理一个I
            Pistache::PollableQueue<shared_ptr<I>> IQueue;
            Pistache::NotifyFd shutdownFd;
            std::thread worker;

            void handleIQueue();

            void run();
        };

        struct WorkerPool {
            WorkerPool(uint poolSize,
                       shared_ptr<PollableQueue<workerInvokeDoneEntry>> doneQueue_) :
                    size(poolSize),
                    doneQueue(std::move(doneQueue_)) {
            }

            void invoke(const shared_ptr<I> &instance);

            void invokeDone(const shared_ptr<Worker> &worker, const workerInvokeDoneEntry &entry);

            void resize(uint newSize);

            void shutdown();

            uint getSize() const { return size; }

            size_t getIdleWorkersCount() { return idleWorkers.size(); }

            size_t getBusyWorkersCount() { return busyWorkers.size(); }

            shared_ptr<PollableQueue<workerInvokeDoneEntry>> getDoneQueue() {
                return doneQueue;
            }

        private:
            uint size;
            shared_ptr<PollableQueue<workerInvokeDoneEntry>> doneQueue;
            std::queue<shared_ptr<Worker>> idleWorkers;
            std::unordered_map<uint64_t, shared_ptr<Worker>> busyWorkers;
            std::mutex mutex;
        };

    public:
        T();

        void run();

        void shutdown();

        [[nodiscard]] uint64_t getID() const { return id; }

        string toString() {
            return "T: {id:" + to_string(id) +
                   ", context:" + context.toString() +
                   ", status:" +
                   ((ICount == wp.getBusyWorkersCount() &&
                     wp.getBusyWorkersCount() + wp.getIdleWorkersCount() <= wp.getSize())
                    ? string("Good!") : string("BAD!")) +
                   ", ICount:" + to_string(ICount) +
                   ", workCount:" + to_string(workCount) +
                   ", dispatchCount:" + to_string(dispatchCount) +
                   ", workerPoolSize:" + to_string(wp.getSize()) +
                   ", idleWorkerCount:" + to_string(wp.getIdleWorkersCount()) +
                   ", busyWorkerCount:" + to_string(wp.getBusyWorkersCount()) +
                   "}";
        }

        void incDispatchCount() {
            dispatchCount++;
        }

    private:
        uint64_t id = utils::generateGid();

        Context context;
        Polling::Epoll poller;

        PollableQueue<InvokeEntry> IQueue;

        shared_ptr<R> currentResource = nullptr;
        shared_ptr<F> workFor = nullptr;
        uint64_t ICount = 0;
        uint64_t workCount = 0;
        uint64_t dispatchCount = 0;

        std::atomic<bool> shutdown_;
        NotifyFd shutdownFd;
        thread t;

        CGroup cg;

        shared_ptr<PollableQueue<workerInvokeDoneEntry>> invokeDoneMsgQueue;
        std::unordered_map<uint64_t, shared_ptr<InvokeEntry>> invokeEntryMap;

        WorkerPool wp;
    private:
        void handleFds(const std::vector<Polling::Event> &events);

        void handleIQueue();

        void handleInvokeDoneMsgQueue();

        bool checkI(const shared_ptr<I> &instance);

        bool adjustResource(const shared_ptr<I> &instance);

        bool changeCgroup();

        void doExecute(const shared_ptr<InvokeEntry> &invokeEntry);

        void doExecuteWasm(const shared_ptr<InvokeEntry> &invokeEntry);

        void doLog(const shared_ptr<InvokeEntry> &invokeEntry);

        void doChangeICount();

    };

    class TSortList {
    public:
        explicit TSortList(uint32_t conc) : maxValue(conc) {}

        void returnOne(uint64_t key);

        void newOne(const shared_ptr<T> &t, bool take = true);

        bool takeIdleOne(shared_ptr<T> &t);

        bool takeOne(shared_ptr<T> &t);

        void shutdown();

        void flush();

        std::vector<shared_ptr<T>> getSortedItem();

    private:
        void putOrUpdate(int increment, const shared_ptr<T> &t);

        void updateT(int count, const shared_ptr<T> &t);

        void newT(int count, const shared_ptr<T> &t);

        bool getTbyKey(uint64_t key, shared_ptr<T> &t) {
            if (map.find(key) == map.end())
                return false;
            t = map[key]->second;
            return true;
        }

        /// list记录了数据单元,以及用于排序的标尺
        std::list<pair<int, shared_ptr<T>>> l;
        /// map记录了数据单元在list中的索引，map的key值即U
        std::unordered_map<uint64_t, std::list<pair<int, shared_ptr<T>>>::iterator> map;
        /// nextT始终始终指向l中下一个服务请求的T
        std::list<pair<int, shared_ptr<T>>>::iterator nextT = l.end();
        int maxValue;
        std::mutex mutex;
    };

    class F {
    public:
        friend class T;

        F() = delete;

        F(string user_,
          string funcName_,
          shared_ptr<R> r_,
          uint32_t concurrency = 1);

        F(string user_,
          string funcName_,
          shared_ptr<R> r_,
          utils::dlResult dr_,
          uint32_t concurrency = 1
        );

        F(string user_,
          string funcName_,
          shared_ptr<R> r_,
          const WAVM::Runtime::ModuleRef &module_,
          uint32_t concurrency = 1);

        static string makeFuncStr(const string &user, const string &funcName) {
            return user + "-" + funcName;
        }

        string getFuncStr() {
            if (isWasm())
                return wasm.module.getFuncStr();
            else
                return user + "-" + funcName;
        }

        [[nodiscard]] const string &getFuncName() const {
            return funcName;
        }

        [[nodiscard]] const dlResult &getDr() const {
            return native.dr;
        }

        [[nodiscard]] const shared_ptr<R> &getR() const {
            return r;
        }

        [[nodiscard]] uint32_t getConcurrency() const {
            return concurrency;
        }

        [[nodiscard]] bool isConcurrency() const {
            return concurrency > 1;
        }

        wasm::WAVMWasmModule &getModule() {
            return wasm.module;
        }

        bool isWasm() const { return isWasm_; }

        void invoke(Message &msg);

        bool getIdleT(shared_ptr<T> &t);

        bool getT(shared_ptr<T> &t);

        /// take 表示此T将在TList计数加+1
        void newT(const shared_ptr<T> &t, bool take = true);

        void shutdownAllT();

        std::vector<shared_ptr<T>> getAllT();

        void setDL(utils::dlResult dr_) {
            native.dr = dr_;
            isWasm_ = false;
        }

        void setWASM(const WAVM::Runtime::ModuleRef &module_) {
            wasm.module = wasm::WAVMWasmModule(user, funcName, module_);
            isWasm_ = true;
        }

        string getPath() {
            if (!isWasm_)
                return PRO_ROOT "/Function/lib/" + funcName + "/function.so";
            else
                return PRO_ROOT "/Function/wasm/function/" + funcName + "/function.wasm";
        };

        string getOBJPath() {
            if (!isWasm_)
                return "";
            else
                return PRO_ROOT "/Function/wasm/object/" + funcName + "/function.wasm.o";
        };

        string toString() {
            return "F: {funcName:" + funcName + ", conc:" + to_string(concurrency) + ", dlPath:" +
                   getPath() +
                   "}";
        }

        bool pingFunc();

    private:

        string user;

        string funcName;

        bool isWasm_ = false;

        struct {
            utils::dlResult dr;
        } native;

        struct {
            wasm::WAVMWasmModule module;
        } wasm;

        shared_ptr<R> r;

        uint32_t concurrency = 1;

        TSortList TList;

    };

}
#endif //RFIT_CORE_H
