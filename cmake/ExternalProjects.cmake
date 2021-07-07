include(FindGit)
find_package(Git)
include(ExternalProject)
include(FetchContent)

# WAVM
FetchContent_Declare(wavm_ext
        GIT_REPOSITORY "https://github.com/Kingdo777/WAVM.git"
        CMAKE_ARGS "-DDLL_EXPORT= \
        -DDLL_IMPORT="
        )
# Pistache
FetchContent_Declare(pistache_ext
        GIT_REPOSITORY "https://github.com/Kingdo777/pistache.git"
        CMAKE_CACHE_ARGS "-DCMAKE_INSTALL_PREFIX:STRING=${CMAKE_INSTALL_PREFIX}"
        BUILD_BYPRODUCTS ${CMAKE_INSTALL_PREFIX}/lib/libpistache.so
        )

# spdlog
FetchContent_Declare(spdlog_ext
        GIT_REPOSITORY "https://github.com/gabime/spdlog"
        GIT_TAG "v1.8.0"
        CMAKE_CACHE_ARGS "-DCMAKE_INSTALL_PREFIX:STRING=${CMAKE_INSTALL_PREFIX}"
        )
# RapidJSON
FetchContent_Declare(rapidjson_ext
        GIT_REPOSITORY "https://github.com/Tencent/rapidjson"
        GIT_TAG "2ce91b823c8b4504b9c40f99abf00917641cef6c"
        CMAKE_CACHE_ARGS "-DCMAKE_INSTALL_PREFIX:STRING=${CMAKE_INSTALL_PREFIX}"
        )
FetchContent_MakeAvailable(wavm_ext pistache_ext spdlog_ext rapidjson_ext)

FetchContent_GetProperties(wavm_ext SOURCE_DIR WAVM_SOURCE_DIR)
FetchContent_GetProperties(pistache_ext SOURCE_DIR PISTACHE_SOURCE_DIR)
FetchContent_GetProperties(spdlog_ext SOURCE_DIR SPDLOG_SOURCE_DIR)
FetchContent_GetProperties(rapidjson_ext SOURCE_DIR RAPIDJSON_SOURCE_DIR)
set(RAPIDJSON_INCLUDE_DIR ${RAPIDJSON_SOURCE_DIR}/include)

message(STATUS WAVM_SOURCE_DIR:${WAVM_SOURCE_DIR})
message(STATUS PISTACHE_SOURCE_DIR:${PISTACHE_SOURCE_DIR})
message(STATUS RAPIDJSON_INCLUDE_DIR:${RAPIDJSON_INCLUDE_DIR})
message(STATUS SPDLOG_SOURCE_DIR:${SPDLOG_SOURCE_DIR})