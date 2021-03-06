#pragma once

#include <boost/filesystem.hpp>
#include <openssl/md5.h>
#include <utils/exception.h>
#include <string>
#include <vector>

namespace RFIT_NS::utils {
    std::string readFileToString(const boost::filesystem::path &path);

    std::vector<uint8_t> readFileToBytes(const boost::filesystem::path &path);

    void writeBytesToFile(const boost::filesystem::path &path,
                          const std::vector<uint8_t> &data);

    void writeStringToFile(const boost::filesystem::path &path,
                           const std::string &data);

    std::vector<uint8_t> hashBytes(const char *data, size_t size);

    bool isWasm(const std::vector<uint8_t> &bytes);

    bool isWasm(const std::string &file);
}
