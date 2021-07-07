file(GLOB HEADERS
        "${RFIT_INCLUDE_DIR}/utils/*.h"
        )

file(GLOB SRC_FILES
        "${RFIT_SOURCE_DIR}/utils/*.cpp"
        )

set(LIB_FILES ${HEADERS} ${SRC_FILES})
rfit_private_lib(utils "${LIB_FILES}")

