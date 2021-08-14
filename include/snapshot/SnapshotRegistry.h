#pragma once

#include <mutex>
#include <string>
#include <unordered_map>

namespace RFIT_NS::snapshot {

    struct SnapshotData {
        size_t size = 0;
        const uint8_t *data = nullptr;
        int fd = 0;
    };

    class SnapshotRegistry {
    public:
        SnapshotRegistry();

        SnapshotData &getSnapshot(const std::string &key);

        void mapSnapshot(const std::string &key, uint8_t *target);

        void takeSnapshot(const std::string &key,
                          SnapshotData data,
                          bool locallyRestorable = true);

        void deleteSnapshot(const std::string &key);

        size_t getSnapshotCount();

        void clear();

    private:
        std::unordered_map<std::string, SnapshotData> snapshotMap;

        std::mutex snapshotsMx;

        int writeSnapshotToFd(const std::string &key);
    };

    SnapshotRegistry &getSnapshotRegistry();

}
