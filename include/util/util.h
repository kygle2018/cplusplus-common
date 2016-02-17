#ifndef __UTIL_UTIL_H__
#define __UTIL_UTIL_H__
#include <stdio.h>
#include <string>

namespace util {

	extern std::string GetStrFromUInt32(uint32_t value);

	extern std::string GetStrFromInt64(int64_t value);

    extern int64_t GetInt64FromString(std::string str);

    extern int64_t GetNowTimeByMs();

	extern int GetIpByDomain(const std::string& strDomain, std::string& strIp);

	extern std::string UrlEncode(const char* pSrcUrl, int iLen);

	extern std::string UrlDecode(const char* pEncodedUrl, int iLen);

	extern int UrlDecode(const char* pIn, int iInLen, char* pOut, int& iOutLen);

	inline std::string UrlEncode(const std::string &str) { return UrlEncode(str.data(), (int)str.size()); }

	inline std::string UrlDecode(const std::string &str) { return UrlDecode(str.data(), (int)str.size()); }
};
#endif

