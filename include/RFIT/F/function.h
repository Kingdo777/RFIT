//
// Created by kingdo on 10/17/20.
//

#ifndef TPFAAS_FUNCTION_H
#define TPFAAS_FUNCTION_H

#include <string>
#include "proto/rfit.pb.h"
#include "RFIT/R/resource.h"

namespace RFIT_NS {

    class F {
    public:

        F() = delete;

        explicit F(R &r, void (*addr)(Message *), uint concurrency = 1) :
                entry_addr(addr),
                r(r),
                concurrency(concurrency) {};

    private:

        string func_name;

        void (*entry_addr)(Message *);

        R &r;

        uint concurrency = 1;

    };
}
#endif //TPFAAS_FUNCTION_H
