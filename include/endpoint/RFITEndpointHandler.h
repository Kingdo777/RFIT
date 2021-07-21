//
// Created by kingdo on 2021/7/7.
//

#ifndef RFIT_RFIT_ENDPOINT_HANDLER_H
#define RFIT_RFIT_ENDPOINT_HANDLER_H

#include <pistache/http.h>
#include <utils/environment.h>
#include "RFIT/RFIT.h"
#include "proto/rfit.pb.h"
#include "utils/rstring.h"

using namespace Pistache;
namespace RFIT_NS::endpoint {
    class RFITEndpointHandler : public Pistache::Http::Handler {
    HTTP_PROTOTYPE(RFITEndpointHandler)

    public:
        explicit RFITEndpointHandler()
                : rfit(RFIT_GLOBAL),
                  config(systemConfig) {};

        void onRequest(const Pistache::Http::Request &request,
                       Pistache::Http::ResponseWriter response) override;

        void onTimeout(const Pistache::Http::Request &request,
                       Pistache::Http::ResponseWriter response) override;


    private:

        RFIT_NS::RFIT &rfit;

        SystemConfig &config;

        int requestToMsg(const Pistache::Http::Request &request, FunctionRegisterMsg &msg) const;

        static int requestToMsg(const Http::Request &request, Message &msg) ;

    };

}
#endif //RFIT_RFIT_ENDPOINT_HANDLER_H
