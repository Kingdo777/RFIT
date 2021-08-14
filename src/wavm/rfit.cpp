#include "WAVMWasmModule.h"
#include "syscalls.h"

#include <WAVM/Platform/Diagnostics.h>
#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/Runtime/Runtime.h>

#include <utils/bytes.h>
#include <utils/files.h>

using namespace WAVM;

namespace RFIT_NS::wasm {

    void rfitLink() {}


    I32 _readInputImpl(I32 bufferPtr, I32 bufferLen) {
        // Get the input
        RFIT_NS::Message *call = getExecutingMsg();
        std::vector<uint8_t> inputBytes =
                RFIT_NS::utils::stringToBytes(call->inputdata());

        // If nothing, return nothing
        if (inputBytes.empty()) {
            return 0;
        }

        // Write to the wasm buffer
        Runtime::Memory *memoryPtr = getExecutingWAVMModule()->defaultMemory;
        U8 *buffer =
                Runtime::memoryArrayPtr<U8>(memoryPtr, (Uptr) bufferPtr, (Uptr) bufferLen);

        int inputSize =
                RFIT_NS::utils::safeCopyToBuffer(inputBytes, buffer, bufferLen);
        return inputSize;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                                   "__rfit_read_input",
                                   I32,
                                   __rfit_read_input,
                                   I32 bufferPtr,
                                   I32 bufferLen) {
        default_logger->debug(
                "S - read_input - {} {}", bufferPtr, bufferLen);

        return _readInputImpl(bufferPtr, bufferLen);
    }

    void _writeOutputImpl(I32 outputPtr, I32 outputLen) {
        std::vector<uint8_t> outputData = getBytesFromWasm(outputPtr, outputLen);
        RFIT_NS::Message *call = getExecutingMsg();
        call->set_outputdata(outputData.data(), outputData.size());
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                                   "__rfit_write_output",
                                   void,
                                   __rfit_write_output,
                                   I32 outputPtr,
                                   I32 outputLen) {
        default_logger->debug(
                "S - write_output - {} {}", outputPtr, outputLen);
        _writeOutputImpl(outputPtr, outputLen);
    }

    void _readPythonInput(I32 buffPtr, I32 buffLen, const std::string &value) {
        // Get wasm buffer
        U8 *buffer = Runtime::memoryArrayPtr<U8>(
                getExecutingWAVMModule()->defaultMemory, (Uptr) buffPtr, (Uptr) buffLen);

        if (value.empty()) {
            // If nothing, just write a null terminator
            buffer[0] = '\0';
        } else {
            // Copy value into WASM
            std::vector<uint8_t> bytes = RFIT_NS::utils::stringToBytes(value);
            std::copy(bytes.begin(), bytes.end(), buffer);

            // Add null terminator
            buffer[value.size()] = '\0';
        }
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                                   "__rfit_get_py_user",
                                   void,
                                   __rfit_get_py_user,
                                   I32 bufferPtr,
                                   I32 bufferLen) {
        default_logger->debug(
                "S - get_py_user - {} {}", bufferPtr, bufferLen);
//        std::string value = getExecutingMsg()->pythonuser();
        std::string value = "python";
        if (value.empty()) {
            throw std::runtime_error("Python user empty, cannot return");
        }

        _readPythonInput(bufferPtr, bufferLen, value);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                                   "__rfit_get_py_func",
                                   void,
                                   __rfit_get_py_func,
                                   I32 bufferPtr,
                                   I32 bufferLen) {
        default_logger->debug(
                "S - get_py_func - {} {}", bufferPtr, bufferLen);
//        std::string value = getExecutingMsg()->pythonfunction();
        std::string value = "hello";
        _readPythonInput(bufferPtr, bufferLen, value);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                                   "__rfit_get_py_entry",
                                   void,
                                   __rfit_get_py_entry,
                                   I32 bufferPtr,
                                   I32 bufferLen) {
        default_logger->debug(
                "S - get_py_entry - {} {}", bufferPtr, bufferLen);
//        std::string value = getExecutingMsg()->pythonentry();
        std::string value = "entry";
        _readPythonInput(bufferPtr, bufferLen, value);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                                   "__rfit_conf_flag",
                                   U32,
                                   __rfit_conf_flag,
                                   I32 keyPtr) {
        const std::shared_ptr<spdlog::logger> &logger = default_logger;
        const std::string key = getStringFromWasm(keyPtr);
        logger->debug("S - conf_flag - {}", key);

        RFIT_NS::utils::SystemConfig &conf = RFIT_NS::utils::getSystemConfig();

        if (key == "PYTHON_PRELOAD") {
//            int res = conf.pythonPreload == "on" ? 1 : 0;
            int res = 0;
            return res;
        } else if (key == "ALWAYS_ON") {
            // For testing
            return 1;
        } else if (key == "ALWAYS_OFF") {
            // For testing
            return 0;
        } else {
            logger->warn("Unknown conf flag: {}", key);
            return 0;
        }
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                                   "__rfit_backtrace",
                                   void,
                                   __rfit_backtrace,
                                   I32 depth) {
        auto logger = default_logger;
        logger->debug("S - rfit_backtrace {}", depth);

        WAVMWasmModule *module = getExecutingWAVMModule();
        module->printDebugInfo();

        Platform::CallStack callStack = Platform::captureCallStack(depth);
        std::vector<std::string> callStackStrs =
                Runtime::describeCallStack(callStack);

        printf("\n------ rfit backtrace ------\n");
        for (auto s : callStackStrs) {
            printf("%s\n", s.c_str());
        }
        printf("-------------------------------\n");

        fflush(stdout);
    }

// 02/12/20 - unfortunately some old Python wasm still needs this
// Emulator API, should not be called from wasm but needs to be present for
// linking
    WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                                   "setEmulatedMessageFromJson",
                                   I32,
                                   setEmulatedMessageFromJson,
                                   I32 msgPtr) {
        default_logger->debug("S - setEmulatedMessageFromJson - {}",
                              msgPtr);
        throw std::runtime_error(
                "Should not be calling emulator functions from wasm");
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                                   "emulatorGetAsyncResponse",
                                   I32,
                                   emulatorGetAsyncResponse) {
        default_logger->debug("S - emulatorGetAsyncResponse");
        throw std::runtime_error(
                "Should not be calling emulator functions from wasm");
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                                   "emulatorSetCallStatus",
                                   void,
                                   emulatorSetCallStatus,
                                   I32 success) {
        default_logger->debug("S - emulatorSetCallStatus {}", success);
        throw std::runtime_error(
                "Should not be calling emulator functions from wasm");
    }
}
