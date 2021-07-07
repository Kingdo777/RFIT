#pragma once

#include <utils/exception.h>
#include <utils/locks.h>
#include <queue>

using namespace std;
using namespace RFIT::utils;

template<typename T>
class Queue {
public:
    void enqueue(T value) {
        UniqueLock lock(mx);
        mq.emplace(std::move(value));
    }

    T tryDequeue(T tryValue) {
        UniqueLock lock(mx);
        if (mq.empty())
            return tryValue;
        T value = std::move(mq.front());
        mq.pop();
        return value;
    }

    void drain() {
        UniqueLock lock(mx);
        while (!mq.empty()) {
            mq.pop();
        }
    }

    long size() {
        UniqueLock lock(mx);
        return mq.size();
    }

    void reset() {
        UniqueLock lock(mx);
        std::queue<T> empty;
        std::swap(mq, empty);
    }

private:
    std::queue<T> mq;
    UniqueLock mx{};
};
