//
// Created by kingdo on 2021/7/9.
//
#include "utils/rstring.h"

namespace RFIT_NS::utils {
    void split(const std::string &s, std::vector<std::string> &tokens, const std::string &delimiters) {
        std::string::size_type lastPos = s.find_first_not_of(delimiters, 0);
        std::string::size_type pos = s.find_first_of(delimiters, lastPos);
        while (std::string::npos != pos || std::string::npos != lastPos) {
            tokens.push_back(s.substr(lastPos, pos - lastPos));
            lastPos = s.find_first_not_of(delimiters, pos);
            pos = s.find_first_of(delimiters, lastPos);
        }
    }

    bool isAllWhitespace(const std::string &input) {
        return std::all_of(input.begin(), input.end(), isspace);
    }

    bool startsWith(const std::string &input, const std::string &subStr) {
        if (subStr.empty()) {
            return false;
        }

        return input.rfind(subStr, 0) == 0;
    }

    bool endsWith(std::string const &value, std::string const &ending) {
        if (ending.empty()) {
            return false;
        } else if (ending.size() > value.size()) {
            return false;
        }
        return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
    }

    bool contains(const std::string &input, const std::string &subStr) {
        if (input.find(subStr) != std::string::npos) {
            return true;
        } else {
            return false;
        }
    }

    std::string removeSubstr(const std::string &input, const std::string &toErase) {
        std::string output = input;

        size_t pos = output.find(toErase);

        if (pos != std::string::npos) {
            output.erase(pos, toErase.length());
        }

        return output;
    }

    bool stringIsInt(const std::string &input) {
        return !input.empty() &&
               input.find_first_not_of("0123456789") == std::string::npos;
    }
}