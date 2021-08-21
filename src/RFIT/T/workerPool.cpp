//
// Created by kingdo on 2021/8/18.
//
#include <RFIT/core.h>
#include <utils/config.h>
#include <utils/locks.h>

namespace RFIT_NS {
    T::Worker::Worker(WorkerPool *wp_) : wp(wp_), poller(), IQueue(), shutdownFd() {
        IQueue.bind(poller);
        shutdownFd.bind(poller);
        run();
    }

    void T::Worker::run() {
        worker = thread([this]() {
            for (;;) {
                vector<Pistache::Polling::Event> events;
                int ready_fds = poller.poll(events);
                if (ready_fds == -1) {
                    throw runtime_error("Polling");
                }
                for (const auto &e:events) {
                    if (e.tag == shutdownFd.tag())
                        return;
                    else if (e.tag == IQueue.tag()) {
                        handleIQueue();
                    }
                }
            }
        });
    }

    void T::Worker::handleIQueue() {
        for (;;) {
            auto instance = IQueue.popSafe();
            if (!instance)
                break;
            string s;
            InvokeStatus status;
            try {
                bool success = (*instance)->invoke();
                status = success ? InvokeStatus::success : InvokeStatus::faild;
                s = success ? "OK" : "Execution failed";
            } catch (exception &e) {
                if (e.what() != nullptr)
                    s = e.what();
                default_logger->error("Execute Wrong in worker: {}", s);
                status = InvokeStatus::wrong;
                s = "Execution Wrong";
            }
            workerInvokeDoneEntry w((*instance)->id, status, s);
            wp->invokeDone(self, w);
        }
    }


    void T::WorkerPool::invoke(const shared_ptr<I> &instance) {
        utils::UniqueLock lock(mutex);
        if (idleWorkers.empty() && busyWorkers.size() < size) {
            auto worker = make_shared<Worker>(this);
            worker->setSelf(worker);
            idleWorkers.push(worker);
        }
        assert(!idleWorkers.empty());
        auto worker = idleWorkers.front();
        idleWorkers.pop();
        busyWorkers[worker->getID()] = worker;
        worker->push(instance);
    }

    void T::WorkerPool::invokeDone(const shared_ptr<Worker> &worker, const workerInvokeDoneEntry &entry) {
        utils::UniqueLock lock(mutex);
        busyWorkers.erase(worker->getID());
        idleWorkers.push(worker);
        doneQueue->push(entry);
    }

    void T::WorkerPool::resize(uint newSize) {
        utils::UniqueLock lock(mutex);
        assert(busyWorkers.empty());
        assert(idleWorkers.size() == size);
        assert(newSize <= systemConfig.maxFuncConcurrency);
        for (auto size_ = size; size_ > newSize; size_--) {
            auto worker = idleWorkers.front();
            idleWorkers.pop();
            worker->shutdown();
        }
        size = newSize;
    }

    void T::WorkerPool::shutdown() {
        utils::UniqueLock lock(mutex);
        for (const auto &bw:busyWorkers) {
            bw.second->kill();
        }
        while (!idleWorkers.empty()) {
            auto iw = idleWorkers.front();
            idleWorkers.pop();
            iw->shutdown();

        }
    }
}