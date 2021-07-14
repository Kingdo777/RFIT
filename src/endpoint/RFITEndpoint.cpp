//
// Created by kingdo on 2021/7/7.
//

#include <csignal>
#include "endpoint/RFITEndpoint.h"
#include "endpoint/RFITEndpointHandler.h"
#include "utils//logging.h"

namespace RFIT_NS::endpoint {
    void RFITEndpoint::start() const {
        const std::shared_ptr<spdlog::logger> &logger = default_logger;

        // Set up signal handler
        sigset_t signals;
        if (sigemptyset(&signals) != 0 || sigaddset(&signals, SIGTERM) != 0 ||
            sigaddset(&signals, SIGKILL) != 0 || sigaddset(&signals, SIGINT) != 0 ||
            sigaddset(&signals, SIGHUP) != 0 || sigaddset(&signals, SIGQUIT) != 0 ||
            pthread_sigmask(SIG_BLOCK, &signals, nullptr) != 0) {

            throw std::runtime_error("Install signal handler failed");
        }

        Pistache::Address addr(Pistache::Ipv4::any(), Pistache::Port(this->port));

        // Configure endpoint
        auto opts = Pistache::Http::Endpoint::options()
                .threads(threadCount)
                .backlog(4096)
                .maxRequestSize(Const::DefaultMaxResponseSize)
                .flags(Pistache::Tcp::Options::ReuseAddr);

        Pistache::Http::Endpoint httpEndpoint(addr);
        httpEndpoint.init(opts);

        // Configure and start endpoint
        httpEndpoint.setHandler(getHandler());
        httpEndpoint.serveThreaded();

        // Wait for a signal
        logger->info("Awaiting signal");
        int signal = 0;
        int status = sigwait(&signals, &signal);
        if (status == 0) {
            logger->info("Received signal: {}", signal);
        } else {
            logger->info("Sigwait return value: {}", signal);
        }

        httpEndpoint.shutdown();
    }

    std::shared_ptr<Pistache::Http::Handler> RFITEndpoint::getHandler() {
        return Pistache::Http::make_handler<RFIT_NS::endpoint::RFITEndpointHandler>();
    }
}