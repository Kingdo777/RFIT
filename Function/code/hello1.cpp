//
// Created by kingdo on 2021/7/9.
//
#include "proto/rfit.pb.h"
#include "ping.h"

void hello1_mainEntry(RFIT_NS::Message &m) {
    PING(m)
    m.set_outputdata("Hello World\n");
}