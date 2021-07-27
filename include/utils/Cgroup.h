//
// Created by kingdo on 2021/7/21.
//

#ifndef RFIT_CGROUP_H
#define RFIT_CGROUP_H

#include <string>

#define BASE_CGROUP_NAME "RFIT"
#define BASE_DIR  "/sys/fs/cgroup/"
#define CG_CPU  "cpu"

namespace RFIT_NS::isolation {

    class CGroup {
    public:
        explicit CGroup(std::string name);

        void addCurrentThread();

        std::string getName();

    private:
        std::string name;
    };
}


#endif //RFIT_CGROUP_H
