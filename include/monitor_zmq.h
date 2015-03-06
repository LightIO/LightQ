/* 
 * File:   monitor_zmq.h
 * Author: Rohit Joshi <rohit.c.joshi@gmail.com>
 *
 * Created on February 26, 2015, 12:27 PM
 */

#ifndef MONITOR_ZMQ_H
#define	MONITOR_ZMQ_H
#include <sstream>
#include "thirdparty/readerwriterqueue.h"
#include "thirdparty/zmq.hpp"
#include "log.h"

namespace lightq {
    //monitor event
    //monitor event
    class monitor_event {
    public:
        /**
         * constructor
         * @param zmq_event
         * @param address
         */
        explicit monitor_event(const zmq_event_t& zmq_event, const std::string& address) {
            event_info_ = zmq_event;
            address_ = address;
        }
        /**
         * to string
         * @return 
         */
         std::string to_string() {
            std::ostringstream ss;
            ss << "address: " << address_;
            ss << ", event: " << event_info_.event << ",  value: " << event_info_.value;
            return ss.str();
        }
    private:
        zmq_event_t event_info_;
        std::string  address_;
        
       
    };
    
    //monitor_zmq
    class monitor_zmq : public zmq::monitor_t {
        friend class connection_zmq;
    private:
        monitor_zmq():zmq::monitor_t(), events_(1024) {
            
        }
        virtual void on_monitor_started() {
            LOG_EVENT("event_monitor_started")
        }
        virtual void on_event_connected(const zmq_event_t &event, const char* addr) {
             monitor_event me(event, addr);
             LOG_EVENT("event_connected: %s", me.to_string().c_str());
             events_.enqueue(me);
        }
        virtual void on_event_connect_delayed(const zmq_event_t &event, const char* addr) {
             monitor_event me(event, addr);
             LOG_EVENT("connect_delayed: %s", me.to_string().c_str());
             events_.enqueue(me); 
        }
        virtual void on_event_connect_retried(const zmq_event_t &event, const char* addr) { 
              monitor_event me(event, addr);
             LOG_EVENT("connect_retried: %s", me.to_string().c_str());
             events_.enqueue(me); 
        }
        virtual void on_event_listening(const zmq_event_t &event, const char* addr) { 
               monitor_event me(event, addr);
             LOG_EVENT("event_listening: %s", me.to_string().c_str());
             events_.enqueue(me); 
        }
        virtual void on_event_bind_failed(const zmq_event_t &event, const char* addr) {
               monitor_event me(event, addr);
             LOG_EVENT("event_bind_failed: %s", me.to_string().c_str());
             events_.enqueue(me); 
        }
        virtual void on_event_accepted(const zmq_event_t &event, const char* addr) {
               monitor_event me(event, addr);
             LOG_EVENT("event_accepted: %s", me.to_string().c_str());
             events_.enqueue(me); 
        }
        virtual void on_event_accept_failed(const zmq_event_t &event, const char* addr) {
              monitor_event me(event, addr);
             LOG_EVENT("event_accept_failed: %s", me.to_string().c_str());
             events_.enqueue(me); 
        }
        virtual void on_event_closed(const zmq_event_t &event, const char* addr) { 
               monitor_event me(event, addr);
             LOG_EVENT("event_closed: %s", me.to_string().c_str());
             events_.enqueue(me);  
        }
        virtual void on_event_close_failed(const zmq_event_t &event, const char* addr) { 
               monitor_event me(event, addr);
             LOG_EVENT("event_close_failed: %s", me.to_string().c_str());
             events_.enqueue(me); 
        }
        virtual void on_event_disconnected(const zmq_event_t &event, const char* addr) {
              monitor_event me(event, addr);
             LOG_EVENT("event_disconnected: %s", me.to_string().c_str());
             events_.enqueue(me); 
        }
        virtual void on_event_unknown(const zmq_event_t &event, const char* addr) {
              monitor_event me(event, addr);
             LOG_EVENT("event_unknown: %s", me.to_string().c_str());
             events_.enqueue(me); 
        }
        moodycamel::ReaderWriterQueue<monitor_event> events_;
    };
    
}


#endif	/* MONITOR_ZMQ_H */

