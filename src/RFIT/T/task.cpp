//
// Created by kingdo on 10/20/20.
//
#include <RFIT/core.h>

namespace RFIT_NS {

    void T::handleFds(const std::vector<Polling::Event> &events) {
        for (auto e: events) {
            if (e.tag == shutdownFd.tag())
                return;
            if (e.tag == IQueue.tag())
                handleIQueue();
        }
    }

    void T::run() {
        context.set();
        for (;;) {
            std::vector<Polling::Event> events;
            int ready_fds = poller.poll(events);
            switch (ready_fds) {
                case -1:
                case 0:
                    break;
                default:
                    if (shutdown_)
                        return;
                    handleFds(events);
            }
        }
    }

    T::T() : context(),
             poller(),
             IQueue(),
             shutdown_(false),
             shutdownFd() {
        IQueue.bind(poller);
        shutdownFd.bind(poller);
        t = thread([this] {
            run();
        });

    }

    void T::shutdown() {
        shutdown_.store(true);
        shutdownFd.notify();
        t.join();
    }

    void T::handleIQueue() {
        for (;;) {
            shared_ptr<InvokeEntry> invokeEntry = IQueue.popSafe();
            if (!invokeEntry)
                break;
            //// 如果不是异步执行，那么ICount是没有任何意义的变量
            ICount++;
            // 配置正确的Cgroup
            adjustResource(invokeEntry->instance);
            doExecute(invokeEntry);
        }
    }

    /// 变更资源配置
    bool T::adjustResource(const shared_ptr<I> &instance) {
        if (!checkI(instance) ||
            (currentResource && currentResource->getHash() == instance->r->getHash()))
            return true;
        auto newResource = instance->r;
        currentResource = newResource;
        return changeCgroup();
    }

    /// 切换线程所在的 Cgroup
    bool T::changeCgroup() {
        return true;
    }

    /// 用于更改 T 所服务的 F，返回值指示是否需要变换资源配置
    bool T::checkI(const shared_ptr<I> &instance) {
        assert(instance != nullptr);
        if (workFor == instance->f) {
            return false;
        } else if (workFor == nullptr || ICount == 0) {
            workFor = instance->f;
            return true;
        }
        // 目前来说一个T只能服务于一个F
        // TODO
        throw std::runtime_error("Must One T for One F\n");
    }

    /// 执行function
    /// 1、调用用户函数
    /// 2、返回结果给用户
    /// 3、清除Function执行过程中注册的fd
    /// 4、保存I的信息到日志中
    /// 5、修改Icount
    void T::doExecute(const shared_ptr<InvokeEntry> &invokeEntry) {
        try {
            invokeEntry->instance->f->invoke(invokeEntry->instance->msg);
        } catch (exception &e) {
            string s;
            if (e.what() != nullptr)
                s = e.what();
            invokeEntry->response.send(Pistache::Http::Code::Bad_Request,
                                       "Register Wrong: " + s);
            invokeEntry->deferred.reject(s);
        }
        invokeEntry->response.send(Http::Code::Ok, invokeEntry->instance->msg.outputdata());
        invokeEntry->deferred.resolve();
        doClean();
        doLog();
        doChangeICount();
    }


    void T::doClean() {

    }

    void T::doLog() {

    }

    void T::doChangeICount() {
        ICount--;
        this->workFor->TList.returnOne(this->id);
    }

    void TSortList::putOrUpdate(int increment, const shared_ptr<T> &t) {
        auto key = t->getID();
        if (map.find(key) == map.end()) {
            insert(0, t, l.begin());
            return;
        }
        auto item = map[key];
        if (0 == increment)
            return;
        insert(item->first + increment, item->second, item, increment > 0);
        l.erase(item);
    }

    void
    TSortList::insert(int count, const shared_ptr<T> &t, std::list<pair<int, shared_ptr<T>>>::iterator begin,
                      bool back) {
        uint64_t key = t->getID();
        /// 保证count不能是负数
        if (count < 0)
            count = 0;
        if (l.empty()) {
            l.emplace_back(count, t);
            map[key] = --l.end();
            return;
        }
        if (back) {
            for (auto item = begin; item != l.end(); item++) {
                if (item->first >= count) {
                    map[key] = l.insert(item, pair<int, shared_ptr<T>>(count, t));
                    return;
                }
            }
            l.emplace_back(count, t);
            map[key] = --l.end();
        } else {
            for (auto item = begin; item != l.end();) {
                if (item->first < count) {
                    map[key] = l.insert(++item, pair<int, shared_ptr<T>>(count, t));
                    return;
                }
                if (item == l.begin())
                    break;
                item--;
            }
            l.push_front(pair<int, shared_ptr<T>>(count, t));
            map[key] = l.begin();
        }
    }

    void TSortList::newOne(const shared_ptr<T> &t, bool take) {
        utils::UniqueLock lock(mutex);
        auto i = take ? 1 : 0;
        putOrUpdate(i, t);
    }

    bool TSortList::takeOne(shared_ptr<T> &t, int maxCount) {
        utils::UniqueLock lock(mutex);
        if (l.empty())
            return false;
        auto t_ = l.front();
        if (t_.first < maxCount) {
            t = t_.second;
            putOrUpdate(1, t);
            return true;
        }
        return false;
    }

    void TSortList::returnOne(uint64_t key) {
        utils::UniqueLock lock(mutex);
        shared_ptr<T> t;
        assert(getT(key, t));
        putOrUpdate(-1, t);
    }

}