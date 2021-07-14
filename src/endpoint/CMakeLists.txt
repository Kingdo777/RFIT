file(GLOB HEADERS
        "${RFIT_INCLUDE_DIR}/endpoint/*.h"
        )

file(GLOB SRC_FILES
        "${RFIT_SOURCE_DIR}/endpoint/*.cpp"
        )

set(LIB_FILES ${HEADERS} ${SRC_FILES})
rfit_private_lib(endpoint "${LIB_FILES}")
target_include_directories(endpoint PRIVATE ${PISTACHE_SOURCE_DIR}/include)
target_link_libraries(endpoint pistache pthread RFIT utils)
