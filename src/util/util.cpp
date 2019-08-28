#include "util/util.h"
#include <sys/time.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sstream>
#include <vector>
#include <cstring>

using namespace std;

namespace util {

    std::string GetStrFromUInt32(uint32_t value) 
    {
        char buf[100] = {0};
        int val_len = 0;

        val_len = snprintf(buf, sizeof(buf) - 1, "%u", value);

        return std::string(buf, val_len);
    }

    std::string GetStrFromInt32(int value) 
    {
        char buf[100] = {0};
        int val_len = 0;

        val_len = snprintf(buf, sizeof(buf) - 1, "%d", value);

        return std::string(buf, val_len);
    }

    std::string GetStrFromInt64(int64_t value) 
    {
        char buf[100] = {0};
        int val_len = 0;

        val_len = snprintf(buf, sizeof(buf) - 1, "%ld", value);

        return std::string(buf, val_len);
    }

    int64_t GetInt64FromString(std::string str) 
    {
        std::stringstream ss;
        ss << str;
        int64_t ret = 0;
        ss >> ret;

        return ret;
    }

    int64_t GetNowTimeByMs()
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
            
        return ((int64_t)(tv.tv_sec) * 1000 + (int64_t)(tv.tv_usec)/1000); 
    }

    int GetIpByDomain(const std::string& strDomain, std::string& strIp)
    {
        char *ptr,**pptr;    
        struct hostent *hptr;    
        char str[32];    
        ptr = (char *)strDomain.data();       /* �1�7�1�7�1�7�1�7gethostbyname()�1�7�1�7�1�7�1�7�1�7�0�5�1�7�1�7�1�7�1�7�1�7�1�7�1�7�1�7hptr�1�7�1�7 */    

        std::vector<std::string> vecIp;    
        int iRet = 0;    
        if ((hptr = gethostbyname(ptr)) == NULL)    
        {        
            //CLIB_LOG_ERROR(g_stSysLog, "[E]uin[%u] gethostbyname error for host:%s", uiUin, ptr);        
            return -1; /* �1�7�1�7�1�7�1�7�1�7�1�7�1�7gethostbyname�1�7�1�7�1�7�1�7�1�7�1�7�1�7�1�7�1�7�1�7�1�71 */    
        }    

        /* �1�7�1�7�1�7�1�7�1�7�1�7�1�7�0�1�Z�1�7�1�7�1�7�1�7�1�7�1�7�1�7 */    
        switch(hptr->h_addrtype)    {        
            case AF_INET:        
            case AF_INET6:            
                pptr=hptr->h_addr_list;            

                /* �1�7�1�7�1�7�0�8�0�7�0�1�1�7�1�7�1�7�1�7�1�7�1�7�ք1�7�0�7�1�7�1�7�1�7�1�7�1�7�1�7�1�7�1�7�1�7�1�7�1�7�1�7�ք1�7�1�7�1�7�1�7�1�7inet_ntop()�1�7�1�7�1�7�1�7 */            
                for(;*pptr!=NULL;pptr++) {                
                    //CLIB_LOG_DEBUG(g_stSysLog,                    
                    //  "[DEBUG]address:%s\n",                    
                    //  inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str)));                
                    vecIp.push_back(std::string(inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str))));            
                }            
                break;        

                default:            
                    //CLIB_LOG_ERROR(g_stSysLog,                "[E]uin[%u] unknown address type", uiUin);            
                    iRet = -1;            
                    break;    

        }    

        size_t size = vecIp.size(); 
        size_t index = 0;    
        
        if (size > 0) {        
            index = time(NULL)%size;        
            strIp = vecIp[index];
        }    
        else    
        {               
            iRet = -1;    
        }    

        return iRet;
    }


    //UrlEncode -- unconditionally encode "ch" into "%XX" where XX are two hex digits
    static char HexDigit(unsigned value)
    {
        switch (value)
        {
            case 0:
                return '0';
            case 1:
                return '1';
            case 2:
                return '2';
            case 3:
                return '3';
            case 4:
                return '4';
            case 5:
                return '5';
            case 6:
                return '6';
            case 7:
                return '7';
            case 8:
                return '8';
            case 9:
                return '9';
            case 0xA:
                return 'a';
            case 0xB:
                return 'b';
            case 0xC:
                return 'c';
            case 0xD:
                return 'd';
            case 0xE:
                return 'e';
            case 0xF:
                return 'f';
            default:
                return ' '; // Keep the compiler happy!
        }
    }


    //UrlEncode & UrlDecode
    string UrlEncode(const char* pSrcUrl, int iLen)
    {
        char ch;
        int srcpos=0, dstpos=0;
        static char sEncodeBuf[1024];
        for (srcpos = 0; srcpos<iLen; ++srcpos)
        {
            //assume ASCII or ANSI:
            ch = pSrcUrl[srcpos];
            if(' '!=ch && isprint(ch))
                sEncodeBuf[dstpos++] = ch;
            else
            {
                sEncodeBuf[dstpos++] = '%';
                sEncodeBuf[dstpos++] = HexDigit(static_cast<unsigned char>(ch) >> 4u);
                sEncodeBuf[dstpos++] = HexDigit(static_cast<unsigned char>(ch) & 0xFu);
            }
        }
        sEncodeBuf[dstpos] = '\0';

        return sEncodeBuf;
    }


    string UrlDecode(const char* pEncodedUrl, int iLen)
    {
        #define BUFFER_SIZE (50*1024)
        char p[BUFFER_SIZE] = {0};
        int iMaxLen = sizeof(p) > iLen ? iLen : sizeof(p);
        strncpy(p, pEncodedUrl, iMaxLen);
        int i=0, j=0;
        char c1, c2;
        while ( *(p+i) )
        {
            if ( *(p+i) == '+' )
            {
                *(p+j)=' ';
                i++;
            }

            else if (*(p+i)=='%' && *(p+i+1) && *(p+i+2) && isxdigit((int)*(p+i+1)) && isxdigit((int)*(p+i+2)) )
            {
                c1 = toupper( *(p+i+1) );
                c2 = toupper( *(p+i+2) );
                c1 = (c1>='A') ? (c1-'A'+10):(c1-'0');
                c2 = (c2>='A') ? (c2-'A'+10):(c2-'0');
                *(p+j)=(c1<<4)+c2;
                i+=3;
            } else {
                *(p+j)=*(p+i);
                i++;
            }
            j++;
        }   // end of while
        p[j]=0x0;        
        p[BUFFER_SIZE-1] = 0;
        
        return string(p);
    }

    int UrlDecode(const char* pIn, int iInLen, char* pOut, int& iOutLen)
    {
        const char* p = pIn;
        int i=0, j=0;
        char c1, c2;

        if (iInLen>iOutLen || NULL==pOut) {
            return -1;
        }

        while ( *(p+i) )
        {
            if ( *(p+i) == '+' )
            {
                *(pOut+j)=' ';
                i++;
            }
            else if (*(p+i)=='%' && *(p+i+1) && *(p+i+2) && isxdigit((int)*(p+i+1)) && isxdigit((int)*(p+i+2)) )
            {
                c1 = toupper( *(p+i+1) );
                c2 = toupper( *(p+i+2) );
                c1 = (c1>='A') ? (c1-'A'+10):(c1-'0');
                c2 = (c2>='A') ? (c2-'A'+10):(c2-'0');
                *(pOut+j)=(c1<<4)+c2;
                i+=3;
            } else {
                *(pOut+j)=*(p+i);
                i++;
            }
            j++;
        }   // end of while
        pOut[j]=0x0;
        iOutLen = j;

        return 0;
    }

    //�1�7�1�7�1�7�1�72016-05-19 14:00:00�1�7�1�7�0�4�1�7�1�7�1�7�1�7�0�2�1�7�1�7�1�7�1�7�1�7�1�7
    std::string GetNowTimeFormatString()
    {
        time_t timep;
        time(&timep);

        return GetStrFromUnixTime(timep);
    }

    std::string GetStrFromUnixTime(time_t unix_time)
    {
        struct tm *p = NULL;
        p = localtime(&unix_time);

        char buf[100];
        memset(buf, 0, sizeof(buf));
        uint32_t mon = 1 + p->tm_mon;
        string str_mon;
        if (mon < 10) {
            str_mon = "0" + GetStrFromUInt32(mon);
        } else {
            str_mon = GetStrFromUInt32(mon);
        }

        uint32_t day = p->tm_mday;
        string str_day;
        if (day < 10) {
            str_day = "0" + GetStrFromUInt32(day);
        } else {
            str_day = GetStrFromUInt32(day);
        }

        uint32_t hour = p->tm_hour;
        string str_hour;
        if (hour < 10) {
            str_hour = "0" + GetStrFromUInt32(hour);
        } else {
            str_hour = GetStrFromUInt32(hour);
        }

        uint32_t min = p->tm_min;
        string str_min;
        if (min < 10) {
            str_min = "0" + GetStrFromUInt32(min);
        } else {
            str_min = GetStrFromUInt32(min);
        }

        uint32_t sec = p->tm_sec;
        string str_sec;
        if (sec < 10) {
            str_sec = "0" + GetStrFromUInt32(sec);
        } else {
            str_sec = GetStrFromUInt32(sec);
        }
        
        snprintf(buf, sizeof(buf), "%d-%s-%s %s:%s:%s", (1900 + p->tm_year), str_mon.c_str(), str_day.c_str(), str_hour.c_str(), str_min.c_str(), str_sec.c_str());

        return string(buf, strlen(buf));
    }

    //19870912_22
    std::string GetStrDBFromUnixTime(time_t unix_time)
    {
        struct tm *p = NULL;
        p = localtime(&unix_time);

        char buf[100];
        memset(buf, 0, sizeof(buf));
        uint32_t mon = 1 + p->tm_mon;
        string str_mon;
        if (mon < 10) {
            str_mon = "0" + GetStrFromUInt32(mon);
        } else {
            str_mon = GetStrFromUInt32(mon);
        }

        uint32_t day = p->tm_mday;
        string str_day;
        if (day < 10) {
            str_day = "0" + GetStrFromUInt32(day);
        } else {
            str_day = GetStrFromUInt32(day);
        }

        uint32_t hour = p->tm_hour;
        string str_hour;
        if (hour < 10) {
            str_hour = "0" + GetStrFromUInt32(hour);
        } else {
            str_hour = GetStrFromUInt32(hour);
        }

        uint32_t min = p->tm_min;
        string str_min;
        if (min < 10) {
            str_min = "0" + GetStrFromUInt32(min);
        } else {
            str_min = GetStrFromUInt32(min);
        }

        uint32_t sec = p->tm_sec;
        string str_sec;
        if (sec < 10) {
            str_sec = "0" + GetStrFromUInt32(sec);
        } else {
            str_sec = GetStrFromUInt32(sec);
        }
        
        snprintf(buf, sizeof(buf), "%d%s%s_%s", (1900 + p->tm_year), str_mon.c_str(), str_day.c_str(), str_hour.c_str());

        return string(buf, strlen(buf));
    }

    unsigned int JsHash(const char *str)
    {
        uint32_t hash=1315423911;                                                                        
        while(*str){                                                                                     
            hash ^= ((hash << 5) + (*str++) + (hash >> 2));                                              
        }                                                                                                
        return (hash & 0x7FFFFFFF);   
    }

    int Split(std::string splitstr, std::string keystr, std::vector<std::string> &group)
    {
        group.clear();

        while(splitstr != ""){
            std::size_t pos = splitstr.find(keystr.c_str());
            std::string partstr = "";
            if (pos == -1) {
                partstr = splitstr;
                splitstr = "";
            }
            else if(pos == splitstr.length() - 1){
                partstr = splitstr.substr(0, pos);
                splitstr = "";
            }
            else{
                partstr = splitstr.substr(0, pos);
                splitstr = splitstr.substr(pos + keystr.length());
            }

            group.push_back(partstr);
        }
        
        return 0;
    }

    int ParseUrlPara(std::string url_para, std::map<std::string, std::string> &map_para)
    {
        map_para.clear();
        std::vector<std::string> vec_group;

        Split( url_para, "&", vec_group);

        for(std::size_t i = 0; i < vec_group.size(); i++) 
        {
            std::vector<std::string> vec_item;
            Split(vec_group[i], "=", vec_item);
            if (vec_item.size() == 2) {
                string key = UrlDecode(vec_item[0]);
                string value = UrlDecode(vec_item[1]);
                map_para[key] = value;
            } else {
                return -1;
            }
        }

        return 0;
    }

    //2016-03-06 12:22:11
    //�1�7�1�7�1�7�1�7�0�5�1�7�1�7�1�7�0�6�1�7�1�7�1�7�1�7�1�7�0�6�1�7�0�6�1�7�0�2�1�7�1�7�0�5�1�7�1�7�1�7�1�7�1�7�1�7�1�7�0�4�1�7�1�7�1�7�1�7�1�7�1�7�0�8�1�7�1�7�1�7�1�7�1�7-1
    time_t StrToTime(const std::string& str_time) 
    {
        struct tm tm_temp = {};
        int i_ret = sscanf(str_time.c_str(),  "%4d-%2d-%2d %2d:%2d:%2d",
            &tm_temp.tm_year,
            &tm_temp.tm_mon,
            &tm_temp.tm_mday,
            &tm_temp.tm_hour,
            &tm_temp.tm_min,
            &tm_temp.tm_sec
            );

        if (i_ret < 0 ) {
            return -1;
        }
        
        tm_temp.tm_year -= 1900;
        tm_temp.tm_mon -= 1;

        return mktime(&tm_temp);
    }

    int GetHttpReqLen(const char *pPkg,  int  iBytesRecved)
    {
        unsigned int contentlen = 0;
        char        *pLine = (char*)pPkg;
        char        *pLineEnd = strstr(pLine, "\r\n");        //skip first line

        if (NULL == pLineEnd) //no first line, err
        {
            if(iBytesRecved > 1024) //protect
            {
                //Attr_API(common::EMONITOR_TOTAL_ERROR_PARAM_INVALID, 1);
                return -1;
            }

            return 0;
        }

        if (pLineEnd > ((char *)pPkg + iBytesRecved - 1))
        {
            return 0;
        }

        pLineEnd += 1; //now first line end point to '\n'

        char *pTmpLineEnd = NULL; //use for switch

        pLine = pLineEnd + 1; //switch to next line

        bool bHeadEnd = false;
        int n = 0;

        while (true)
        {
            pLineEnd = strchr(pLine, '\n'); //find line end
            if (NULL == pLineEnd)
            {
                return 0;
            }
            else if (pLineEnd > (char *)pPkg + iBytesRecved)
            {
                return 0;
            }

            //check if line starts with "\r\n", if yes, it means head ends ("\r\n\r\n")
            //another "\r\n" can be found on prev line end;
            if (pLine[0] == '\r' && pLine[1] == '\n')
            {
                //head end
                bHeadEnd = true;
                break;
            }

            //msy be "Content-length:"  try
            if (pLine[0] == 'C' || pLine[0] == 'c')
            {
                if ( pLineEnd[-1]=='\r')
                    pTmpLineEnd = pLineEnd-1;
                else
                    pTmpLineEnd = pLineEnd;

                n = pTmpLineEnd - pLine;

                char * pTmpLine = pLine;

                //use [n> 15 && pTmpLine[14] == ':'] to make sure less strncasecmp
                if(n> 15 && pTmpLine[14] == ':' &&0 == strncasecmp(pTmpLine, "Content-length:", 15))
                {
                    pTmpLine += 15;

                    while(*pTmpLine==' '||*pTmpLine=='\t'||*pTmpLine=='\r') pTmpLine++;

                    char tmp = *pTmpLineEnd;
                    *pTmpLineEnd = '\0'; //prepare for atoll
                    contentlen   = atoll(pTmpLine);
                    *pTmpLineEnd = tmp; //roll back
                }
            }

            pLine = pLineEnd + 1; //switch to next line
        }

        if (contentlen > (1*1024*1024))
        {
            return -1;
        }

        if(!bHeadEnd) //still not recv line "\r\n"
        {
            if(iBytesRecved > 1024) //protect
            {
                return -1;
            }

            return 0;
        }

        int HeadDataSize = pLine - (char *)pPkg + 2;

        //need check head size?

        int iPkgLen = HeadDataSize + contentlen;
        if(iBytesRecved >= iPkgLen)
        {
            return iPkgLen;
        }
        else //still not recv all content
        {
            return 0;
        }
    }
    
    //2016-08-29 14:00:00
    bool CheckDbFormatTime(const std::string& str_db_time)
    {
        if (str_db_time.size() != 19) {
            return false;
        }

        if (str_db_time.substr(0,2) != "20") {
            return false;
        }

        if (str_db_time.at(2) > '9' || str_db_time.at(2) < '0') {
            return false;
        }

        if (str_db_time.at(3) > '9' || str_db_time.at(3) < '0') {
            return false;
        }

        if (str_db_time.at(4) != '-') {
            return false;
        }

        if (str_db_time.at(5) > '1' || str_db_time.at(5) < '0') {
            return false;
        }

        
        if (str_db_time.at(6) > '9' || str_db_time.at(6) < '0') {
            return false;
        }

        if (str_db_time.at(7) != '-') {
            return false;
        }

        if (str_db_time.at(8) > '3' || str_db_time.at(8) < '0') {
            return false;
        }

        
        if (str_db_time.at(9) > '9' || str_db_time.at(9) < '0') {
            return false;
        }

        if (str_db_time.at(10) != ' ') {
            return false;
        }

        if (str_db_time.at(11) > '2' || str_db_time.at(11) < '0') {
            return false;
        }

        
        if (str_db_time.at(12) > '9' || str_db_time.at(12) < '0') {
            return false;
        }

        if (str_db_time.at(13) != ':') {
            return false;
        }

        if (str_db_time.at(14) > '5' || str_db_time.at(14) < '0') {
            return false;
        }

        
        if (str_db_time.at(15) > '9' || str_db_time.at(15) < '0') {
            return false;
        }

        if (str_db_time.at(16) != ':') {
            return false;
        }

        if (str_db_time.at(17) > '5' || str_db_time.at(17) < '0') {
            return false;
        }

        
        if (str_db_time.at(18) > '9' || str_db_time.at(18) < '0') {
            return false;
        }

        return true;
    }

    std::string StrToUpper(const string& str)
    {
        std::string str_ret;
        str_ret.resize(str.size());
        std::transform(str.begin(),str.end(), str_ret.begin(), ::toupper);

        return str_ret;
    }

	std::string StrToLower(const string& str)
    {
        std::string str_ret;
        str_ret.resize(str.size());
        std::transform(str.begin(),str.end(), str_ret.begin(), ::tolower);

        return str_ret;
    }
}

