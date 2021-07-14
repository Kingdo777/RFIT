//
// Created by kingdo on 2021/7/9.
//
#include "proto/rfit.pb.h"

void hello_main(RFIT_NS::Message &m) {
    m.set_outputdata("Hello World\n");
}