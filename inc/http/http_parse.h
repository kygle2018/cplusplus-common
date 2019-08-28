 #ifndef __INC_HTTP_HTTP_H__
 #define __INC_HTTP_HTTP_H__
 #include "util/util.h"
 #include <string>
 #include <map>
 #include <vector>

using namespace std;

namespace http {

    enum HttpStatusCode {
        //请求已经被接受，需要继续处理
        HTTP_STATUS_CODE_CONTINUE = 100,
        HTTP_STATUS_CODE_SWITCHING_PROTOCOLS = 101,
        HTTP_STATUS_CODE_PROCESSING = 102,

        //请求已成功被服务器接收、理解、并接受
        HTTP_STATUS_CODE_SUCCESS = 200,
        HTTP_STATUS_CODE_CREATED = 201,
        HTTP_STATUS_CODE_ACCEPTED = 202,
        HTTP_STATUS_CODE_NON_AUTHORITATIVE_INFORMATION = 203,
        HTTP_STATUS_CODE_NO_CONTENT = 204,
        HTTP_STATUS_CODE_RESET_CONTENT = 205,
        HTTP_STATUS_CODE_PARTIAL_CONTENT = 206,
        HTTP_STATUS_CODE_MULTI_STATUS = 207,

        //需要客户端采取进一步的操作才能完成请求。通常，这些状态码用来重定向，后续的请求地址在本次响应Location域中指明
        HTTP_STATUS_CODE_MULTIPLE_CHOICES = 300,
        HTTP_STATUS_CODE_MOVED_PERMANENTLY = 301,
        HTTP_STATUS_CODE_FOUND = 302,
        HTTP_STATUS_CODE_SEE_OTHER = 303,
        HTTP_STATUS_CODE_NOT_MODIFIED = 304,
        HTTP_STATUS_CODE_USE_PROXY = 305,
        HTTP_STATUS_CODE_SWITCH_PROXY = 306,
        HTTP_STATUS_CODE_TEMPORARY_REDIRECT = 307,

        //客户端错误，客户端开起来发生了错误，妨碍了服务器的处理
        HTTP_STATUS_CODE_BAD_REQUEST = 400,
        HTTP_STATUS_CODE_UNAUTHORIZED = 401,
        HTTP_STATUS_CODE_PAYMENT_REQUIRED = 402,
        HTTP_STATUS_CODE_FORBIDDEN = 403,
        HTTP_STATUS_CODE_NOT_FOUND = 404,
        HTTP_STATUS_CODE_METHOD_NOT_ALLOWED = 405,
        HTTP_STATUS_CODE_NOT_ACCEPTABLE = 406,
        HTTP_STATUS_CODE_PROXY_AUTHENTICATION_REQUIRED = 407,
        HTTP_STATUS_CODE_REQUEST_TIMEOUT = 408,
        HTTP_STATUS_CODE_CONFLICT = 409,
        HTTP_STATUS_CODE_GONE = 410,
        HTTP_STATUS_CODE_LENGTH_REQUIRED = 411,
        HTTP_STATUS_CODE_PRENCONDITION_FAILED = 412,
        HTTP_STATUS_CODE_REQUEST_ENTITY_TOO_LARGE = 413,
        HTTP_STATUS_CODE_REQUEST_URI_TOO_LONG = 414,
        HTTP_STATUS_CODE_UNSUPPORTED_MEDIA_TYPE = 415,
        HTTP_STATUS_CODE_REQUESTED_RANGE_NOT_SATISFIABLE = 416,
        HTTP_STATUS_CODE_EXPCEPTION_FAILED = 417,
        HTTP_STATUS_CODE_IM_A_TEAPOT = 418,
        HTTP_STATUS_CODE_TOO_MANY_CONNECTIONS = 421, //同一个IP连接数限制
        HTTP_STATUS_CODE_UNPROCESSABLE_ENTITY = 422,
        HTTP_STATUS_CODE_LOCKED = 423,
        HTTP_STATUS_CODE_FAILED_DEPENDENCY = 424,
        HTTP_STATUS_CODE_UNORDERED_COLLECTION = 425,
        HTTP_STATUS_CODE_UPGRRADE_REQUIRED = 426,
        HTTP_STATUS_CODE_RETRY_WITH = 449,
        HTTP_STATUS_CODE_UNAVAILABLE_FOR_LEGAL_REASONS = 451,

        //服务器错误，服务器在处理请求的过程中有错误或者异常状态发生，也有可能是服务器意识到以当前的软硬件资源无法完成对请求的处理。
        HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR = 500,
        HTTP_STATUS_CODE_NOT_IMPLEMENTED = 501,
        HTTP_STATUS_CODE_BAD_GATWAY = 502,
        HTTP_STATUS_CODE_SERVICE_UNAVAILABLE = 503, //过载
        HTTP_STATUS_CODE_GATEWAY_TIMEOUT = 504,
        HTTP_STATUS_CODE_HTTP_VERSION_NOT_SUPPORTED = 505,
        HTTP_STATUS_CODE_VARIANT_ALSO_NEGOTIATES = 506,
        HTTP_STATUS_CODE_INSUFFICIENT_STORAGE = 507,
        HTTP_STATUS_CODE_LOOP_DETECTED = 508,
        HTTP_STATUS_CODE_BANDWIDTH_LIMIT_EXCEEDED = 509,
        HTTP_STATUS_CODE_NOT_EXTENDED = 510
    };

    enum HttpErrCode {
        ERR_NO_OK = 0,
        ERR_NO_HAS_PARSED = -100,
        ERR_NO_NOT_PARSED = -101,
        ERR_NO_INVALID_REQUEST_FORMAT = -102,
        ERR_NO_INVALID_REQUEST_METHOD = -103,
        ERR_NO_LACK_REQUIRED_HEADER = -104,
        ERR_NO_INVALID_CONTENT_LEN = -105,
        ERR_NO_INVALID_RESPONSE_FORMAT = -106,
        ERR_NO_INVALID_HTTP_PKG = -107,
    };

    enum HttpPkgType {
        HTTP_PKG_TYPE_REQUEST = 0,
        HTTP_PKG_TYPE_RESPONSE = 1
    };

    enum HttpMethodType {
        HTTP_METHOD_TYPE_GET = 0,
        HTTP_METHOD_TYPE_POST = 1,
        HTTP_METHOD_TYPE_HEAD = 2,
        HTTP_METHOD_TYPE_PUT = 3,
        HTTP_METHOD_TYPE_DELETE = 4,
        HTTP_METHOD_TYPE_TRACE = 5,
        HTTP_METHOD_TYPE_CONNECT = 6,
        HTTP_METHOD_TYPE_OPTIONS = 7,
        HTTP_METHOD_TYPE_END = 10,
        HTTP_METHOD_TYPE_INVALID = 100
    };

    const int kMaxHttpPacketLength = 20 * 1024 * 1024; //单个http包，不能超过20M

    class CHttpParse {
    private:
        int i_err_code_;
        std::string str_err_msg_;
        
        //common
        std::string str_postdata_;
        std::map<std::string, std::string> map_headers_;
        int i_pkg_type_;
        unsigned int i_content_len_;
        
        //requst
        //std::string str_method_;
        int i_method_type_;
        std::string str_url_;
        std::string str_querystring_;

        //response
        float f_http_version_; 
        int i_status_;

        bool b_parsed_;
        
    public:
        CHttpParse();
        ~CHttpParse();

        
        int ContentLen();
        std::string PostData() { return str_postdata_; };
        bool IsParsed() { return b_parsed_;};
        int PkgType() { return i_pkg_type_;};
        int MethodType() { return i_method_type_;};
        string QueryUrl() { return str_url_;};
        string QueryString() { return str_querystring_;};
        string GetHeader(const string& key);
        string GetQueryPath();

        //解析http协议包
        int ParsePkg(const std::string& data);

        //目前只支持POST请求，检查http请求的完整性
        int CheckPkg(const string& data);

        //组响应包
        static int BuildResponse(const std::string& rsp_data, std::string& str_rsp);
        static int BuildResponse(int  i_status, std::string& str_rsp) ;
        static int BuildResponse(const std::string& rsp_data, const std::string&str_headers, std::string& str_rsp);

        //组请求包
        static int BuildRequst(const std::string& str_url, const std::string& str_data, std::string& str_req_body);

        //组请求包时，需要支持添加请求头部的组包函数
        
        
        
        void Clear();

    private:
        int _ParseResponse(const std::string& str_data);
        int _ParseRequest(const std::string& data);
        int _ParseGetRequest();
        int _ParsePostRequst();
        int _ParseFirstWord(const std::string& str); 
        int _ParseHeaders(const string& str_headers);
        int _ParseChunkData();

        int _CheckGetRequest(const string& data);
        int _CheckPostRequest(const std::string& data);

        
        //检查http响应包的完整性
        int _CheckResponse(const std::string& str_rsp);
        int _CheckChunkResponse(const std::string& str_rsp);
        int _CheckContentLengthResponse(const std::string& str_rsp, int head_len);
        
        bool _ValidHttpVersion(const string& str);
    };//end of CHttpParse
};//end of http
#endif

