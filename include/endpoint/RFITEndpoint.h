//
// Created by kingdo on 2021/7/7.
//

#ifndef RFIT_RFIT_ENDPOINT_H
#define RFIT_RFIT_ENDPOINT_H

#include <pistache/endpoint.h>
#include "RFIT/RFIT.h"
#include "utils/config.h"

namespace RFIT_NS::endpoint {
    class RFITEndpoint : public Pistache::Http::Endpoint {
    public:
        explicit RFITEndpoint() = default;

        void start() const;

        static std::shared_ptr<Pistache::Http::Handler> getHandler();

    private:
        int port = systemConfig.listenPort;
        int threadCount = systemConfig.reactorThreadCount;
    };
}

#endif //RFIT_RFIT_ENDPOINT_H
