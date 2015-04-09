/* 
 * File:   log.h
 * Author: Rohit Joshi <rohit.c.joshi@gmail.com>
 *
 * Created on March 5, 2015, 6:56 AM
 */

#ifndef LOG_H
#define	LOG_H
#include <unistd.h>
#include <sstream>
//#include <boost/format.hpp>
#include "thirdparty/spdlog/spdlog.h"


 
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT __FILE__ ":" TOSTRING(__LINE__)
#define TID pthread_self()
#define PID getpid()

#ifdef __GNUC__ //__GNUC__
#define _FUNC_NAME_ __PRETTY_FUNCTION__
#else
#define _FUNC_NAME_ _func_
#endif //__GNUC__

#define stringify( name ) # name
#define FORMAT_BUFFER_SIZE 8192

#undef LOG_TRACE_IN
#undef LOG_TRACE_OUT
#undef LOG_TRACE
#undef LOG_DEBUG
#undef LOG_INFO
#undef LOG_EVENT
#undef LOG_WARNING
#undef LOG_ERROR
#undef LOG_CRITICAL
#undef LOG_ALERT
#undef LOG_EMERG
#undef LOG_OFF

namespace lightq {
    
    //log class wrapper
    class log {
    public:
        /**
	 * LogLevel eum
	 */
	typedef enum {
		LOG_TRACE_IN = -2,
		LOG_TRACE_OUT = -1,
		LOG_TRACE = 0,
                LOG_DEBUG = 1,
		LOG_INFO = 2,
		LOG_EVENT = 3,
		LOG_WARNING = 4,
		LOG_ERROR = 5,
                LOG_CRITICAL = 6,
                LOG_ALERT = 7,
                LOG_EMERG = 8,
                LOG_OFF = 9

	} logtype_t;
       
        
       
        /**
         * get logger
         * @param lightq_logger
         * @return 
         */
        inline static std::shared_ptr<spdlog::logger>& logger() {
            static std::shared_ptr<spdlog::logger>  logger = spdlog::get("lightq_logger");
            return logger;
        }
        /**
         * event logger
         * @return 
         */
        inline static std::shared_ptr<spdlog::logger>& event_logger() {
            static std::shared_ptr<spdlog::logger>  logger = spdlog::get("lightq_event_logger");
            return logger;
        }
        
        /**
         * initialize logging default
         * @param level
         * @return 
         */
        static bool init(const std::string& process_name, spdlog::level::level_enum level = spdlog::level::notice) {
            
            std::string logfile("logs/");
            std::istringstream ss(process_name);
            std::string token;
            std::vector<std::string> tokens;
            tokens.reserve(10);
            while (std::getline(ss, token, '/')) {
                tokens.push_back(token);
            }
            if(tokens.size() > 0) {
                logfile.append(tokens[tokens.size()-1]);
            }else {
                logfile.append(process_name);
            }
            logfile.append("_");
            //uint32_t pid = getpid();
            
            //initialize details logger
            std::string details_logfile(logfile);
            details_logfile.append(std::to_string(PID));
            details_logfile.append(".log");
#ifdef DEBUG
             auto lightq_logger = spdlog::stdout_logger_mt("lightq_logger");
#else
            auto lightq_logger = spdlog::rotating_logger_mt("lightq_logger", details_logfile, 1048576 * 500, 10);
           // spdlog::set_async_mode(1048576); //queue size
#endif
            lightq_logger->set_level(level);
            
            
            //initialize event logger
            std::string event_logfile(logfile);
            event_logfile.append("events_");
            event_logfile.append(std::to_string(PID));
            event_logfile.append(".log");
            auto lightq_event_logger = spdlog::rotating_logger_mt("lightq_event_logger", event_logfile, 1048576 * 500, 10);
            //spdlog::set_async_mode(1048576); //queue size
            lightq_event_logger->set_level(level);
            return true;
        }
        
        
         /**
     * write log
     * @param logtype
     * @param file
     * @param line
     * @param func
     * @param buf
     * @param ...
     */
    static void log_write(int logtype, const char* file, unsigned int line,
            const char* func, const char *buf, ...) {
        
        va_list args;
        va_start(args, buf);
        std::string msg(log::format_arg_list(buf, args));
        va_end(args);
        std::string func_inout(func);
        func_inout.append("|");

        if (logtype == log::LOG_TRACE_IN) {
            func_inout.append("-->IN|");
            logtype = log::LOG_TRACE;
        }
        else if (logtype == log::LOG_TRACE_OUT) {
            func_inout.append("<--OUT|");
            logtype = log::LOG_TRACE;
        }


        const char* filename = file;
        const char* p = strrchr(file, '/');
        if (p) {
            filename = ++p;
        }

#ifdef __APPLE__
        //get threadid
        uint64_t tid;
        pthread_threadid_np(NULL, &tid);
        
        char szBuf[FORMAT_BUFFER_SIZE];
        szBuf[0] = '\0';
        sprintf(szBuf,"|%5d|%8llX|%s:%u|%s%s", PID, (tid & 0xffffffff), filename, line, func_inout.c_str(), msg.c_str());
     
        if(logtype == log::LOG_EVENT) {
            event_logger()->notice(szBuf);
           // event_logger()->notice("|%5d|%8X|%s:%u|%s%s", PID, (tid & 0xffffffff), filename, line, func_inout.c_str(), msg.c_str());
        
        }
       
        if(logger()->should_log((spdlog::level::level_enum)logtype)) {
            logger()->force_log((spdlog::level::level_enum)logtype,szBuf);
          //  logger()->force_log((spdlog::level::level_enum)logtype,"|%5d|%8X|%s:%u|%s%s", PID, (tid & 0xffffffff), filename, line, func_inout.c_str(), msg.c_str());
        }
      
#else
        if(logtype == log::LOG_EVENT) {
            event_logger()->notice("|%5d|%8llX|%s:%u|%s|%s", PID, (TID & 0xffffffff), filename,line,func_inout.c_str(), msg.c_str());
        }
        if(logger()->should_log((spdlog::level::level_enum)logtype)) {
            logger()->force_log((spdlog::level::level_enum)logtype,"|%5d|%8X|%s:%u|%s%s", PID, (TID & 0xffffffff), filename, line, func_inout.c_str(), msg.c_str());
        }
        
#endif
    }
    
    /**
     * format log
     * @param pszFormat
     * @param args
     * @return 
     */
    static std::string format_arg_list(const char *pszFormat, va_list args) {
        if (!pszFormat) return "";

        char szBuf[FORMAT_BUFFER_SIZE];
        int nRet = vsnprintf(szBuf, FORMAT_BUFFER_SIZE, pszFormat, args);
        if (nRet >= (int) FORMAT_BUFFER_SIZE) {
            // redo the formatting
            char *buffer = (char*) malloc((nRet + 1) * sizeof (char));
            vsnprintf(buffer, nRet, pszFormat, args);
            std::string sMsg(buffer, nRet);
            free(buffer);
            return std::move(sMsg);
        } else {
            std::string sMsg(szBuf, nRet);
            return std::move(sMsg);
        }
    }

        
    };


/**
 * LOG_EMERGENCY logging macro
 */
#define LOG_EMERG(msg, ...) \
        if(log::logger().get() && log::logger()->should_log(spdlog::level::emerg)) \
            log::log_write(log::LOG_EMERG, __FILE__, __LINE__, _FUNC_NAME_, msg, ## __VA_ARGS__);
/**
 * LOG_ALERT logging macro
 */
#define LOG_ALERT(msg, ...) \
        if(log::logger().get() && log::logger()->should_log(spdlog::level::alert)) \
            log::log_write(log::LOG_ALERT, __FILE__, __LINE__, _FUNC_NAME_, msg, ## __VA_ARGS__);
    
/**
 * LOG_CRITICAL logging macro
 */
#define LOG_CRITICAL(msg, ...) \
        if(log::logger().get() && log::logger()->should_log(spdlog::level::critical)) \
            log::log_write(log::LOG_CRITICAL, __FILE__, __LINE__, _FUNC_NAME_, msg, ## __VA_ARGS__);
/**
 * LOG_ERROR logging macro
 */
#define LOG_ERROR(msg, ...) \
        if(log::logger().get() && log::logger()->should_log(spdlog::level::err)) \
            log::log_write(log::LOG_INFO, __FILE__, __LINE__, _FUNC_NAME_, msg, ## __VA_ARGS__);
/**
 * LOG_WARN logging macro
 */
#define LOG_WARN(msg, ...) \
       if(log::logger().get() && log::logger()->should_log(spdlog::level::warn)) \
            log::log_write(log::LOG_WARN, __FILE__, __LINE__, _FUNC_NAME_, msg, ## __VA_ARGS__);    
/**
 * LOG_EVENT logging macro
 */
#define LOG_EVENT(msg, ...) \
        if(log::event_logger().get() && log::logger()->should_log(spdlog::level::notice)) \
            log::log_write(log::LOG_EVENT, __FILE__, __LINE__, _FUNC_NAME_, msg, ## __VA_ARGS__);

    /**
 * LOG_INFO logging macro
 */
#define LOG_INFO(msg, ...) \
        if(log::logger().get() && log::logger()->should_log(spdlog::level::info)) \
            log::log_write(log::LOG_INFO, __FILE__, __LINE__, _FUNC_NAME_, msg, ## __VA_ARGS__);
    
/**
 * DEBUG logging macro
 */
#define LOG_DEBUG(msg, ...) \
        if(log::logger().get() && log::logger()->should_log(spdlog::level::debug)) \
            log::log_write(log::LOG_DEBUG, __FILE__, __LINE__, _FUNC_NAME_, msg, ## __VA_ARGS__);


/**
 * LOG_TRACE logging macro
 */
#define LOG_TRACE(msg, ...) \
       if(log::logger().get() && log::logger()->should_log(spdlog::level::trace)) \
            log::log_write(log::LOG_TRACE, __FILE__, __LINE__, _FUNC_NAME_, msg, ## __VA_ARGS__);
/**
 * LOG_IN logging macro for TRACE IN
 */
#define LOG_IN(msg, ...) \
      if(log::logger().get() && log::logger()->should_log(spdlog::level::trace)) \
            log::log_write(log::LOG_TRACE_IN, __FILE__, __LINE__, _FUNC_NAME_, msg, ## __VA_ARGS__);

/**
 * LOG_OUT logging macro for TRACE out with return;
 */
#define LOG_OUT(...) \
       if(log::logger().get() && log::logger()->should_log(spdlog::level::trace)) \
            log::log_write(log::LOG_TRACE_OUT, __FILE__, __LINE__, _FUNC_NAME_,  ## __VA_ARGS__); \
       return;
/**
 * LOG_RET logging macro for TRACE out with returning variable passed
 */
#define LOG_RET(msg, ptr) \
       if(log::logger().get() && log::logger()->should_log(spdlog::level::trace)) \
            log::log_write(log::LOG_TRACE_OUT, __FILE__, __LINE__, _FUNC_NAME_, msg,  ptr); \
       return(ptr);
/**
 * Send success response
 */
#define LOG_RET_TRUE(msg) \
       if(log::logger().get() && log::logger()->should_log(spdlog::level::trace)) \
            log::log_write(log::LOG_TRACE_OUT, __FILE__, __LINE__, _FUNC_NAME_, msg,  true); \
       return(true);
/**
 * Send Failed response
 */
#define LOG_RET_FALSE(msg) \
       if(log::logger().get() && log::logger()->should_log(spdlog::level::trace)) \
            log::log_write(log::LOG_TRACE_OUT, __FILE__, __LINE__, _FUNC_NAME_, msg,  false); \
       return(false);

/**
 * LOG_CUSTOM macro for custom logging
 */
#define LOG_CUSTOM(type, file, line, msg, ...) \
        if(log::logger().get() && log::logger()->should_log((spdlog::level::level_enum)type)) \
            log::log_write((log::logtype_t)type, __FILE__, __LINE__, _FUNC_NAME_, msg, ## __VA_ARGS__);

}//lightq


#endif	/* LOG_H */

