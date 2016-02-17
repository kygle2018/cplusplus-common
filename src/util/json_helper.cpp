#include "util/json_helper.h"
#include "util/log.h"
//#include <string>

namespace util {
    std::string JsonEncode(const Json::Value& value)
    {
        Json::FastWriter writer;
        return writer.write(value);
    }

    bool JsonDecode(const std::string& str_data, Json::Value& js_value) 
    {
        Json::Reader reader;
        return reader.parse(str_data,  js_value,  false);
    }
};

