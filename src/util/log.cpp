#include "util/log.h"
#include <stdarg.h>
#include "common/config.h"

namespace util {
/*
    int Log(int log_level, const char* format, ...) 
    {
        char buf[kMaxLogLength];
        memset(buf, 0, kMaxLogLength);
        va_list ap;
        va_start(ap, format);
        int len = vsnprintf(buf, kMaxLogLength, format, ap); 
        va_end(ap);

        if (len < 0) {
            ERROR("log string is too long, over 4096.");
            return -1;
        }

        std::string str_log;
        str_log.append(buf, len);

        switch(log_level) {
            case TRACE:
                TRACE(str_log);
                //util::LogIns::Instance().log()
                break;

            case DEBUG:
                DEBUG(str_log);
                //LogIns::Instance().log().log(DEBUG_LOG_LEVEL, str_log, __FILE__, __LINE__);
                break;

            case INFO:
                INFO(str_log);
                break;

            case WARN:
                WARNING(str_log);
                break;

            case ERROR:
                ERROR(str_log);
                break;

            default:
                ERROR("invalid log_level");
                return -1;
        }
        
        return 0;
    }
*/

    int error_log_(int log_level, int line, char* file,  const char* format, ...) 
    {
        char buf[kMaxLogLength];
        memset(buf, 0, kMaxLogLength);
        va_list ap;
        va_start(ap, format);
        int len = vsnprintf(buf, kMaxLogLength, format, ap); 
        va_end(ap);

        if (len < 0) {
            ERROR("log string is too long, over 4096.");
            return -1;
        }

        std::string str_log;
        str_log.append(buf, len);

        switch(log_level) {
            case TRACE:
                //TRACE(str_log);
                LogIns::Instance().error_log().log(TRACE_LOG_LEVEL, str_log, file, line);
                break;

            case DEBUG:
                //DEBUG(str_log);
                LogIns::Instance().error_log().log(DEBUG_LOG_LEVEL, str_log, file, line);
                break;

            case INFO:
                //INFO(str_log);
                LogIns::Instance().error_log().log(INFO_LOG_LEVEL, str_log, file, line);
                break;

            case WARN:
                //WARNING(str_log);
                LogIns::Instance().error_log().log(WARN_LOG_LEVEL, str_log, file, line);
                break;

            case ERROR:
                //ERROR(str_log);
                LogIns::Instance().error_log().log(ERROR_LOG_LEVEL, str_log, file, line);
                break;

            default:
                ERROR("invalid log_level");
                return -1;
        }
        
        return 0;
    }

    int flow_log_(int log_level, int line, char* file,  const char* format, ...) 
    {
        char buf[kMaxLogLength];
        memset(buf, 0, kMaxLogLength);
        va_list ap;
        va_start(ap, format);
        int len = vsnprintf(buf, kMaxLogLength, format, ap); 
        va_end(ap);

        if (len < 0) {
            ERROR("log string is too long, over 4096.");
            return -1;
        }

        std::string str_log;
        str_log.append(buf, len);

        switch(log_level) {
            case TRACE:
                //TRACE(str_log);
                LogIns::Instance().flow_log().log(TRACE_LOG_LEVEL, str_log, file, line);
                break;

            case DEBUG:
                //DEBUG(str_log);
                LogIns::Instance().flow_log().log(DEBUG_LOG_LEVEL, str_log, file, line);
                break;

            case INFO:
                //INFO(str_log);
                LogIns::Instance().flow_log().log(INFO_LOG_LEVEL, str_log, file, line);
                break;

            case WARN:
                //WARNING(str_log);
                LogIns::Instance().flow_log().log(WARN_LOG_LEVEL, str_log, file, line);
                break;

            case ERROR:
                //ERROR(str_log);
                LogIns::Instance().flow_log().log(ERROR_LOG_LEVEL, str_log, file, line);
                break;

            default:
                ERROR("invalid log_level");
                return -1;
        }
        
        return 0;
    }


    CLog::CLog() 
    {
        
    }
    
    CLog::~CLog() 
    {
       
    }

    int CLog::init()
    {
        common::CConfig& config = common::CConfigIns::Instance();
        PropertyConfigurator::doConfigure(config.GetLogConfFile().c_str());
		error_log_ = Logger::getRoot(); //错误日志
        flow_log_ = Logger::getInstance(config.GetLoggerName().c_str()); //流水日志

        return 0;
        
    }
}

