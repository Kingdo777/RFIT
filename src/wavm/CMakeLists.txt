include_directories(
        ${RFIT_INCLUDE_DIR}/wavm
        ${WAVM_SOURCE_DIR}/Lib
)

file(GLOB HEADERS "${RFIT_INCLUDE_DIR}/wavm/*.h")

set(LIB_FILES
        WAVMWasmModule.cpp
        LoadedDynamicModule.cpp
        syscalls.h
        codegen.cpp
        dynlink.cpp
        env.cpp
        rfit.cpp
        io.cpp
        libcxx.cpp
        maths.cpp
        memory.cpp
        messages.cpp
        network.cpp
        process.cpp
        scheduling.cpp
        signals.cpp
        syscalls.cpp
        timing.cpp
        util.cpp
        threads.cpp
        ${HEADERS}
        )

rfit_private_lib(wavmmodule "${LIB_FILES}")
target_link_libraries(wavmmodule wasm libWAVM storage)
