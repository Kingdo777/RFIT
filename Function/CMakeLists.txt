cmake_minimum_required(VERSION 3.18)
project(RFIT-func)

set(CMAKE_CXX_STANDARD 17)

set(FUNCTION_DIR ${CMAKE_CURRENT_SOURCE_DIR}/code)
set(LIB_DIR ../cmake-build-debug/lib)
set(INCLUDE_DIR ../include)

message(STATUS ${LIB_DIR})

function(rfit_fun lib_name)
    add_library(${lib_name} SHARED ${FUNCTION_DIR}/${lib_name}.cpp)
    target_compile_options(${lib_name} PRIVATE "-fPIC")
    target_link_directories(${lib_name} PUBLIC ${LIB_DIR})
    target_link_libraries(${lib_name} proto utils)
    target_include_directories(${lib_name} PUBLIC ${INCLUDE_DIR})
endfunction()

rfit_fun(hello)
rfit_fun(hello1)




