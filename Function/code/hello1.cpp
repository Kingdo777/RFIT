//
// Created by kingdo on 2021/7/9.
//

#include <string>
#include "proto/rfit.pb.h"
#include "utils/json.h"
#include "ping.h"

using namespace std;
using namespace RFIT_NS::utils;

void hello1_mainEntry(RFIT_NS::Message &m) {
    PING(m)
    Document d;
    d.Parse(m.inputdata().c_str());
    string name = "Guest";
    name = getStringFromJson(d, "name", name);
    m.set_outputdata("Hello " + name);
}