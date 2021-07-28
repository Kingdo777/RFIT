//
// Created by kingdo on 2021/7/22.
//

#ifndef RFIT_LRU_H
#define RFIT_LRU_H

#include <list>
#include <unordered_map>
#include <vector>
#include <utils/locks.h>

using namespace std;

namespace RFIT_NS::utils {
    template<typename T, typename U>
    class LRU {
    public:
        void access(const T &t, const U &u) {
            utils::UniqueLock lru_lock(mutex);
            auto i_map = map.find(u);
            if (i_map != map.end()) {
                l.erase(i_map->second);
            }
            add(t, u);
        }

        std::vector<T> getSortedItem() {
            utils::UniqueLock lru_lock(mutex);
            std::vector<T> t;
            for (auto i = l.begin(); i != l.end(); i++) {
                t.push_back(*i);
            }
            return t;
        }

    private:
        std::list<T> l;
        unordered_map<U, typename std::list<T>::iterator> map;
        std::mutex mutex;
    private:
        void add(const T &t, const U &u) {
            l.push_back(t);
            map[u] = --l.end();
        }
    };
}
#endif //RFIT_LRU_H
