option(STEAL_INSTANCE "if open the STEAL_INSTANCE" OFF)

file(GLOB HEADERS
#        "${RFIT_INCLUDE_DIR}/RFIT/R/*.h"
#        "${RFIT_INCLUDE_DIR}/RFIT/F/*.h"
#        "${RFIT_INCLUDE_DIR}/RFIT/I/*.h"
#        "${RFIT_INCLUDE_DIR}/RFIT/T/*.h"
        "${RFIT_INCLUDE_DIR}/RFIT/TaskPool/*.h"
        "${RFIT_INCLUDE_DIR}/RFIT/RFIT.h"
        "${RFIT_INCLUDE_DIR}/RFIT/core.h"
        )

file(GLOB SRC_FILES
        "${RFIT_SOURCE_DIR}/RFIT/R/*.cpp"
        "${RFIT_SOURCE_DIR}/RFIT/F/*.cpp"
        "${RFIT_SOURCE_DIR}/RFIT/I/*.cpp"
        "${RFIT_SOURCE_DIR}/RFIT/T/*.cpp"
        "${RFIT_SOURCE_DIR}/RFIT/TaskPool/*.cpp"
        "${RFIT_SOURCE_DIR}/RFIT/RFIT.cpp"
        )
if (STEAL_INSTANCE)
    add_definitions(-DSTEAL_INSTANCE)
endif ()

set(LIB_FILES ${HEADERS} ${SRC_FILES})
rfit_private_lib(RFIT "${LIB_FILES}")
target_link_libraries(RFIT m pthread boost_filesystem  boost_system proto)

