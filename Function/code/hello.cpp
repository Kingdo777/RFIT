//
// Created by kingdo on 2021/7/9.
//
#include <unistd.h>
#include "proto/rfit.pb.h"
#include "ping.h"

void hello_mainEntry(RFIT_NS::Message &m) {
    PING(m)
    usleep(100 * 1000);
    m.set_outputdata("Hello World");
}