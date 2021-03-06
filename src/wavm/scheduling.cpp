#include "WAVMWasmModule.h"
#include "syscalls.h"

#include <WAVM/Runtime/Intrinsics.h>

using namespace WAVM;

namespace RFIT_NS::wasm {

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "getpriority",
                               I32,
                               getpriority,
                               I32 a,
                               I32 b)
{
    default_logger->debug("S - getpriority - {} {}", a, b);
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "setpriority",
                               I32,
                               setpriority,
                               I32 a,
                               I32 b,
                               I32 c)
{
    default_logger->debug("S - setpriority - {} {} {}", a, b, c);
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(wasi, "sched_yield", I32, wasi_sched_yield)
{
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

void schedulingLink() {}
}
