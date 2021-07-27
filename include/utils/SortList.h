//
// Created by kingdo on 2021/7/22.
//

#ifndef RFIT_SORTLIST_H
#define RFIT_SORTLIST_H

#include <list>
#include <unordered_map>
#include <utils/locks.h>

using namespace std;

namespace RFIT_NS::utils {
    /// 从小到大排序
    /// T是数据单元; U是T的可标志ID，U值对于每一个T都是唯一的
    template<typename T, typename U>
    class SortList {
    private:

        void insert(uint64_t count, const T &t, U u,
                    typename std::list<pair<uint64_t, T>>::iterator begin,
                    bool back = true) {
            if (l.empty()) {
                l.push_back(pair<uint64_t, T>(count, t));
                map[u] = --l.end();
                return;
            }

            if (back) {
                for (auto item = begin; item != l.end(); item++) {
                    if (item->first >= count) {
                        map[u] = l.insert(item, pair<uint64_t, T>(count, t));
                        return;
                    }
                }
                l.push_back(pair<uint64_t, T>(count, t));
                map[u] = --l.end();
            } else {
                for (auto item = begin; item != l.end();) {
                    if (item->first < count) {
                        map[u] = l.insert(++item, pair<uint64_t, T>(count, t));
                        return;
                    }
                    if (item == l.begin())
                        break;
                    item--;
                }
                l.push_front(pair<uint64_t, T>(count, t));
                map[u] = l.begin();
            }

        }

        /// list记录了数据单元,以及用于排序的标尺
        std::list<pair<uint64_t, T>> l;
        /// map记录了数据单元在list中的索引，map的key值即U
        std::unordered_map<U, typename std::list<pair<uint64_t, T>>::iterator> map;
        std::mutex mutex;
    public:
        void putOrUpdate(uint64_t newCount, const T &t, U u) {
            utils::UniqueLock lock(mutex);
            if (map.find(u) == map.end()) {
                insert(newCount, t, u, l.begin());
                return;
            }
            typename std::list<pair<uint64_t, T>>::iterator item = map[u];
            if (item->first == newCount)
                return;
            insert(newCount, item->second, u, item, item->first < newCount);
            l.erase(item);
        }
    };
}
#endif //RFIT_SORTLIST_H
