#include "snapshot/SnapshotRegistry.h"
#include <utils/locks.h>
#include <utils/logging.h>
#include <utils/memory.h>

#include <sys/mman.h>

namespace RFIT_NS::snapshot {
    SnapshotRegistry::SnapshotRegistry() {}

    SnapshotData &SnapshotRegistry::getSnapshot(
            const std::string &key) {
        auto logger = default_logger;
        if (snapshotMap.count(key) == 0) {
            logger->error("Snapshot for {} does not exist", key);
            throw std::runtime_error("Snapshot doesn't exist");
        }

        return snapshotMap[key];
    }

    void SnapshotRegistry::mapSnapshot(const std::string &key, uint8_t *target) {
        auto logger = RFIT_NS::utils::getLogger();
        SnapshotData d = getSnapshot(key);

        if (!RFIT_NS::utils::isPageAligned((void *) target)) {
            logger->error("Mapping snapshot to non page-aligned address");
            throw std::runtime_error(
                    "Mapping snapshot to non page-aligned address");
        }

        if (d.fd == 0) {
            logger->error("Attempting to map non-restorable snapshot");
            throw std::runtime_error("Mapping non-restorable snapshot");
        }

        void *mmapRes =
                mmap(target, d.size, PROT_WRITE, MAP_PRIVATE | MAP_FIXED, d.fd, 0);

        if (mmapRes == MAP_FAILED) {
            logger->error(
                    "mmapping snapshot failed: {} ({})", errno, ::strerror(errno));
            throw std::runtime_error("mmapping snapshot failed");
        }
    }

    void SnapshotRegistry::takeSnapshot(const std::string &key,
                                        SnapshotData data,
                                        bool locallyRestorable) {
        // Note - we only preserve the snapshot in the in-memory file, and do not
        // take ownership for the original data referenced in SnapshotData
        RFIT_NS::utils::UniqueLock lock(snapshotsMx);
        snapshotMap[key] = data;

        // Write to fd to be locally restorable
        if (locallyRestorable) {
            writeSnapshotToFd(key);
        }
    }

    void SnapshotRegistry::deleteSnapshot(const std::string &key) {
        RFIT_NS::utils::UniqueLock lock(snapshotsMx);

        if (snapshotMap.count(key) == 0) {
            return;
        }

        SnapshotData d = snapshotMap[key];

        // Note - the data referenced by the SnapshotData object is not owned by the
        // snapshot registry so we don't delete it here. We only remove the file
        // descriptor used for mapping memory
        if (d.fd > 0) {
            ::close(d.fd);
        }

        snapshotMap.erase(key);
    }

    size_t SnapshotRegistry::getSnapshotCount() {
        RFIT_NS::utils::UniqueLock lock(snapshotsMx);
        return snapshotMap.size();
    }

    SnapshotRegistry &getSnapshotRegistry() {
        static SnapshotRegistry reg;
        return reg;
    }

    void SnapshotRegistry::clear() {
        for (auto p : snapshotMap) {
            if (p.second.fd > 0) {
                ::close(p.second.fd);
            }
        }

        snapshotMap.clear();
    }

    int SnapshotRegistry::writeSnapshotToFd(const std::string &key) {
        auto logger = RFIT_NS::utils::getLogger();

        int fd = ::memfd_create(key.c_str(), 0);
        SnapshotData snapData = getSnapshot(key);

        // Make the fd big enough
        int ferror = ::ftruncate(fd, snapData.size);
        if (ferror) {
            logger->error("ferror call failed with error {}", ferror);
            throw std::runtime_error("Failed writing memory to fd (ftruncate)");
        }

        // Write the data
        ssize_t werror = ::write(fd, snapData.data, snapData.size);
        if (werror == -1) {
            logger->error("Write call failed with error {}", werror);
            throw std::runtime_error("Failed writing memory to fd (write)");
        }

        // Record the fd
        getSnapshot(key).fd = fd;

        logger->debug("Wrote snapshot {} to fd {}", key, fd);
        return fd;
    }
}
