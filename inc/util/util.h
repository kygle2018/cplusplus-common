#ifndef __UTIL_UTIL_H__
#define __UTIL_UTIL_H__
#include <stdio.h>
#include <string>
#include <stdint.h>
#include <time.h>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>


namespace util {
    const int kMaxLogLength = 4 * 1024 * 1024;
    const int kMaxPrintLog = 8 * 1000;

	extern std::string GetStrFromUInt32(uint32_t value);

	extern std::string GetStrFromInt32(int value);

	extern std::string GetStrFromInt64(int64_t value);

	extern int64_t GetInt64FromString(std::string str);

	extern int64_t GetNowTimeByMs();

	extern int GetIpByDomain(const std::string& strDomain, std::string& strIp);

	extern std::string UrlEncode(const char* pSrcUrl, int iLen);

	extern std::string UrlDecode(const char* pEncodedUrl, int iLen);

	extern int UrlDecode(const char* pIn, int iInLen, char* pOut, int& iOutLen);

	inline std::string UrlEncode(const std::string &str) { return UrlEncode(str.data(), (int)str.size()); }

	inline std::string UrlDecode(const std::string &str) { return UrlDecode(str.data(), (int)str.size()); }

	//°′??2016-05-19 14:00:00??ê?·μ??ê±??êy?Y
	extern std::string GetNowTimeFormatString();
	extern std::string GetStrFromUnixTime(time_t unix_time);
	extern std::string GetStrDBFromUnixTime(time_t unix_time);
	
	extern unsigned int JsHash(const char *str);

	extern int Split(std::string splitstr, std::string keystr, std::vector<std::string> &group);

	extern int ParseUrlPara(std::string url_para, std::map<std::string, std::string> &map_para);

	/*******************************************
	去掉开头和结尾的字符
	*******************************************/
	inline std::string& Trim(std::string& s, char o=' ')
	{
	   if (s.empty())
	   {
	     return s;
	   }

	   std::string::iterator c;
	   // Erase whitespace before the string
	   for (c = s.begin(); c != s.end() && (*c++) == o;);
	     s.erase(s.begin(), --c);

	   // Erase whitespace after the string
	   for (c = s.end(); c != s.begin() && (*--c) ==o;);
	     s.erase(++c, s.end());

	   return s;
	}

	template<typename T> std::string to_str(const T& t)
	 {
	     std::ostringstream s;
	     s << t;
	     return s.str();
	 }
	 
	 template<typename T> T from_str(const std::string& s)
	 {
	     std::istringstream is(s);
	     T t;
	     is >> t;
	     return t;
	 }
 
	inline bool IsNum(std::string & input)
	{
	    if (input.empty())
	    {
	        return false;
	    }
	    std::string::iterator iter;
	    for (iter = input.begin(); iter != input.end(); iter++)
	    {

	        if(*iter > '9' || *iter < '0')
	        {
	            return false;
	        }
	    }

	    return true;
	}
    
	extern time_t StrToTime(const std::string& str_time);
	extern bool CheckDbFormatTime(const std::string& str_db_time);

	extern std::string StrToUpper(const std::string& str);
	extern std::string StrToLower(const std::string& str);
};
#endif

