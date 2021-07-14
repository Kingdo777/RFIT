#include "endpoint/RFITEndpoint.h"
#include "utils/logging.h"
#include "RFIT/RFIT.h"

using namespace RFIT_NS::endpoint;

int main() {

    const std::shared_ptr<spdlog::logger> &logger = default_logger;
    RFITEndpoint e;
    logger->info("Starting RFIT endpoint");
    e.start();
    logger->info("Shutting HTTP endpoint");
    RFIT_GLOBAL.shutdown();
    return 0;
}
