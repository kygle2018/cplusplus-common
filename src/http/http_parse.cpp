#include "http/http_parse.h"
#include "util/util.h"
#include "common/uls_log.h"

using namespace util;

namespace http {
    CHttpParse::CHttpParse() 
    {
        this->b_parsed_ = false;
        this->i_err_code_ = 0;
        this->i_status_ = 0;
        this->i_pkg_type_ = HTTP_PKG_TYPE_REQUEST;
        this->i_method_type_ = HTTP_METHOD_TYPE_INVALID;
        this->f_http_version_ = 0;
        this->i_content_len_ = 0;
    }

    CHttpParse::~CHttpParse() 
    {
        
    }

    void CHttpParse::Clear()
    {
        this->str_err_msg_.clear();
        this->str_postdata_.clear();
        this->str_querystring_.clear();
        this->str_url_.clear();

        this->i_err_code_ = 0;
        this->i_status_ = 0;
        this->b_parsed_ = false;
        this->i_pkg_type_ = HTTP_PKG_TYPE_REQUEST;
        this->i_method_type_ = HTTP_METHOD_TYPE_INVALID;
        this->f_http_version_ = 0;
        this->i_content_len_ = 0;
    }

    int CHttpParse::ContentLen()
    {
        if (false == this->b_parsed_) {
            this->i_err_code_ = ERR_NO_NOT_PARSED;
            this->str_err_msg_ = "http is not parsed.";
            return -1;
        }
        
        return this->i_content_len_;
    }

    //
    string CHttpParse::GetHeader(const string& key)
    {
        string str_ret;
        map<string, string>::iterator it;
        string find_key = util::StrToLower(key);

        it = map_headers_.find(find_key);

        if (it != map_headers_.end()) {
            str_ret = it->second;
        }

        return str_ret;
    }

    string CHttpParse::GetQueryPath()
    {
        string str_ret;
        vector<string> vec_tmp;
        util::Split( this->str_url_, "?", vec_tmp);
        if (vec_tmp.size() <= 2 && vec_tmp.size() > 0) {
            str_ret = vec_tmp[0];
        }

        return str_ret;
    }

    int CHttpParse::CheckPkg(const string& data)
    {
        if (data.size() < 5) {
            return 0;
        }

        int ret = 0;

        if (data.substr(0, 5) == "HTTP/") {
            ret = _CheckResponse(data);
        } else if (data.substr(0, 5) == "POST ") {
            ret = _CheckPostRequest(data);
        } else if (data.substr(0, 4) == "GET ") {
            ret = _CheckGetRequest(data);
        } else {
            ret = -1;
        }

        if (ret == 0) {
            if (data.size() > kMaxHttpPacketLength) {
                ret = -1;
            }
        }

        return ret;
    }

    int CHttpParse::ParsePkg(const std::string& data)
    {
        if (true == this->b_parsed_) {
            this->i_err_code_ = ERR_NO_HAS_PARSED;
            this->str_err_msg_ = "http is parsed.";
            ULS_LOG(_LC_ERROR_, " http is parsed.");
            return -1;
        }

        std::size_t found = data.find(" ");
        if (found == std::string::npos) {
            this->i_err_code_ = ERR_NO_INVALID_HTTP_PKG;
            this->str_err_msg_ = "invalid http pkg.";
            ULS_LOG(_LC_ERROR_, "invalid http pkg.");
            return -2;
        }

        string str_tmp = data.substr(0, found);

        int ret = _ParseFirstWord(str_tmp);
        if (ret != 0) {
            this->i_err_code_ = ERR_NO_INVALID_HTTP_PKG;
            this->str_err_msg_ = "invalid http pkg.";
            ULS_LOG(_LC_ERROR_, "invalid http pkg.");
            return -3;
        }

        if (this->i_pkg_type_ == HTTP_PKG_TYPE_REQUEST) {
            ret = _ParseRequest(data);      
        } else if (this->i_pkg_type_ == HTTP_PKG_TYPE_RESPONSE) {
            ret = _ParseResponse(data);
        } else {
            this->i_err_code_ = ERR_NO_INVALID_HTTP_PKG;
            this->str_err_msg_ = "invlid http pkg type.";
            ULS_LOG(_LC_ERROR_, "invalid http pkg type.");
            return -5;
        }

        if (ret != 0 ) {
            this->i_err_code_ = ERR_NO_INVALID_HTTP_PKG;
            this->str_err_msg_ = "invalid http pkg.";
            ULS_LOG(_LC_ERROR_, "invalid http pkg.");
            return -6;
        }
        this->b_parsed_ = true;

        return 0;
    }

    int CHttpParse::_ParseFirstWord(const std::string& str) 
    {
        //ULS_LOG(_LC_DEBUG_, "first word is %s.", str.c_str());
        string http_response_eg = "HTTP/1.1";
        if (str.substr(0, 5) == "HTTP/") {          
            if (str.size() != http_response_eg.size()) {
                //i_err_code_ = ERR_NO_INVALID_HTTP_PKG;
                ULS_LOG(_LC_ERROR_, "invalid http pkg.");
                return -1;
            }

            f_http_version_ = util::from_str<float>(str.substr(5));
            this->i_pkg_type_ = HTTP_PKG_TYPE_RESPONSE;
        } else {
            i_pkg_type_ = HTTP_PKG_TYPE_REQUEST;
            if (str == "GET") {
                this->i_method_type_ = HTTP_METHOD_TYPE_GET;
            } 
            else if (str == "POST") {
                this->i_method_type_ = HTTP_METHOD_TYPE_POST;
            } 
            else {
                this->i_method_type_ = HTTP_METHOD_TYPE_INVALID;
                //i_err_code_ = ERR_NO_INVALID_HTTP_PKG;
                ULS_LOG(_LC_ERROR_, "invalid http method type.");
                return -2;
            }
        }

        return 0;
    }

    int CHttpParse::_ParseRequest(const std::string& data)
    {
        if (true == this->b_parsed_) {
            this->i_err_code_ = ERR_NO_HAS_PARSED;
            this->str_err_msg_ = "http is parsed.";
            ULS_LOG(_LC_ERROR_, "http is parsed.");
            return -1;
        }

        //获取请求url
        std::size_t found = data.find(" ");
        if ( found == std::string::npos) {
            this->i_err_code_ = ERR_NO_INVALID_REQUEST_FORMAT;
            ULS_LOG(_LC_ERROR_, "cannot find space.");
            return -1;
        }

        unsigned int start = 0;

        start = found + 1;
        found = data.find(" ", start);
        if ( found == std::string::npos) {
            this->i_err_code_ = ERR_NO_INVALID_REQUEST_FORMAT;
            ULS_LOG(_LC_ERROR_, "cannot find space.");
            return -3;
        }
        str_url_ = data.substr(start, found - start);

        start = found + 1;
        found = data.find("\r\n", start);
        if (found == std::string::npos) {
            this->i_err_code_ = ERR_NO_INVALID_REQUEST_FORMAT;
            ULS_LOG(_LC_ERROR_, "invalid request format.");
            return -4;
        }

        string str_tmp = data.substr(start, found - start);
        if (str_tmp.substr(0, 5) != "HTTP/") {
            this->i_err_code_ = ERR_NO_INVALID_REQUEST_FORMAT;
            ULS_LOG(_LC_ERROR_, "invalid request format.");
            return -5;
        }

        start = start + 5;
        str_tmp = data.substr(start, found - start);
        if (!_ValidHttpVersion(str_tmp)) {
            this->i_err_code_ = ERR_NO_INVALID_REQUEST_FORMAT;
            ULS_LOG(_LC_ERROR_, "invalid request format.");
            return -6;
        }

        f_http_version_ = util::from_str<float>(str_tmp);

        start = found + 2;
        found = data.find("\r\n\r\n", start);
        if (found == string::npos) {
            this->i_err_code_ = ERR_NO_INVALID_REQUEST_FORMAT;
            ULS_LOG(_LC_ERROR_, "invalid request format.");
            return -7;
        }

        string str_headers = data.substr(start, found - start);
        int ret = _ParseHeaders(str_headers);
        if (ret != 0) {
            ULS_LOG(_LC_ERROR_, "_ParseHeaders failed. ret:%d", ret);
            return -8;
        }

        start = found + 4;
        this->str_postdata_ = data.substr(start);

        //if (i_method_type_ == HTTP_METHOD_TYPE_GET) {
        //不管是get请求还是post请求都需要解析querystring
        ret = _ParseGetRequest();
        if (ret != 0) {
            ULS_LOG(_LC_ERROR_,"parst querystring failed.");
            return -11;
        }
        //} 
        if (i_method_type_ == HTTP_METHOD_TYPE_GET) {
            //不做特殊处理
        } else if (i_method_type_ == HTTP_METHOD_TYPE_POST) {
            ret = _ParsePostRequst();
        }
        else {
            ULS_LOG(_LC_ERROR_, "invalid method type: %d", i_method_type_);
            return -9;
        }

        if (ret != 0) {
            ULS_LOG(_LC_ERROR_, "Parse data failed. ret: %d", ret);
            return -10;
        }

        return 0;
    }

    int CHttpParse::_ParseHeaders(const string& str_headers)
    {
        vector<string> vec_lines;
        util::Split( str_headers, "\r\n", vec_lines);
        for(size_t i = 0; i < vec_lines.size(); i++) {
            size_t found = vec_lines[i].find(":");
            if (found == string::npos) {
                return -1;
            }

            string key = vec_lines[i].substr(0, found);
            string value = vec_lines[i].substr(found + 1);
            key = util::Trim(key);
            key = util::StrToLower(key);
            value = util::Trim(value);
            this->map_headers_[key] = value;
        }

        return 0;
    }

    int CHttpParse::_ParsePostRequst()
    {
        if (true == this->b_parsed_) {
            this->i_err_code_ = ERR_NO_HAS_PARSED;
            this->str_err_msg_ = "http is parsed.";
            return -1;
        }

        //头部header字段都已经转成小写保存在map中了  
        map<string, string>::iterator it = this->map_headers_.find("content-length");
        if (it == this->map_headers_.end()) {
            //兼容不带Content-Length的post请求
            return 0;
        }

        string str_len = it->second;
        if (!util::IsNum(str_len)) {
            return -3;
        }

        this->i_content_len_ = util::from_str<unsigned int>(str_len);

        if (this->i_content_len_ != str_postdata_.size()) {
            return -4;
        }

        return 0;       
    }

    int CHttpParse::_ParseGetRequest()
    {
        if (true == this->b_parsed_) {
            return -1;
        }

        size_t found = this->str_url_.find("?");
        if (found != string::npos) {
            this->str_querystring_ = this->str_url_.substr(found + 1);
        }

        return 0;
    }

    //返回0表示包没有收完整，需要继续收包, 需要详细的检查是否是http包，目前检查的不完善，需要优化
    int CHttpParse::_CheckPostRequest(const std::string& data) 
    {
        std::string str_tmp_data = util::StrToLower(data);
        
        std::string find_string = "content-length:";
        std::size_t found = str_tmp_data.find(find_string);
        if (found == std::string::npos) 
        {
            //如果Content-Length没有找到，就认为body为空
            find_string = "\r\n\r\n";
            found = data.find(find_string);
            if (found == std::string::npos) {
                return 0;
            }
            
            return found + find_string.size();
        }
        
        unsigned int start = 0;

        start = found + find_string.length();
        find_string = "\r\n";
        found = data.find(find_string, start);
        if (found == std::string::npos) {
            return 0;
        }

        std::string str_len = data.substr(start, found - start);
        int content_len = atoi(str_len.c_str());

        find_string = "\r\n\r\n";
        found = data.find(find_string);
        if (found == std::string::npos) {
            return 0;
        }

        start = found + find_string.length();
        if (start + content_len > data.length()) {
            return 0;
        }

        return (start + content_len);
    }

    int CHttpParse::_CheckGetRequest(const string& data)
    {
        string find_string = "\r\n\r\n";
        size_t found = data.find(find_string);
        if (found == std::string::npos) {
            return 0;
        }

        return (found + find_string.size());
    }

    int CHttpParse::BuildResponse(const std::string& rsp_data,  std::string& str_rsp)
    {
       str_rsp.clear();
       uint32_t content_len = rsp_data.size();
       
       str_rsp += "HTTP/1.1 200 OK\r\n";
       str_rsp += "Content-type: application/json\r\n";
       str_rsp += "Content-Length: " + util::GetStrFromUInt32(content_len) + "\r\n";
       str_rsp += "Connection: Keep-Alive\r\n";
       str_rsp += "\r\n";

       str_rsp.append(rsp_data);

       return 0;
    }

    int CHttpParse::BuildResponse(const std::string& rsp_data, const std::string&str_headers, std::string& str_rsp)
    {
       str_rsp.clear();
       uint32_t content_len = rsp_data.size();
       
       str_rsp += "HTTP/1.1 200 OK\r\n";
       if (str_headers.empty()) {
            str_rsp += "Content-type: application/json\r\n";
       } else {
            str_rsp.append(str_headers);
       }
       str_rsp += "Content-Length: " + util::GetStrFromUInt32(content_len) + "\r\n";
       str_rsp += "Connection: Keep-Alive\r\n";
       str_rsp += "\r\n";

       str_rsp.append(rsp_data);

       return 0;
    }


    int CHttpParse::BuildResponse(int  i_status, std::string& str_rsp) 
    {
        if (i_status == HTTP_STATUS_CODE_LOOP_DETECTED) {
            str_rsp += "HTTP/1.1 508 Loop Detected\r\n";
            str_rsp += "Content-Length: 0\r\n";
            str_rsp += "Connection: close\r\n";
            str_rsp += "\r\n";
        } else if (i_status == HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR) {
            str_rsp += "HTTP/1.1 500 Internal Server Error\r\n";
            str_rsp += "Content-Length: 0\r\n";
            str_rsp += "Connection: close\r\n";
            str_rsp += "\r\n";
        }

        return 0;
    }

    //如果str_data为空，表示发送GET请求
    int CHttpParse::BuildRequst(const std::string& str_url, const std::string& str_data, std::string& str_req_body)
    {
        std::string path;
        std::string host;
        bool has_data = false;
        str_req_body.clear();
        if (str_data.empty()) {
            str_req_body += "GET ";
            has_data = false;
        } else {
            str_req_body += "POST ";
            has_data = true;
        }

        unsigned int start = 0;
        std::size_t found = str_url.find("//");
        if (found != std::string::npos) {
            start = found + 2;
        }

        found = str_url.find('/', start);
        if (found == std::string::npos) {
            //util::Log(util::ERROR, "invalid url. url:%s\n", str_url.c_str());
            //return -1;
            path = "/";
            host = str_url.substr(start);
        } else {
            path = str_url.substr(found);
            host = str_url.substr(start, found - start);
        }

        str_req_body += path;
        str_req_body += " HTTP/1.1\r\n";

        str_req_body = str_req_body + "Host: " + host + "\r\n";

        if (has_data) {
            str_req_body += "Content-Type: text/html; charset=utf-8\r\n";
            str_req_body = str_req_body + "Content-Length:" + util::GetStrFromUInt32(str_data.size()) + "\r\n";
            str_req_body += "\r\n";
            str_req_body += str_data;
        } else {
            str_req_body += "\r\n";
        }
        
        return 0;
    }   

    int CHttpParse::_ParseResponse(const std::string& str_res_data)
    {
        std::size_t found = str_res_data.find("\r\n");
        if ( found == std::string::npos) {
            //util::Error_log(util::ERROR, "invalid response. str_res_data:%s\n", str_res_data.c_str());
            this->i_err_code_ = ERR_NO_INVALID_RESPONSE_FORMAT;
            this->str_err_msg_ = "invalid respnse. str_res_data:" + str_res_data;
            return -1;
        }
        
        std::string status_line = str_res_data.substr(0, found);
        unsigned int start = 0;

        found = status_line.find(" ");
        if (found == std::string::npos) {
            //util::Error_log(util::ERROR, "invalid response. str_res_data:%s\n", str_res_data.c_str());
            this->i_err_code_ = ERR_NO_INVALID_RESPONSE_FORMAT;
            this->str_err_msg_ = "invalid respnse. str_res_data:" + str_res_data;
            return -2;
        }

        start = found + 1;
        found = status_line.find(" ");
        if (found == std::string::npos) {
            this->i_err_code_ = ERR_NO_INVALID_RESPONSE_FORMAT;
            this->str_err_msg_ = "invalid respnse. str_res_data:" + str_res_data;
            return -3;
        }

        i_status_ = atoi(status_line.substr(start, found - start).c_str());

        found = str_res_data.find("\r\n\r\n");
        if (found != std::string::npos) {
            str_postdata_ = str_res_data.substr(found + 4);
        } else {
            this->i_err_code_ = ERR_NO_INVALID_RESPONSE_FORMAT;
            this->str_err_msg_ = "invalid respnse. str_res_data:" + str_res_data;
            return -4;
        }

        start = status_line.size() + 2;
        string str_headers = str_res_data.substr(start, found - start);

        int i_ret = _ParseHeaders(str_headers);
        if (i_ret != 0) {
            ULS_LOG(_LC_ERROR_, "_ParseHeaders failed. i_ret: %d", i_ret);
            return -5;
        }

        if (GetHeader("Transfer-Encoding") == "chunked") {

            i_ret = _ParseChunkData();
            if (i_ret != 0) {
                ULS_LOG(_LC_ERROR_, "_ParseChunkData failed. i_ret: %d", i_ret);
                return -6;
            }
        }

        if (i_status_ != HTTP_STATUS_CODE_SUCCESS) {
            //util::Error_log(util::ERROR, "http status not 200. str_res_data:%s\n", str_res_data.c_str());
            //return -2;
        }

        return 0;
    }

    int CHttpParse::_ParseChunkData() 
    {
        //ULS_LOG(_LC_DEBUG_, "str_postdata_: %s", str_postdata_.c_str());
        string str_data, str_tmp;
        unsigned tmp_len = 0;
        size_t found = 0;
        size_t start = 0;
        
        while(1) {
            //ULS_LOG(_LC_DEBUG_, "start: %u, found:%u", start, found);
            
            found = str_postdata_.find("\r\n", start);
            if (found == string::npos) {
                return -1;
            }

            //ULS_LOG(_LC_DEBUG_, "start: %u, found:%u", start, found);
            str_tmp = str_postdata_.substr(start, found - start);

            //ULS_LOG(_LC_DEBUG_, "str_tmp: %s", str_tmp.c_str());
            sscanf(str_tmp.c_str(), "%x",  &tmp_len);

            //ULS_LOG(_LC_DEBUG_, "start: %u, found:%u", start, found);
            if (str_tmp.empty()) {
                break;
            }

            if (tmp_len == 0) {
                break;
            }

            start = found + 2;
            str_data.append(str_postdata_.substr(start, tmp_len));

            //ULS_LOG(_LC_DEBUG_, "start: %u, found:%u", start, found);
            
            start += tmp_len;
            found = str_postdata_.find("\r\n", start);

            if (found == string::npos) {
                return -1;
            }

            //ULS_LOG(_LC_DEBUG_, "start: %u, found:%u", start, found);
            start = found + 2;
        }

        i_content_len_ = str_data.size();
        str_postdata_ = str_data;

        //ULS_LOG(_LC_DEBUG_, "str_data: %s", str_data.c_str());

        return 0;
    }

    int CHttpParse::_CheckResponse(const std::string& str_rsp)
    {
        std::string str_tmp_rsp = util::StrToLower(str_rsp);
        std::size_t found = str_rsp.find("\r\n\r\n");
        if (found == std::string::npos) {
            return 0;
        }

        //unsigned int data_len = str_rsp.size() - found - 4;
        unsigned int head_len = found + 4;

        int i_ret = 0;
        if (str_tmp_rsp.find("transfer-encoding:") != string::npos) {
            i_ret = _CheckChunkResponse(str_rsp);
        } 
        else if (str_tmp_rsp.find("content-length:") != string::npos) {
            i_ret = _CheckContentLengthResponse(str_rsp, head_len);
        } else {
            i_ret = head_len;
        }

        return i_ret;
    }

    //存在逻辑异常，需要进行处理
    int CHttpParse::_CheckChunkResponse(const std::string& str_rsp)
    {
        string str_find = "\r\n0\r\n\r\n";
        size_t found = str_rsp.find(str_find);
        if (found == string::npos) {
            return 0;
        }

        return (found + str_find.size());
    }

    int CHttpParse::_CheckContentLengthResponse(const std::string& str_rsp, int head_len)
    {
        std::string str_find = "content-length:";

        std::string str_tmp_rsp = util::StrToLower(str_rsp);

        size_t found = str_tmp_rsp.find(str_find);
        if (found == std::string::npos) {
            return -1;
        }

        unsigned int start = found + str_find.size();
        found = str_rsp.find("\r\n", start);
        if (found == std::string::npos || found < start) {
            return -1;
        }

        std::string str_content_len = str_rsp.substr(start, found - start);
        if (str_rsp.size() < atoi(str_content_len.c_str()) + head_len) {
            return 0;
        }
        
        return (head_len + atoi(str_content_len.c_str()));
    }

    bool CHttpParse::_ValidHttpVersion(const string& str) 
    {
        if (str.size() != 3) {
            return false;
        }

        if (str.at(0) < '0' || str.at(0) > '9') {
            return false;
        }

        if (str.at(1) != '.') {
            return false;
        }

        if (str.at(2) < '0' || str.at(2) > '9') {
            return false;
        }

        return true;
    }

    
};

