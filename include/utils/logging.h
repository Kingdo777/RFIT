#pragma once

#include <spdlog/spdlog.h>

#define default_logger RFIT_NS::utils::getLogger()

namespace RFIT_NS::utils {
    std::shared_ptr<spdlog::logger> &getLogger(const std::string &name = "default");
}
