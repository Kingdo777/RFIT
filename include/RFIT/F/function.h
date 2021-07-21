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
                   utils::dlResult dr_,
                   boost::filesystem::path p,
                   uint32_t concurrency = 1);;

        [[nodiscard]] const string &getFuncName() const {
            return funcName;
        }

        [[nodiscard]] const dlResult &getDr() const {
            return dr;
        }

        [[nodiscard]] const shared_ptr<R> &getR() const {
            return r;
        }

        [[nodiscard]] uint32_t getConcurrency() const {
            return concurrency;
        }

        [[nodiscard]] bool isConcurrency() const {
            return concurrency > 1;
        }

        void invoke(Message &msg);

    private:

        const string &funcName;

        utils::dlResult dr;

        boost::filesystem::path dlPath;

        shared_ptr<R> r;

        uint32_t concurrency = 1;

    };
}
#endif //TPFAAS_FUNCTION_H
