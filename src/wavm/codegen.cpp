#include "WAVMWasmModule.h"

#include <WAVM/IR/Module.h>
#include <WAVM/IR/Types.h>
#include <WAVM/Runtime/Runtime.h>
#include <WAVM/WASM/WASM.h>
#include <WAVM/WASTParse/WASTParse.h>

#include <utils/files.h>
#include <utils/logging.h>

using namespace WAVM;

namespace RFIT_NS::wasm {
    WAVM::Runtime::ModuleRef makeModule(const std::vector<uint8_t> &bytes) {
        auto logger = RFIT_NS::utils::getLogger();
        IR::Module moduleIR;
        // Feature flags
        moduleIR.featureSpec.simd = true;

        if (RFIT_NS::utils::isWasm(bytes)) {
            // Handle WASM
            WASM::LoadError loadError;
            bool success = WASM::loadBinaryModule(
                    bytes.data(), bytes.size(), moduleIR, &loadError);
            if (!success) {
                logger->error("Parse failure: {}", loadError.message);
                throw std::runtime_error("Failed to parse wasm binary");
            }
        } else {
            std::vector<WAST::Error> parseErrors;
            bool success = WAST::parseModule(
                    (const char *) bytes.data(), bytes.size(), moduleIR, parseErrors);
            WAST::reportParseErrors(
                    "wast_file", (const char *) bytes.data(), parseErrors);
            if (!success) {
                throw std::runtime_error("Failed to parse non-wasm file");
            }
        }
        // Compile the module to object code
        Runtime::ModuleRef module = Runtime::compileModule(moduleIR);
        return module;
    }
}
