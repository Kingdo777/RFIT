//
// Created by kingdo on 10/20/20.
//
#include <RFIT/core.h>
#include <utils/clock.h>

namespace RFIT_NS {

    void T::handleFds(const std::vector<Polling::Event> &events) {
        for (auto e: events) {
            if (e.tag == shutdownFd.tag())
                return;
            if (e.tag == IQueue.tag()) {
                handleIQueue();
            }
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
            //// 如果不是异步执行，那么ICount是没有任何意义的变量 TODO
            ICount++;
            workCount++;
            // 配置正确的Cgroup
            assert(adjustResource(invokeEntry->instance));
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
        doLog(invokeEntry);
        doChangeICount();
    }


    void T::doClean() {

    }

    void T::doLog(const shared_ptr<InvokeEntry> &invokeEntry) {
        invokeEntry->instance->msg.set_finishtimestamp(utils::Clock::epochMillis());
    }

    void T::doChangeICount() {
        ICount--;
        this->workFor->TList.returnOne(this->id);
    }

    void TSortList::putOrUpdate(int increment, const shared_ptr<T> &t) {
        /// 必须确保increment的值变化不能超过maxValue，因为目前只能操作一个T
        assert(abs(increment) <= maxValue);
        auto key = t->getID();
        if (map.find(key) == map.end()) {
            /// 此时需要创建新的T，那么increment必须不能是负数
            assert(increment >= 0);
            newT(increment, t);
        } else {
            if (0 == increment)
                return;
            /// 如果是增加，那么必须是操作nextT
            assert(increment < 0 || (increment > 0 && t == nextT->second));
            auto iter = map[key];
            auto location = iter->first + increment;
            assert(location >= 0 && location <= maxValue);
            updateT(location, iter->second);
        }
    }

    void TSortList::newT(int count, const shared_ptr<T> &t) {
        uint64_t key = t->getID();
        auto iter = map.find(key);
        assert(iter == map.end());
        /// 如果仅仅是单纯的创建一个T或者当前不存在nextT
        if (count == 0 || (count != maxValue && nextT == l.end())) {
            /// 创建，然后放在队尾
            l.emplace_back(count, t);
            map[key] = --l.end();
            if (nextT == l.end())
                nextT = map[key];
        } else if (count == maxValue) {
            /// 创建，然后放在队首
            l.emplace_front(maxValue, t);
            map[key] = l.begin();
        } else { /// count != maxValue && nextT ！= l.end()
            if (count > nextT->first) {
                map[key] = l.insert(nextT, std::pair<int, shared_ptr<T>>(count, t));
                nextT = map[key];
            } else {
                auto i = nextT;
                for (++i; i != l.end(); ++i) {
                    if (count >= i->first) {
                        map[key] = l.insert(i, std::pair<int, shared_ptr<T>>(count, t));
                        return;
                    }
                }
                l.emplace_back(count, t);
                map[key] = --l.end();
            }
        }
    }

    void TSortList::updateT(int count, const shared_ptr<T> &t) {
        uint64_t key = t->getID();
        auto iter = map.find(key);
        assert(iter != map.end());
        if (count == map[key]->first)
            return;
        if (iter->second == nextT) {
            if (count == 0) {
                l.emplace_back(count, t);
                map[key] = --l.end();
                l.erase(nextT++);
            } else if (count >= map[key]->first) {
                map[key]->first = count;
                if (count == maxValue)
                    ++nextT;
            } else {
                auto i = nextT;
                for (++i; i != l.end(); ++i)
                    if (count >= i->first) {
                        map[key] = l.insert(i, std::pair<int, shared_ptr<T>>(count, t));
                        break;
                    }
                if (i == l.end()) {
                    l.emplace_back(count, t);
                    map[key] = --l.end();
                }
                l.erase(nextT++);
            }
        } else {
            assert(count < map[key]->first);
            if (nextT == l.end()) {
                l.emplace_back(count, t);
                l.erase(map[key]);
                nextT = map[key] = --l.end();
            } else {
                if (count < nextT->first) {
                    auto i = map[key];
                    l.erase(i++);
                    for (; i != l.end(); ++i)
                        if (count >= i->first) {
                            map[key] = l.insert(i, std::pair<int, shared_ptr<T>>(count, t));
                            return;
                        }
                    if (i == l.end()) {
                        l.emplace_back(count, t);
                        map[key] = --l.end();
                    }
                } else {
                    l.erase(map[key]);
                    nextT = map[key] = l.insert(nextT, std::pair<int, shared_ptr<T>>(count, t));
                }
            }
        }
    }

    void TSortList::newOne(const shared_ptr<T> &t, bool take) {
        utils::UniqueLock lock(mutex);
        auto i = take ? 1 : 0;
        putOrUpdate(i, t);
    }

    bool TSortList::takeIdleOne(shared_ptr<T> &t) {
        utils::UniqueLock lock(mutex);
        if (l.empty())
            return false;
        auto t_ = l.back();
        if (t_.first == 0) {
            t = t_.second;
            l.pop_back();
            map.erase(t->getID());
            return true;
        }
        return false;
    }

    bool TSortList::takeOne(shared_ptr<T> &t) {
        utils::UniqueLock lock(mutex);
        if (nextT == l.end())
            return false;
        assert(nextT->first < maxValue);
        t = nextT->second;
        putOrUpdate(1, t);
        return true;
    }

    void TSortList::returnOne(uint64_t key) {
        utils::UniqueLock lock(mutex);
        shared_ptr<T> t;
        assert(getTbyKey(key, t));
        putOrUpdate(-1, t);
    }

    void TSortList::shutdown() {
        utils::UniqueLock lock(mutex);
        for (const auto &t : l) {
            t.second->shutdown();
        }
    }

    void TSortList::flush() {
        utils::UniqueLock lock(mutex);
        l.clear();
        map.clear();
    }

    std::vector<shared_ptr<T>> TSortList::getSortedItem() {
        utils::UniqueLock lock(mutex);
        std::vector<shared_ptr<T>> t;
        for (const auto &item : l) {
            t.push_back(item.second);
        }
        return t;
    }

}