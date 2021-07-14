#pragma once

#include <unordered_set>

namespace RFIT_NS::utils {
std::string randomString(int len);

std::string randomStringFromSet(const std::unordered_set<std::string>& s);
}
