/*
 * log.cpp
 *
 *  Created on: Jan 16, 2011
 *      Author: Rohit Joshi
 *  Arawat Inc Reserved 2011
 */

#include <cstdarg>
#include <cstdio>
#include <iostream>
#include <cstring>
#include <boost/format.hpp>
#include "log.hpp"



#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT __FILE__ ":" TOSTRING(__LINE__)
#define TID pthread_self()
#define PID getpid()

#define stringify( name ) # name
/*
 *Namespace arawat
 */
namespace prakashq {



    log::T_LOG_UTIL_LOG log::log_func_ptr_ = NULL;

    static const unsigned LF_EVENT = 1L << log::LOG_EVENT;
    static const unsigned LF_INFO = 1L << log::LOG_INFO;
    static const unsigned LF_WARNING = 1L << log::LOG_WARNING;
    static const unsigned LF_ERROR = 1L << log::LOG_ERROR;
    static const unsigned LF_TRACE = 1L << log::LOG_TRACE;
    static const unsigned LF_TRACE_IN = 1L << log::LOG_TRACE_IN;
    static const unsigned LF_TRACE_OUT = 1L << log::LOG_TRACE_OUT;
    static const unsigned LF_ALL = LF_INFO | LF_WARNING | LF_ERROR | LF_TRACE
            | LF_TRACE_IN | LF_TRACE_OUT;
    static const unsigned LF_DEFAULT = LF_WARNING | LF_ERROR;

    unsigned log::log_levels_ = LF_DEFAULT;

    /**
     * format log
     * @param pszFormat
     * @param args
     * @return 
     */
    std::string log::format_arg_list(const char *pszFormat, va_list args) {
        if (!pszFormat) return "";

        char szBuf[FORMAT_BUFFER_SIZE];
        int nRet = vsnprintf(szBuf, FORMAT_BUFFER_SIZE, pszFormat, args);
        if (nRet >= (int) FORMAT_BUFFER_SIZE) {
            // redo the formatting
            char *buffer = (char*) malloc((nRet + 1) * sizeof (char));
            vsnprintf(buffer, nRet, pszFormat, args);
            std::string sMsg(buffer, nRet);
            free(buffer);
            return sMsg;
        } else {
            std::string sMsg(szBuf, nRet);
            return sMsg;
        }
    }

    /**
     * Set the log level
     */
    void log::log_level(log::logtype_t eType) {
        log_levels_ |= eType;
    }

    /**
     * Enable info
     */
    void log::enable_error() {
        log_levels_ |= LF_ERROR;
        log_levels_ &= (~LF_WARNING);
        log_levels_ &= (~LF_EVENT);
        log_levels_ &= (~LF_INFO);
        log_levels_ &= (~LF_TRACE);
        log_levels_ &= (~LF_TRACE_IN);
        log_levels_ &= (~LF_TRACE_OUT);
    }

    /**
     * Enable info
     */
    void log::enable_warn() {
        log_levels_ |= LF_ERROR;
        log_levels_ |= LF_WARNING;
        log_levels_ &= (~LF_EVENT);
        log_levels_ &= (~LF_INFO);
        log_levels_ &= (~LF_TRACE);
        log_levels_ &= (~LF_TRACE_IN);
        log_levels_ &= (~LF_TRACE_OUT);

    }

    /**
     * Enable event
     */
    void log::enable_event() {
        log_levels_ |= LF_ERROR;
        log_levels_ |= LF_WARNING;
        log_levels_ |= LF_EVENT;
        log_levels_ &= (~LF_INFO);
        log_levels_ &= (~LF_TRACE);
        log_levels_ &= (~LF_TRACE_IN);
        log_levels_ &= (~LF_TRACE_OUT);
    }

    void log::disable_event() {
        log_levels_ &= (~LF_EVENT);
    }

    /**
     * Enable info
     */
    void log::enable_info() {
        log_levels_ |= LF_ERROR;
        log_levels_ |= LF_WARNING;
        log_levels_ |= LF_EVENT;
        log_levels_ |= LF_INFO;
        log_levels_ &= (~LF_TRACE);
        log_levels_ &= (~LF_TRACE_IN);
        log_levels_ &= (~LF_TRACE_OUT);
    }

    void log::disable_info() {
        log_levels_ &= (~LF_INFO);
    }

    /**
     * Enable trace
     */
    void log::enable_trace() {
        log_levels_ |= LF_ERROR;
        log_levels_ |= LF_WARNING;
        log_levels_ |= LF_EVENT;
        log_levels_ |= LF_INFO;
        log_levels_ |= LF_TRACE;
        log_levels_ |= LF_TRACE_IN;
        log_levels_ |= LF_TRACE_OUT;
    }

    void log::disable_trace() {
        log_levels_ &= (~LF_TRACE);
        log_levels_ &= (~LF_TRACE_IN);
        log_levels_ &= (~LF_TRACE_OUT);
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
    void log::log_write(const unsigned int logtype, const char* file, unsigned int line,
            const char* func, const char *buf, ...) {
        std::string msg;
        va_list args;
        va_start(args, buf);
        msg = prakashq::log::format_arg_list(buf, args);
        va_end(args);
        std::string func_inout = func;
        func_inout += "|";

        if (logtype == log::LOG_TRACE_IN)
            func_inout += "-->IN|";
        else if (logtype == log::LOG_TRACE_OUT)
            func_inout += "<--OUT|";


        const char* filename = file;
        const char* p = strrchr(file, '/');
        if (p) {
            filename = ++p;
        }

#ifdef __APPLE__
        uint64_t tid;
        pthread_threadid_np(NULL, &tid);
        uint32_t pid = PID;

        std::cout << boost::format("%5d|%8X|") % PID % (tid % 10000000000)
                << filename << ":" << std::dec << line << "|" << func_inout << "|" << msg << "\n" << std::flush;
#else
        std::cout << boost::format("%5d|%8X|") << % PID % (TID % 10000000000)
                << filename << ":" << std::dec << line << "|" << msg;
#endif
    }

}//arawat
