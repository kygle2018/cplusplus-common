#include "util/util.h"
#include <sys/time.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sstream>
#include <vector>

using namespace std;

namespace util {

	std::string GetStrFromUInt32(uint32_t value) 
	{
    	char buf[100] = {0};
    	int val_len = 0;

    	val_len = snprintf(buf, sizeof(buf) - 1, "%u", value);

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
		ptr = (char *)strDomain.data();       /* 调用gethostbyname()。调用结果都存在hptr中 */    

		std::vector<std::string> vecIp;    
		int iRet = 0;    
		if ((hptr = gethostbyname(ptr)) == NULL)    
		{        
			//CLIB_LOG_ERROR(g_stSysLog, "[E]uin[%u] gethostbyname error for host:%s", uiUin, ptr);        
			return -1; /* 如果调用gethostbyname发生错误，返回1 */    
		}    

		/* 将主机的规范名打出来 */    
		switch(hptr->h_addrtype)    {        
			case AF_INET:        
			case AF_INET6:            
				pptr=hptr->h_addr_list;            

				/* 将刚才得到的所有地址都打出来。其中调用了inet_ntop()函数 */            
				for(;*pptr!=NULL;pptr++) {                
					//CLIB_LOG_DEBUG(g_stSysLog,                    
					//	"[DEBUG]address:%s\n",                    
					//	inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str)));                
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
			//CLIB_LOG_ERROR(g_stSysLog,            "[E]uin[%u] Not Find Any Ip address!", uiUin);        
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
            /*
            else if ( *(p+i) != '%' )
            {
                *(p+j)=*(p+i);
                i++;
            }*/
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
}

