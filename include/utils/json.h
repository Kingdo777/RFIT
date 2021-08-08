#pragma once

#include <proto/rfit.pb.h>
#include <utils/timing.h>
#include <utils/exception.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

using namespace rapidjson;

namespace RFIT_NS::utils {

    class JsonFieldNotFound : public RFIT_NS::utils::RFITException {
    public:
        explicit JsonFieldNotFound(const std::string &message)
                : RFIT_NS::utils::RFITException(message) {}
    };

    std::string messageToJson(const FunctionRegisterMsg &msg);

    std::string messageToJson(const Message &msg);


    bool getBoolFromJson(Document &doc, const std::string &key, bool dflt);

    std::vector<uint32_t> getUintArrayFromJson(Document &doc,
                                               const std::string &key);

    int getIntFromJson(Document &doc, const std::string &key, int dflt);

    int64_t getInt64FromJson(Document &doc, const std::string &key, int dflt);

    std::string getStringFromJson(Document &doc,
                                  const std::string &key,
                                  const std::string &dflt);

    std::string jsonTest();
}
