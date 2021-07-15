//
// Created by kingdo on 10/17/20.
//

#ifndef TPFAAS_FUNCTION_H
#define TPFAAS_FUNCTION_H

#include <string>
#include <utility>
#include "proto/rfit.pb.h"
#include "utils/dl.h"
#include "RFIT/R/resource.h"

namespace RFIT_NS {

    typedef void (*FuncType)(Message &);

    class F {
    public:

        F() = delete;

        explicit F(const string &funcName_,
                   shared_ptr<R> r_,
                   utils::dlResult dr_, uint
                   concurrency = 1) :
                funcName(funcName_),
                dr(dr_),
                r(std::move(r_)),
                concurrency(concurrency) {};

    private:

        const string &funcName;

        utils::dlResult dr;

        shared_ptr<R> r;

        uint concurrency = 1;

    };
}
#endif //TPFAAS_FUNCTION_H
