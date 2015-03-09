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
        std::string address_;


    };

    //monitor_zmq

    class monitor_zmq : public zmq::monitor_t {
        friend class connection_zmq;
    private:

        monitor_zmq() : zmq::monitor_t(), num_clients_(0) {

        }
        
        virtual ~monitor_zmq() {
            LOG_IN("");
            LOG_OUT("");
        }

        virtual void on_monitor_started() {
            LOG_IN("");
            LOG_EVENT("event_monitor_started")
            LOG_OUT("");
        }

        virtual void on_event_connected(const zmq_event_t &event, const char* addr) {
            LOG_IN("event.event[%u],event.value[%u], addr[%s] ", event.event, event.value, addr);
            LOG_EVENT("event_connected: event.event[%u],event.value[%u], addr[%s] ", event.event, event.value, addr);
            ++num_clients_;
            //monitor_event me(event, addr);

            // events_.enqueue(me);
            LOG_OUT("");
        }

        virtual void on_event_connect_delayed(const zmq_event_t &event, const char* addr) {
            LOG_IN("event.event[%u],event.value[%u], addr[%s] ", event.event, event.value, addr);

            LOG_EVENT("connect_delayed: event.event[%u],event.value[%u], addr[%s] ", event.event, event.value, addr);
            //monitor_event me(event, addr);
            //events_.enqueue(me); 
            LOG_OUT("");
        }

        virtual void on_event_connect_retried(const zmq_event_t &event, const char* addr) {
            LOG_IN("event.event[%u],event.value[%u], addr[%s] ", event.event, event.value, addr);
            LOG_EVENT("connect_retried: event.event[%u],event.value[%u], addr[%s] ", event.event, event.value, addr);
            //monitor_event me(event, addr);
            //  events_.enqueue(me); 
            LOG_OUT("");
        }

        virtual void on_event_listening(const zmq_event_t &event, const char* addr) {
            LOG_IN("event.event[%u],event.value[%u], addr[%s] ", event.event, event.value, addr);
            LOG_EVENT("event_listening: event.event[%u],event.value[%u], addr[%s] ", event.event, event.value, addr);
            //monitor_event me(event, addr);
            //events_.enqueue(me); 
            LOG_OUT("");
        }

        virtual void on_event_bind_failed(const zmq_event_t &event, const char* addr) {
            LOG_IN("event.event[%u],event.value[%u], addr[%s] ", event.event, event.value, addr);
            LOG_EVENT("event_bind_failed: event.event[%u],event.value[%u], addr[%s] ", event.event, event.value, addr);
            //monitor_event me(event, addr);
            //events_.enqueue(me); 
            LOG_OUT("");
        }

        virtual void on_event_accepted(const zmq_event_t &event, const char* addr) {
            LOG_IN("event.event[%u],event.value[%u], addr[%s] ", event.event, event.value, addr);
            LOG_EVENT("event_accepted: event.event[%u],event.value[%u], addr[%s] ", event.event, event.value, addr);
            // monitor_event me(event, addr);
            // events_.enqueue(me); 
            ++num_clients_;
            LOG_TRACE("Total number of clients connected: %d", num_clients_);
            LOG_OUT("");
        }

        virtual void on_event_accept_failed(const zmq_event_t &event, const char* addr) {
            LOG_IN("event.event[%u],event.value[%u], addr[%s] ", event.event, event.value, addr);
            LOG_EVENT("event_accept_failed: event.event[%u],event.value[%u], addr[%s] ", event.event, event.value, addr);
            //monitor_event me(event, addr);
            //events_.enqueue(me); 
            LOG_OUT("");
        }

        virtual void on_event_closed(const zmq_event_t &event, const char* addr) {
            LOG_IN("event.event[%u],event.value[%u], addr[%s] ", event.event, event.value, addr);
            LOG_EVENT("event_closed: event.event[%u],event.value[%u], addr[%s] ", event.event, event.value, addr);
            // monitor_event me(event, addr);
            //events_.enqueue(me); 
            LOG_OUT("");
        }

        virtual void on_event_close_failed(const zmq_event_t &event, const char* addr) {
            LOG_IN("event.event[%u],event.value[%u], addr[%s] ", event.event, event.value, addr);
            LOG_EVENT("event_close_failed: event.event[%u],event.value[%u], addr[%s] ", event.event, event.value, addr);
            //monitor_event me(event, addr);
            //events_.enqueue(me); 
            LOG_OUT("");
        }

        virtual void on_event_disconnected(const zmq_event_t &event, const char* addr) {
            LOG_IN("event.event[%u],event.value[%u], addr[%s] ", event.event, event.value, addr);
            --num_clients_;
            LOG_EVENT("event_disconnected: event.event[%u],event.value[%u], addr[%s] ", event.event, event.value, addr);
            //monitor_event me(event, addr);
            //events_.enqueue(me); 
            LOG_TRACE("Total number of clients connected: %d", num_clients_);
            LOG_OUT("");
        }

        virtual void on_event_unknown(const zmq_event_t &event, const char* addr) {
            LOG_IN("event.event[%u],event.value[%u], addr[%s] ", event.event, event.value, addr);
            LOG_EVENT("event_unknown: event.event[%u],event.value[%u], addr[%s] ", event.event, event.value, addr);
            //monitor_event me(event, addr);
            //events_.enqueue(me); 
            LOG_OUT("");
        }
       // moodycamel::ReaderWriterQueue<monitor_event> events_;
        int32_t num_clients_;
    };

}


#endif	/* MONITOR_ZMQ_H */

