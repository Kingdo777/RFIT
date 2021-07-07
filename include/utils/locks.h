#pragma once

#include <mutex>
#include <shared_mutex>

//mutex 就是普通的互斥锁，而shared_mutex是读写锁
//shared_mutex可以既可以调用lock()上独占锁定，也就是写锁，也可以调用share_lock()进行共享锁定，也就是读锁
//而mutex，只有lock()函数，进行互斥锁定

//unique_lock可以认为是锁的智能工具，与lock_guard类似，unique_lock会对象构建时自动的调用lock函数，并在对象删除时解锁
//shared_lock，则与unique_lock类似，只不过在上锁时，调用的是share_lock(),同时也会在对象删除时自动解锁

//这就意味着unique_lock可以和互斥锁与读写锁组合，但是shared_lock只能和读写锁组合

namespace RFIT::utils {
    typedef std::unique_lock<std::mutex> UniqueLock;
    typedef std::unique_lock<std::shared_mutex> FullLock;
    typedef std::shared_lock<std::shared_mutex> SharedLock;
}
