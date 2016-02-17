#ifndef __UTIL_LOG_H__
#define __UTIL_LOG_H__
#include <string>
//#include <glog/logging.h>
#include <util/singleton.h>
#include <log4cplus/logger.h>
#include <log4cplus/configurator.h>
#include <log4cplus/logger.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/layout.h>
#include <log4cplus/loggingmacros.h>

using namespace log4cplus;

namespace util {

const int kMaxLogLength = 4096;

enum LOG_LEVEL {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
};

#define Error_log(log_level, format, ...)  error_log_(log_level, __LINE__, __FILE__, format, ## __VA_ARGS__);
#define Flow_log(log_level, format, ...)  flow_log_(log_level, __LINE__, __FILE__, format, ## __VA_ARGS__);


//extern int Log(int log_level, const char* format, ...);
extern int error_log_(int log_level, int line,  char* file, const char* format, ...);
extern int flow_log_(int log_level, int line,  char* file, const char* format, ...);


#define TRACE(p) LOG4CPLUS_TRACE(util::LogIns::Instance().error_log(), p) 
#define DEBUG(p) LOG4CPLUS_DEBUG(util::LogIns::Instance().error_log(), p) 
#define INFO(p) LOG4CPLUS_INFO(util::LogIns::Instance().error_log(), p) 
#define WARNING(p) LOG4CPLUS_WARN(util::LogIns::Instance().error_log(), p) 
#define ERROR(p) LOG4CPLUS_ERROR(util::LogIns::Instance().error_log(), p)

class CLog {
    public:
        CLog();
        ~CLog();

        //log_level: 0->info, 2->error.
        int init();
        Logger& error_log() { return error_log_;};
		Logger& flow_log() { return flow_log_;};

    private:
        Logger error_log_;
		Logger flow_log_;
};

typedef util::Singleton<CLog> LogIns;

}
#endif
