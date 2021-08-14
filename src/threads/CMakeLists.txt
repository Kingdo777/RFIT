file(GLOB HEADERS "${RFIT_INCLUDE_DIR}/threads/*.h")

set(LIB_FILES
        MutexManager.cpp
        ${HEADERS}
        )

rfit_private_lib(threads "${LIB_FILES}")
target_link_libraries(threads utils)
