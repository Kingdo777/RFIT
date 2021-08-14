include_directories(
        ${RFIT_INCLUDE_DIR}/storage
        ${WAVM_SOURCE_DIR}/Lib
)

file(GLOB HEADERS
        "${RFIT_INCLUDE_DIR}/storage/FileDescriptor.h"
        "${RFIT_INCLUDE_DIR}/storage/FileSystem.h"
#        "${RFIT_INCLUDE_DIR}/storage/FileLoader.h"
#        "${RFIT_INCLUDE_DIR}/storage/LocalFileLoader.h"
        )

set(LIB_FILES
        FileDescriptor.cpp
        #        FileLoader.cpp
        FileSystem.cpp
        #        LocalFileLoader.cpp
        #        instance.cpp
        ${HEADERS}
        )

rfit_private_lib(storage "${LIB_FILES}")
