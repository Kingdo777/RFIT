//
// Created by kingdo on 2021/7/9.
//
#include <string>
#include <vector>

using namespace std;
namespace RFIT_NS::utils {
    void split(const string &s, vector<string> &tokens, const string &delimiters = " ") {
        string::size_type lastPos = s.find_first_not_of(delimiters, 0);
        string::size_type pos = s.find_first_of(delimiters, lastPos);
        while (string::npos != pos || string::npos != lastPos) {
            tokens.push_back(s.substr(lastPos, pos - lastPos));
            lastPos = s.find_first_not_of(delimiters, pos);
            pos = s.find_first_of(delimiters, lastPos);
        }
    }
}