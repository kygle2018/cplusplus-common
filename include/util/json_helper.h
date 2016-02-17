#ifndef __COMMON_JSON_HELPER_H__
#define __COMMON_JSON_HELPER_H__
#include "json/json.h"
#include <string>

namespace util {
std::string JsonEncode(const Json::Value& value);
bool JsonDecode(const std::string& str_data, Json::Value& js_value);
};
#endif 
