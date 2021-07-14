#include <utils/gids.h>

#include <atomic>
#include <mutex>

#include <utils/locks.h>
#include <utils/random.h>

static std::atomic_int counter = 0;
static std::size_t gidKeyHash = 0;
static std::mutex gidMx;

#define GID_LEN 20

namespace RFIT_NS::utils {
    unsigned int generateGid() {
        if (gidKeyHash == 0) {
            RFIT_NS::utils::UniqueLock lock(gidMx);
            if (gidKeyHash == 0) {
                // Generate random hash
                std::string gidKey = RFIT_NS::utils::randomString(GID_LEN);
                gidKeyHash = std::hash<std::string>{}(gidKey);
            }
        }

        unsigned int intHash = gidKeyHash % INT32_MAX;
        unsigned int result = intHash + counter.fetch_add(1);
        if (result) {
            return result;
        } else {
            return intHash + counter.fetch_add(1);
        }
    }
}
