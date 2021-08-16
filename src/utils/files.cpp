#include <utils/bytes.h>
#include <utils/files.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace RFIT_NS::utils {
    std::string readFileToString(const boost::filesystem::path &path) {
        if (!boost::filesystem::exists(path))
            return "";

        std::ifstream stream(path);
        std::stringstream buffer;
        buffer << stream.rdbuf();
        buffer.flush();
        return buffer.str();
    }

    std::vector<uint8_t> readFileToBytes(const boost::filesystem::path &path) {
        if (!boost::filesystem::exists(path))
            return {};

        std::ifstream file(path, std::ios::binary);
        // Stop eating new lines in binary mode
        file.unsetf(std::ios::skipws);

        // Reserve space
        std::streampos fileSize;
        file.seekg(0, std::ios::end);
        fileSize = file.tellg();

        std::vector<uint8_t> result;
        result.reserve(fileSize);

        // Read the data
        file.seekg(0, std::ios::beg);
        result.insert(result.begin(),
                      std::istreambuf_iterator<char>(file),
                      std::istreambuf_iterator<char>());

        return result;
    }

    void doWrite(const boost::filesystem::path &path, char *data, size_t size) {
        if (!boost::filesystem::exists(path.parent_path()))
            boost::filesystem::create_directories(path.parent_path());
        std::ofstream outfile;
        outfile.open(path, std::ios::out | std::ios::binary);
        if (!outfile.is_open()) {
            throw std::runtime_error("Could not write to file " + path.string());
        }
        outfile.write((char *) data, size);
        outfile.close();
    }

    void writeBytesToFile(const boost::filesystem::path &path,
                          const std::vector<uint8_t> &data) {
        doWrite(path, (char *) data.data(), data.size());
    }

    void writeStringToFile(const boost::filesystem::path &path,
                           const std::string &data) {
        doWrite(path, (char *) data.data(), data.size());
    }

    bool isWasm(const std::vector<uint8_t> &bytes) {
        static const uint8_t wasmMagicNumber[4] = {0x00, 0x61, 0x73, 0x6d};
        if (bytes.size() >= 4 && !memcmp(bytes.data(), wasmMagicNumber, 4)) {
            return true;
        } else {
            return false;
        }
    }

    bool isWasm(const std::string &file) {
        static const uint8_t wasmMagicNumber[4] = {0x00, 0x61, 0x73, 0x6d};
        if (file.size() >= 4 && !memcmp(file.data(), wasmMagicNumber, 4)) {
            return true;
        } else {
            return false;
        }
    }

    std::vector<uint8_t> hashBytes(const char *data, size_t size) {
        std::vector<uint8_t> result(MD5_DIGEST_LENGTH);
        MD5(reinterpret_cast<const unsigned char *>(data),
            size,
            result.data());

        return result;
    }
}
