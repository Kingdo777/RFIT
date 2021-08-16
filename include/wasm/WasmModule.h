#pragma once

#include "WasmEnvironment.h"
#include <proto/rfit.pb.h>
#include <utils/logging.h>
#include <utils/memory.h>
#include <utils/queue.h>

#include <exception>
#include <future>
#include <mutex>
#include <string>
#include <thread>
#include <tuple>
#include <storage/FileSystem.h>
#include <threads/MutexManager.h>


#define ONE_MB_BYTES (1024 * 1024)

#define WASM_BYTES_PER_PAGE 65536

// Note: this is *not* controlling the size provisioned by the linker, that is
// hard-coded in the build. This variable is just here for reference and must be
// updated to match the value in the build.
#define STACK_SIZE (4 * ONE_MB_BYTES)
#define THREAD_STACK_SIZE (2 * ONE_MB_BYTES)

// Properties of dynamic modules. Heap size must be wasm-module-page-aligned.
// One page is 64kB
#define DYNAMIC_MODULE_STACK_SIZE (2 * ONE_MB_BYTES)
#define DYNAMIC_MODULE_MEMORY_SIZE (66 * WASM_BYTES_PER_PAGE)
#define GUARD_REGION_SIZE (10 * WASM_BYTES_PER_PAGE)

// Special known function names
// Zygote function (must match rfit.h linked into the functions themselves)
#define ZYGOTE_FUNC_NAME "_rfit_zygote"
#define WASM_CTORS_FUNC_NAME "__wasm_call_ctors"
#define ENTRY_FUNC_NAME "_start"

namespace RFIT_NS::wasm {

    bool isWasmPageAligned(int32_t offset);

    class WasmModule {
    public:
        WasmModule();

        virtual ~WasmModule();

        string getFuncStr();

        // ----- Module lifecycle -----
        virtual void bindToFunction();

        virtual void bindToFunctionNoZygote();

        virtual bool execute(RFIT_NS::Message &msg);

        virtual bool execute();

        virtual bool isBound();

        virtual void flush();

        // ----- argc/ argv -----
        uint32_t getArgc();

        uint32_t getArgvBufferSize();

        virtual void writeArgvToMemory(uint32_t wasmArgvPointers,
                                       uint32_t wasmArgvBuffer);

        // ----- Environment variables
        virtual void writeWasmEnvToMemory(uint32_t envPointers, uint32_t envBuffer);

        WasmEnvironment &getWasmEnvironment();

        // ----- Filesystem -----
        storage::FileSystem &getFileSystem();

        // ----- Stdout capture -----
        ssize_t captureStdout(const struct iovec *iovecs, int iovecCount);

        ssize_t captureStdout(const void *buffer);

        std::string getCapturedStdout();

        void clearCapturedStdout();

        // ----- Memory management -----
        uint32_t getCurrentBrk();

        virtual uint32_t growMemory(uint32_t nBytes);

        virtual uint32_t shrinkMemory(uint32_t nBytes);

        virtual uint32_t mmapMemory(uint32_t nBytes);

        virtual uint32_t mmapFile(uint32_t fp, uint32_t length);

        virtual void unmapMemory(uint32_t offset, uint32_t nBytes);

        uint32_t createMemoryGuardRegion(uint32_t wasmOffset);

        virtual uint8_t *wasmPointerToNative(int32_t wasmPtr);

        virtual size_t getMemorySizeBytes();

        threads::MutexManager &getMutexes();

        // ----- Snapshot/ restore -----
        std::string snapshot(bool locallyRestorable = true);

        void restore(const std::string &snapshotKey);

        // ----- Debugging -----
        virtual void printDebugInfo();

    protected:

        std::string boundUser;

        std::string boundFunction;

        uint32_t currentBrk = 0;

        storage::FileSystem filesystem;

        WasmEnvironment wasmEnvironment;

        int stdoutMemFd;
        ssize_t stdoutSize;

        std::shared_mutex moduleMemoryMutex;

        threads::MutexManager mutexes;

        // Argc/argv
        unsigned int argc;
        std::vector<std::string> argv;
        size_t argvBufferSize;

        // Shared memory regions
        std::unordered_map<std::string, uint32_t> sharedMemWasmPtrs;

        int getStdoutFd();

        void prepareArgcArgv();

        virtual uint8_t *getMemoryBase();
    };

// ----- Global functions -----

    void setExecutingModule(wasm::WasmModule *module);

    wasm::WasmModule *getExecutingModule();

    RFIT_NS::Message *getExecutingMsg();

    void setExecutingMsg(RFIT_NS::Message *msg);

// Convenience functions
    size_t getNumberOfWasmPagesForBytes(uint32_t nBytes);

    uint32_t roundUpToWasmPageAligned(uint32_t nBytes);

    size_t getPagesForGuardRegion();

/*
 * Exception thrown when wasm module terminates
 */
    class WasmExitException : public std::exception {
    public:
        explicit WasmExitException(int exitCode)
                : exitCode(exitCode) {}

        int exitCode;
    };

}
