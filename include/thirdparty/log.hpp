/*
 * File:   log.hpp
 * Author: Rohit Joshi
 *
 * Created on Dec Jan 16, 2011, 12:32 AMa
 * Arawat Inc. Reserved @2011
 */

#ifndef _ARAWATLOG_H
#define	_ARAWATLOG_H

#include <string>
#include <boost/tuple/tuple.hpp>
#include <cstdarg>

#ifdef __GNUC__ //__GNUC__
#define _FUNC_NAME_ __PRETTY_FUNCTION__
#else
#define _FUNC_NAME_ _func_
#endif //__GNUC__
/*
 *Namespace arawat
 */
namespace prakashq {

/**
 * namespace util
 */

/**
 * #define HOST_BUFSIZE to 256
 */
#define HOST_BUFSIZE 256
/**
 * #define FORMAT_BUFFER_SIZE to 8192
 */
#define FORMAT_BUFFER_SIZE 8192



/**
 * result which provides bool return code and string result if required
 */
typedef boost::tuple<bool, std::string > result_t;

/**
 * Class log
 */
class log {
public:
	/**
	 * Logging Call back function.
	 */
	typedef void (* T_LOG_UTIL_LOG)(const unsigned int logtype,
			const char* file, unsigned int line, const char* func, const char *,
			...);

	/**
	 * LogLevel eum
	 */
	typedef enum {
		LOG_TRACE_IN,
		LOG_TRACE_OUT,
		LOG_TRACE,
		LOG_INFO,
		LOG_EVENT,
		LOG_WARNING,
		LOG_ERROR

	} logtype_t;

	/**
	 * Destructor
	 */
	virtual ~log() {
	}

	/**
	 * Get LogCallback func pointer
	 */
	inline static const T_LOG_UTIL_LOG get_func_ptr() {
		return log_func_ptr_;
	}

	/**
	 * Initilaize the pointer
	 */
	inline static void init(T_LOG_UTIL_LOG log_func_ptr) {
		log_func_ptr_ = log_func_ptr;
	}

	/**
	 * Set the log level
	 */
	static void log_level(log::logtype_t eType);

	/**
	 * Enable info
	 */
	static void enable_error();
	/**
	 * Enable info
	 */
	static void enable_warn();
	/**
	 * Enable event
	 */
	static void enable_event();
	static void disable_event();
	/**
	 * Enable info
	 */
	static void enable_info();
	static void disable_info();

	/**
	 * Enable trace
	 */
	static void enable_trace();
	static void disable_trace();

	/**
	 * IsInfoEnabled
	 */
	inline static bool is_info() {
		return is_log_level(log::LOG_INFO);
	}

	/**
	 * Is Trace enabled
	 */
	inline static bool is_trace() {
		return is_log_level(log::LOG_TRACE);
	}

	/**
	 * Is Log level enabled
	 */
	inline static bool is_log_level(log::logtype_t eType) {
		return ((1L << eType) & log_levels_);
	}


	/**
	 * Format argument list
	 */
	static std::string format_arg_list(const char *pszFormat, va_list args);
        
        /**
         * log write function pointer: default
         * @param logtype
         * @param file
         * @param line
         * @param func
         * @param buf
         * @param ...
         */
        static void log_write(const unsigned int logtype, const char* file, unsigned int line,
            const char* func, const char *buf, ...);


private:
	/**
	 * Log Call Back function variable
	 */
	static log::T_LOG_UTIL_LOG log_func_ptr_;
	/**
	 * LogLevel member variable
	 */
	static unsigned log_levels_;

	/**
	 * Constructor
	 */
	log() {
	}

	/**
	 * Copy Constructor
	 * @param orig log
	 */
	log(const log& orig) {
	}
};
/**
 * LOG_EVENT logging macro
 */
#define LOG_EVENT(msg, ...) \
        if(log::get_func_ptr()  && log::is_log_level(log::LOG_EVENT)) \
        log::get_func_ptr()(log::LOG_EVENT, __FILE__, __LINE__, _FUNC_NAME_, msg, ## __VA_ARGS__);

/**
 * LOG_INFO logging macro
 */
#define LOG_INFO(msg, ...) \
        if(log::get_func_ptr() && log::is_log_level(log::LOG_INFO)) \
        log::get_func_ptr()(log::LOG_INFO, __FILE__, __LINE__, _FUNC_NAME_, msg, ## __VA_ARGS__);
/**
 * LOG_ERROR logging macro
 */
#define LOG_ERROR(msg, ...) \
        if(log::get_func_ptr() && log::is_log_level(log::LOG_ERROR)) log::get_func_ptr()(log::LOG_ERROR, __FILE__, __LINE__,  _FUNC_NAME_, msg,  ## __VA_ARGS__);
/**
 * LOG_WARN logging macro
 */
#define LOG_WARN(msg, ...) \
       if(log::get_func_ptr() && log::is_log_level(log::LOG_WARNING)) log::get_func_ptr()(log::LOG_WARNING, __FILE__, __LINE__, _FUNC_NAME_, msg,  ## __VA_ARGS__);
/**
 * LOG_TRACE logging macro
 */
#define LOG_TRACE(msg, ...) \
       if(log::get_func_ptr() && log::is_log_level(log::LOG_TRACE)) log::get_func_ptr()(log::LOG_TRACE, __FILE__, __LINE__, _FUNC_NAME_, msg,  ## __VA_ARGS__);
/**
 * LOG_IN logging macro for TRACE IN
 */
#define LOG_IN(msg, ...) \
       if(log::get_func_ptr() && log::is_log_level(log::LOG_TRACE_IN)) log::get_func_ptr()(log::LOG_TRACE_IN, __FILE__, __LINE__, _FUNC_NAME_, msg,  ## __VA_ARGS__);

/**
 * LOG_OUT logging macro for TRACE out with return;
 */
#define LOG_OUT(...) \
       if(log::get_func_ptr() && log::is_log_level(log::LOG_TRACE)) log::get_func_ptr()(log::LOG_TRACE_OUT, __FILE__, __LINE__, _FUNC_NAME_, "",  ## __VA_ARGS__); \
       return;
/**
 * LOG_RET logging macro for TRACE out with returning variable passed
 */
#define LOG_RET(msg, ptr) \
       if(log::get_func_ptr() && log::is_log_level(log::LOG_TRACE)) log::get_func_ptr()(log::LOG_TRACE_OUT, __FILE__, __LINE__, _FUNC_NAME_, msg,  ptr); \
       return(ptr);
/**
 * Send success response
 */
#define LOG_RET_TRUE(msg) \
       if(log::get_func_ptr() && log::is_log_level(log::LOG_TRACE)) log::get_func_ptr()(log::LOG_TRACE_OUT, __FILE__, __LINE__, _FUNC_NAME_, msg,  true); \
       return(true);
/**
 * Send Failed response
 */
#define LOG_RET_FALSE(msg) \
       if(log::get_func_ptr() && log::is_log_level(log::LOG_TRACE)) log::get_func_ptr()(log::LOG_TRACE_OUT, __FILE__, __LINE__, _FUNC_NAME_, msg,  false); \
       return(false);
/**
 * Return tuple
 */
#define LOG_RET_RESULT(result) \
	       if(log::get_func_ptr() && log::is_log_level(log::LOG_TRACE)) log::get_func_ptr()(log::LOG_TRACE_OUT, __FILE__, __LINE__, _FUNC_NAME_, result.get<1>().c_str(),  result.get<0>()); \
	       return(result);
/**
 * LOG_CUSTOM macro for custom logging
 */
#define LOG_CUSTOM(type, file, line, msg, ...) \
        if(log::get_func_ptr() && log::is_log_level(type)) log::get_func_ptr()((log::logtype_t)type, __FILE__, __LINE__, _FUNC_NAME_, msg, ## __VA_ARGS__);

}//arawat
#endif	/* _LOG_LOG_H */

