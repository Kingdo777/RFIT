#pragma once

#include <spdlog/spdlog.h>

#define default_logger RFIT::utils::getLogger()

namespace RFIT::utils {
    std::shared_ptr<spdlog::logger> &getLogger(const std::string &name = "default");
}
