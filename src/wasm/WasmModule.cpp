#include "wasm/WasmModule.h"

#include <snapshot/SnapshotRegistry.h>
#include <utils/bytes.h>
#include <utils/config.h>
#include <utils/environment.h>
#include <utils/gids.h>
#include <utils/locks.h>
#include <storage/FileSystem.h>
#include <utils/memory.h>
#include <utils/timing.h>

#include <boost/filesystem.hpp>
#include <sstream>
#include <sys/mman.h>
#include <sys/uio.h>
#include <threads/MutexManager.h>

namespace RFIT_NS::wasm {
// Using TLS here to isolate between executing functions
    static thread_local wasm::WasmModule *executingModule;
    static thread_local RFIT_NS::Message *executingMsg;

    bool isWasmPageAligned(int32_t offset) {
        if (offset & (WASM_BYTES_PER_PAGE - 1)) {
            return false;
        } else {
            return true;
        }
    }

    wasm::WasmModule *getExecutingModule() {
        return executingModule;
    }

    void setExecutingModule(wasm::WasmModule *module) {
        executingModule = module;
    }

    RFIT_NS::Message  *getExecutingMsg() {
        return executingMsg;
    }

    void setExecutingMsg(RFIT_NS::Message *msg) {
        executingMsg = msg;
    }

    size_t getNumberOfWasmPagesForBytes(uint32_t nBytes) {
        // Round up to nearest page
        size_t pageCount =
                (size_t(nBytes) + WASM_BYTES_PER_PAGE - 1) / WASM_BYTES_PER_PAGE;

        return pageCount;
    }

    uint32_t roundUpToWasmPageAligned(uint32_t nBytes) {
        size_t nPages = getNumberOfWasmPagesForBytes(nBytes);
        return (uint32_t) (nPages * WASM_BYTES_PER_PAGE);
    }

    size_t getPagesForGuardRegion() {
        size_t regionSize = GUARD_REGION_SIZE;
        size_t nWasmPages = getNumberOfWasmPagesForBytes(regionSize);
        return nWasmPages;
    }

    WasmModule::WasmModule() = default;

    WasmModule::~WasmModule() = default;

    void WasmModule::flush() {}

    storage::FileSystem &WasmModule::getFileSystem() {
        return filesystem;
    }

    wasm::WasmEnvironment &WasmModule::getWasmEnvironment() {
        return wasmEnvironment;
    }

    std::string WasmModule::snapshot(bool locallyRestorable) {
        PROF_START(wasmSnapshot)

        // Create snapshot key
        uint32_t gid = RFIT_NS::utils::generateGid();
        std::string snapKey =
                this->boundUser + "_" + this->boundFunction + "_" + std::to_string(gid);

        // Note - we only want to take the snapshot to the current brk, not the top
        // of the allocated memory
        RFIT_NS::snapshot::SnapshotData data;
        data.data = getMemoryBase();
        data.size = getCurrentBrk();

        RFIT_NS::snapshot::SnapshotRegistry &reg =
                RFIT_NS::snapshot::getSnapshotRegistry();
        reg.takeSnapshot(snapKey, data, locallyRestorable);

        PROF_END(wasmSnapshot)

        return snapKey;
    }

    void WasmModule::restore(const std::string &snapshotKey) {
        PROF_START(wasmSnapshotRestore)

        auto logger = RFIT_NS::utils::getLogger();
        RFIT_NS::snapshot::SnapshotRegistry &reg =
                RFIT_NS::snapshot::getSnapshotRegistry();

        // Expand memory if necessary
        RFIT_NS::snapshot::SnapshotData data = reg.getSnapshot(snapshotKey);
        uint32_t memSize = getCurrentBrk();

        if (data.size == memSize) {
            logger->debug("Snapshot memory size equal to current memory");
        } else if (data.size > memSize) {
            logger->debug("Growing memory to fit snapshot");
            size_t bytesRequired = data.size - memSize;
            this->growMemory(bytesRequired);
        } else {
            logger->debug("Shrinking memory to fit snapshot");
            size_t shrinkBy = memSize - data.size;
            this->shrinkMemory(shrinkBy);
        }

        // Map the snapshot into memory
        uint8_t *memoryBase = getMemoryBase();
        reg.mapSnapshot(snapshotKey, memoryBase);

        PROF_END(wasmSnapshotRestore)
    }

    int WasmModule::getStdoutFd() {
        if (stdoutMemFd == 0) {
            stdoutMemFd = memfd_create("stdoutfd", 0);
            RFIT_NS::utils::getLogger()->debug("Capturing stdout: fd={}",
                                               stdoutMemFd);
        }

        return stdoutMemFd;
    }

    ssize_t WasmModule::captureStdout(const struct iovec *iovecs, int iovecCount) {
        int memFd = getStdoutFd();
        ssize_t writtenSize = ::writev(memFd, iovecs, iovecCount);

        if (writtenSize < 0) {
            RFIT_NS::utils::getLogger()->error("Failed capturing stdout: {}",
                                               strerror(errno));
            throw std::runtime_error(std::string("Failed capturing stdout: ") +
                                     strerror(errno));
        }

        RFIT_NS::utils::getLogger()->debug("Captured {} bytes of formatted stdout",
                                           writtenSize);
        stdoutSize += writtenSize;
        return writtenSize;
    }

    ssize_t WasmModule::captureStdout(const void *buffer) {
        int memFd = getStdoutFd();

        ssize_t writtenSize =
                dprintf(memFd, "%s\n", reinterpret_cast<const char *>(buffer));

        if (writtenSize < 0) {
            RFIT_NS::utils::getLogger()->error("Failed capturing stdout: {}",
                                               strerror(errno));
            throw std::runtime_error("Failed capturing stdout");
        }

        RFIT_NS::utils::getLogger()->debug("Captured {} bytes of unformatted stdout",
                                           writtenSize);
        stdoutSize += writtenSize;
        return writtenSize;
    }

    std::string WasmModule::getCapturedStdout() {
        if (stdoutSize == 0) {
            return "";
        }

        // Go back to start
        int memFd = getStdoutFd();
        lseek(memFd, 0, SEEK_SET);

        // Read in and return
        char *buf = new char[stdoutSize];
        read(memFd, buf, stdoutSize);
        std::string stdoutString(buf, stdoutSize);
        RFIT_NS::utils::getLogger()->debug(
                "Read stdout length {}:\n{}", stdoutSize, stdoutString);

        return stdoutString;
    }

    void WasmModule::clearCapturedStdout() {
        close(stdoutMemFd);
        stdoutMemFd = 0;
        stdoutSize = 0;
    }

    uint32_t WasmModule::getArgc() {
        return argc;
    }

    uint32_t WasmModule::getArgvBufferSize() {
        return argvBufferSize;
    }

    void WasmModule::prepareArgcArgv() {
        // Here we set up the arguments to main(), i.e. argc and argv
        // We allow passing of arbitrary commandline arguments via the invocation
        // message. These are passed as a string with a space separating each
        // argument.
        argv = {"function.wasm"};
        argc = argv.size();

        // Work out the size of the buffer to hold the strings (allowing
        // for null terminators)
        argvBufferSize = 0;
        for (const auto &thisArg : argv) {
            argvBufferSize += thisArg.size() + 1;
        }
    }

    uint32_t WasmModule::getCurrentBrk() {
        RFIT_NS::utils::ReadLock lock(moduleMemoryMutex);
        return currentBrk;
    }

    uint32_t WasmModule::createMemoryGuardRegion(uint32_t wasmOffset) {
        auto logger = RFIT_NS::utils::getLogger();

        uint32_t regionSize = GUARD_REGION_SIZE;
        uint8_t *nativePtr = wasmPointerToNative(wasmOffset);

        // NOTE: we want to protect these regions from _writes_, but we don't
        // want to stop them being read, otherwise snapshotting will fail.
        // Therefore we make them read-only
        int res = mprotect(nativePtr, regionSize, PROT_READ);
        if (res != 0) {
            logger->error("Failed to create memory guard: {}",
                          std::strerror(errno));
            throw std::runtime_error("Failed to create memory guard");
        }

        logger->debug(
                "Created guard region {}-{}", wasmOffset, wasmOffset + regionSize);

        return wasmOffset + regionSize;
    }

    threads::MutexManager& WasmModule::getMutexes()
    {
        return mutexes;
    }

// ------------------------------------------
// Functions to be implemented by subclasses
// ------------------------------------------

    void WasmModule::bindToFunction() {
        throw std::runtime_error("execute not implemented");
    }

    void WasmModule::bindToFunctionNoZygote() {
        throw std::runtime_error("execute not implemented");
    }

    bool WasmModule::execute(RFIT_NS::Message &msg) {
        throw std::runtime_error("execute not implemented");
    }

    bool WasmModule::execute() {
        throw std::runtime_error("execute not implemented");
    }

    bool WasmModule::isBound() {
        throw std::runtime_error("isBound not implemented");
    }

    void WasmModule::writeArgvToMemory(uint32_t wasmArgvPointers,
                                       uint32_t wasmArgvBuffer) {
        throw std::runtime_error("writeArgvToMemory not implemented");
    }

    void WasmModule::writeWasmEnvToMemory(uint32_t envPointers, uint32_t envBuffer) {
        throw std::runtime_error("writeWasmEnvToMemory not implemented");
    }

    uint32_t WasmModule::growMemory(uint32_t nBytes) {
        throw std::runtime_error("growMemory not implemented");
    }

    uint32_t WasmModule::shrinkMemory(uint32_t nBytes) {
        throw std::runtime_error("shrinkMemory not implemented");
    }

    uint32_t WasmModule::mmapMemory(uint32_t nBytes) {
        throw std::runtime_error("mmapMemory not implemented");
    }

    uint32_t WasmModule::mmapFile(uint32_t fp, uint32_t length) {
        throw std::runtime_error("mmapFile not implemented");
    }

    void WasmModule::unmapMemory(uint32_t offset, uint32_t nBytes) {
        throw std::runtime_error("unmapMemory not implemented");
    }

    uint8_t *WasmModule::wasmPointerToNative(int32_t wasmPtr) {
        throw std::runtime_error("wasmPointerToNative not implemented");
    }

    void WasmModule::printDebugInfo() {
        throw std::runtime_error("printDebugInfo not implemented");
    }

    size_t WasmModule::getMemorySizeBytes() {
        throw std::runtime_error("getMemorySizeBytes not implemented");
    }

    uint8_t *WasmModule::getMemoryBase() {
        throw std::runtime_error("getMemoryBase not implemented");
    }

    string WasmModule::getFuncStr() {
        assert(isBound());
        return boundUser + "-" + boundFunction;
    }
}
