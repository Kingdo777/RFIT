include_directories(
        ${RFIT_INCLUDE_DIR}/wasm
)

set(HEADERS
        "${RFIT_INCLUDE_DIR}/wasm/WasmEnvironment.h"
        "${RFIT_INCLUDE_DIR}/wasm/WasmModule.h"
        )

set(LIB_FILES
        WasmEnvironment.cpp
        WasmModule.cpp
        ${HEADERS}
        )

rfit_private_lib(wasm "${LIB_FILES}")
target_link_libraries(wasm utils snapshot storage threads)
