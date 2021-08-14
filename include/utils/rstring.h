//
// Created by kingdo on 2021/7/9.
//

#ifndef RFIT_STRING_H
#define RFIT_STRING_H

#include <algorithm>
#include <string>
#include <vector>

namespace RFIT_NS::utils {

    void split(const std::string &s, std::vector<std::string> &tokens, const std::string &delimiters);


    bool isAllWhitespace(const std::string &input);

    bool startsWith(const std::string &input, const std::string &subStr);

    bool endsWith(const std::string &input, const std::string &subStr);

    bool contains(const std::string &input, const std::string &subStr);

    std::string removeSubstr(const std::string &input, const std::string &toErase);

    bool stringIsInt(const std::string &input);
}

#endif //RFIT_STRING_H