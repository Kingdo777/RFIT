#pragma once

#include <utils/exception.h>
#include <utils/locks.h>
#include <pistache/mailbox.h>
#include <queue>

using namespace std;
using namespace RFIT_NS::utils;

//template<typename T>
//class IQueue : public Pistache::PollableQueue<T> {
//public:
//
//    using Base = Pistache::PollableQueue<T>;
//    typedef typename Pistache::Queue<T>::Entry Entry;
//
//    template<class U>
//    void push(U &&u)  {
//        UniqueLock lock(mx);
//        Base::push(u);
//        count++;
//    }
//
//    Entry *pop() override {
//        UniqueLock lock(mx);
//        Base::pop();
//        count--;
//    }
//
//    size_t size() {
//        UniqueLock lock(mx);
//        return count;
//    }
//
//private:
//    std::mutex mx{};
//    size_t count = 0;
//};
