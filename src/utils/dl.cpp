//
// Created by kingdo on 2021/7/14.
//

#include "utils/dl.h"

namespace RFIT_NS::utils {

    void close_remove_DL(const boost::filesystem::path &p, void *handle) {
        if (handle)
            dlclose(handle);
        boost::filesystem::remove(p);
    }

    std::string makeDL(const std::string &dlName, const char *data, size_t dataSize) {
        boost::filesystem::path funPath(FUNC_PATH);
        funPath.append(dlName);
        if (!boost::filesystem::exists(funPath))
            boost::filesystem::create_directories(funPath);
        std::string outputFile = funPath.append("function.so").string();
        std::ofstream out(outputFile, std::ios_base::out | std::ios_base::binary);
        out.write(data, dataSize).flush();
        out.close();
        return outputFile;
    }

    std::pair<dlResult, std::string> getFuncEntry(const std::string &path, const std::string &symbolName) {
        std::pair<dlResult, std::string> res = {{}, ""};
        char *error;
        res.first.handle = dlopen(path.c_str(), RTLD_LAZY);
        if (res.first.handle == nullptr) {
            error = dlerror();
            if (error)
                res.second = error;
            else
                res.second = "handle is null，but unknown the reason";
            close_remove_DL(path);
            return res;
        }
        dlerror();    /* Clear any existing error */
        res.first.addr = dlsym(res.first.handle, getMangledName(symbolName).c_str());
        if (res.first.addr == nullptr) {
            error = dlerror();
            if (error)
                res.second = error;
            close_remove_DL(path, res.first.handle);
            return res;
        }
        if (!res.second.empty())
            close_remove_DL(path, res.first.handle);
        return res;
    }

    std::string getMangledName(const std::string &funcName) {
        size_t len = funcName.size();
        return "_Z" + std::to_string(len) + funcName + "RN7RFIT_NS7MessageE";
    }


}