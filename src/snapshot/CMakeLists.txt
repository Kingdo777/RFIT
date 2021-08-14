file(GLOB HEADERS "${RFIT_INCLUDE_DIR}/snapshot/*.h")

set(LIB_FILES
    SnapshotRegistry.cpp
    ${HEADERS}
)

rfit_private_lib(snapshot "${LIB_FILES}")

target_link_libraries(snapshot proto utils)
