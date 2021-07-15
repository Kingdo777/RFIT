//
// Created by kingdo on 2021/7/14.
//

#ifndef RFIT_DL_H
#define RFIT_DL_H

#include <boost/filesystem.hpp>
#include <dlfcn.h>
#include <string>
#include <fstream>

#define FUNC_PATH "/home/kingdo/CLionProjects/RFIT/Function/lib"

namespace RFIT_NS::utils {
    struct dlResult {
        void *handle;
        void *addr;
    };

    std::pair<dlResult, std::string> getFuncEntry(const std::string &path, const std::string &symbolName);

    std::string makeDL(const std::string &dlName, char const *data, size_t dataSize);

    std::string getMangledName(const std::string &funcName);

}
#endif //RFIT_DL_H
