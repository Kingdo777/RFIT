
include(FindProtobuf)
find_package(Protobuf REQUIRED)

message(STATUS "Using protobuf  \
    ${PROTOBUF_LIBRARY} \
    ${PROTOBUF_PROTOC_LIBRARY} \
    ${PROTOBUF_PROTOC_EXECUTABLE} \
")

set(PB_HEADER "${CMAKE_CURRENT_BINARY_DIR}/rfit.pb.h")

set(PB_HEADER_COPIED "${RFIT_INCLUDE_DIR}/proto/rfit.pb.h")

set(PB_SRC "${CMAKE_CURRENT_BINARY_DIR}/rfit.pb.cc")

add_custom_command(
        OUTPUT "${PB_HEADER}" "${PB_SRC}"
        COMMAND protoc
        ARGS --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${CMAKE_CURRENT_LIST_DIR}"
        rfit.proto
        DEPENDS rfit.proto
)

# Copy the generated headers into place
add_custom_command(
        OUTPUT "${PB_HEADER_COPIED}"
        DEPENDS "${PB_HEADER}"
        COMMAND ${CMAKE_COMMAND} -E copy
        ${PB_HEADER}
        ${RFIT_INCLUDE_DIR}/proto/
)


# ----------------------------------------------
# Faabric wrapper library
# ----------------------------------------------

set(HEADERS
        ${PB_HEADER_COPIED}
        )

set(LIB_FILES
        ${HEADERS}
        ${PB_SRC}
        )

add_library(proto "${LIB_FILES}")

target_link_libraries(proto ${PROTOBUF_LIBRARY})

