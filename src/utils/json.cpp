#include <utils/json.h>

namespace RFIT_NS::utils {
    std::string messageToJson(const FunctionRegisterResponseMsg &msg) {
        Document d;
        d.SetObject();
        Document::AllocatorType &a = d.GetAllocator();
        // Need to be explicit with strings here to make a copy _and_ make sure we
        // specify the length to include any null-terminators from bytes
        d.AddMember("status", msg.status(), a);
        d.AddMember("funcName", Value(msg.funcname().c_str(), msg.funcname().size(), a).Move(), a);
        d.AddMember("memSize", msg.memsize(), a);
        d.AddMember("coreRation", msg.coreration(), a);
        d.AddMember("concurrency", msg.concurrency(), a);
        if (!msg.message().empty()) {
            d.AddMember("message", Value(msg.message().c_str(), msg.message().size(), a).Move(), a);
        }
        StringBuffer sb;
        Writer<StringBuffer> writer(sb);
        d.Accept(writer);
        return sb.GetString();
    }

//
//std::string getJsonOutput(const faabric::Message& msg)
//{
//    Document d;
//    d.SetObject();
//    Document::AllocatorType& a = d.GetAllocator();
//    d.AddMember(
//      "output_data",
//      Value(msg.outputdata().c_str(), msg.outputdata().size(), a).Move(),
//      a);
//    StringBuffer sb;
//    Writer<StringBuffer> writer(sb);
//    d.Accept(writer);
//    return sb.GetString();
//}

    std::string getStringFromJson(Document &doc, const std::string &key, const std::string &dflt) {
        Value::MemberIterator it = doc.FindMember(key.c_str());
        if (it == doc.MemberEnd()) {
            return dflt;
        }
        const char *valuePtr = it->value.GetString();
        return std::string(valuePtr, valuePtr + it->value.GetStringLength());
    }

    int64_t getInt64FromJson(Document &doc, const std::string &key, int dflt) {
        Value::MemberIterator it = doc.FindMember(key.c_str());
        if (it == doc.MemberEnd()) {
            return dflt;
        }

        return it->value.GetInt64();
    }

    std::vector<uint32_t> getUintArrayFromJson(Document &doc, const std::string &key) {
        Value::MemberIterator it = doc.FindMember(key.c_str());
        std::vector<uint32_t> result;
        if (it == doc.MemberEnd()) {
            return result;
        }

        for (const auto &i : it->value.GetArray()) {
            result.emplace_back(i.GetUint());
        }

        return result;
    }

    bool getBoolFromJson(Document &doc, const std::string &key, bool dflt) {
        Value::MemberIterator it = doc.FindMember(key.c_str());
        if (it == doc.MemberEnd()) {
            return dflt;
        }

        return it->value.GetBool();
    }

    int getIntFromJson(Document &doc, const std::string &key, int dflt) {
        Value::MemberIterator it = doc.FindMember(key.c_str());
        if (it == doc.MemberEnd()) {
            return dflt;
        }

        return it->value.GetInt();
    }

    std::string getValueFromJsonString(const std::string &key,
                                       const std::string &jsonIn) {
        MemoryStream ms(jsonIn.c_str(), jsonIn.size());
        Document d;
        d.ParseStream(ms);

        std::string result = getStringFromJson(d, key, "");
        return result;
    }

//faabric::Message jsonToMessage(const std::string& jsonIn)
//{
//    PROF_START(jsonDecode)
//    auto logger = faabric::util::getLogger();
//
//    MemoryStream ms(jsonIn.c_str(), jsonIn.size());
//    Document d;
//    d.ParseStream(ms);
//
//    faabric::Message msg;
//
//    // Set the message type
//    int msgType = getIntFromJson(d, "type", 0);
//    if (!faabric::Message::MessageType_IsValid(msgType)) {
//        logger->error("Bad message type: {}", msgType);
//        throw std::runtime_error("Invalid message type");
//    }
//    msg.set_type(static_cast<faabric::Message::MessageType>(msgType));
//
//    msg.set_timestamp(getInt64FromJson(d, "timestamp", 0));
//    msg.set_id(getIntFromJson(d, "id", 0));
//    msg.set_user(getStringFromJson(d, "user", ""));
//    msg.set_function(getStringFromJson(d, "function", ""));
//    msg.set_executedhost(getStringFromJson(d, "exec_host", ""));
//    msg.set_masterhost(getStringFromJson(d, "master_host", ""));
//    msg.set_finishtimestamp(getInt64FromJson(d, "finished", 0));
//
//    msg.set_snapshotkey(getStringFromJson(d, "snapshot_key", ""));
//    msg.set_snapshotsize(getIntFromJson(d, "snapshot_size", 0));
//    msg.set_funcptr(getIntFromJson(d, "func_ptr", 0));
//
//    msg.set_pythonuser(getStringFromJson(d, "py_user", ""));
//    msg.set_pythonfunction(getStringFromJson(d, "py_func", ""));
//    msg.set_pythonentry(getStringFromJson(d, "py_entry", ""));
//
//    msg.set_inputdata(getStringFromJson(d, "input_data", ""));
//    msg.set_outputdata(getStringFromJson(d, "output_data", ""));
//
//    msg.set_isasync(getBoolFromJson(d, "async", false));
//    msg.set_ispython(getBoolFromJson(d, "python", false));
//    msg.set_istypescript(getBoolFromJson(d, "typescript", false));
//    msg.set_isstatusrequest(getBoolFromJson(d, "status", false));
//    msg.set_isexecgraphrequest(getBoolFromJson(d, "exec_graph", false));
//
//    msg.set_resultkey(getStringFromJson(d, "result_key", ""));
//    msg.set_statuskey(getStringFromJson(d, "status_key", ""));
//
//    msg.set_ismpi(getBoolFromJson(d, "mpi", false));
//    msg.set_mpiworldid(getIntFromJson(d, "mpi_world_id", 0));
//    msg.set_mpirank(getIntFromJson(d, "mpi_rank", 0));
//    msg.set_mpiworldsize(getIntFromJson(d, "mpi_world_size", 0));
//
//    msg.set_cmdline(getStringFromJson(d, "cmdline", ""));
//
//    PROF_END(jsonDecode)
//
//    return msg;
//}
}
