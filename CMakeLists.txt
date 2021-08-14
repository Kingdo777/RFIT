cmake_minimum_required(VERSION 3.18)
project(RFIT CXX)

# 是否统计时间
option(OPEN_TRACE "If Open Time Trace" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -g -Wall")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(RFIT_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(RFIT_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)


function(rfit_private_lib lib_name lib_deps)
    add_library(${lib_name} STATIC ${lib_deps})
    target_compile_options(${lib_name} PRIVATE "-fPIC")
endfunction()

function(rfit_public_lib lib_name lib_deps)
    add_library(${lib_name} SHARED ${lib_deps})
    target_compile_options(${lib_name} PRIVATE "-fPIC")
endfunction()


if (OPEN_TRACE)
    add_definitions(-DOPEN_TRACE)
endif ()
add_definitions(-DWAVM_API=)
add_definitions(-DPRO_ROOT="/home/kingdo/CLionProjects/RFIT")

include(cmake/ExternalProjects.cmake)

include_directories(
        ${RFIT_INCLUDE_DIR}
        ${PISTACHE_SOURCE_DIR}/include
        ${WAVM_SOURCE_DIR}/Include
        ${WAVM_SOURCE_DIR}/Lib
        ${RAPIDJSON_INCLUDE_DIR}
)

add_subdirectory(src/utils)
add_subdirectory(src/endpoint)
add_subdirectory(src/RFIT)
add_subdirectory(src/proto)
add_subdirectory(src/storage)
add_subdirectory(src/snapshot)
add_subdirectory(src/threads)
add_subdirectory(src/wasm)
add_subdirectory(src/wavm)
#add_subdirectory(Function)

add_executable(main src/main.cpp)
target_link_libraries(main utils endpoint RFIT)

add_subdirectory(test)
