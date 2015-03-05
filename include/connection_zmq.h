/* 
 * File:   connection_zmq.h
 * Author: Rohit Joshi <rohit.c.joshi@gmail.com>
 *
 * Created on February 25, 2015, 11:04 AM
 */

#ifndef CONNECTION_ZMQ_H
#define	CONNECTION_ZMQ_H
#include <thread>
#include "thirdparty/zhelpers.hpp"
#include "thirdparty/log.hpp"

#include "connection.h"
#include "monitor_zmq.h"
#include "utils.h"

#define ZMQ_IOTHREADS 4

namespace prakashq {

    //zmq connection type

    class connection_zmq : public connection {
    public:

        enum connection_type {
            zmq_pair,
            zmq_pub,
            zmq_sub,
            zmq_req,
            zmq_rep,
            zmq_dealer,
            zmq_router,
            zmq_pull,
            zmq_push,
            zmq_xpub,
            zmq_xsub,
            zmq_stream
        };

        enum bind_type {
            zmq_bind,
            zmq_connect,
            zmq_default
        };
        //constuctor

        connection_zmq(const std::string& topic,
                const std::string& uri, endpoint_type conn_type) :

        connection(topic, uri, connection::stream_type::stream_zmq, conn_type) {
            LOG_IN("");
            fd_ = -1;
            p_socket_ = NULL;
            LOG_OUT("");
        }
        //destructor

        ~connection_zmq() {
            LOG_IN("");
            try {
                LOG_TRACE("Stopping monitoring...");
                monitor_.abort();

            } catch (std::exception& ex) {
                LOG_ERROR("Exception thrown: %s", ex.what());
            }
            
            if(monitor_thread_.joinable()) {
                LOG_TRACE("Join monitoring thread");
                monitor_thread_.join();
            }
            
            if (p_socket_) {
                LOG_TRACE("Delete p_socket");
                delete p_socket_;
                p_socket_ = NULL;
            }
            LOG_OUT("");
        }


        //init

        bool init(connection_type zmq_type, bool monitor_enabled = false, bind_type bindtype = bind_type::zmq_default) {
            LOG_IN("context:%p, zmq_connection_type:%d, zmq_bind_type:%d", &context, zmq_type, bindtype);
            connection_type_ = zmq_type;
            monitor_enabled_ = monitor_enabled;
            try {
                p_socket_ = new zmq::socket_t(context(), get_zmq_connect_type());



                if (bindtype == bind_type::zmq_default || bindtype == bind_type::zmq_bind) {
                    LOG_INFO("Binding to %s", resource_uri_.c_str());
                    if (monitor_enabled_) {
                        LOG_INFO("Start monitoring..");
                        monitor_uri_ = "inproc://prakashq_";
                        monitor_uri_.append(resource_uri_);
                        monitor_uri_.append("_");
                        monitor_uri_.append(std::to_string(fd_));
                        monitor_uri_.append("_");
                        monitor_uri_.append(std::to_string(connection_type_));
                        start_monitoring();
                    }
                    p_socket_->bind(resource_uri_.c_str());

                } else {
                    LOG_INFO("Connecting to %s", resource_uri_.c_str());
                    p_socket_->connect(resource_uri_.c_str());
                }
                size_t fdsize = sizeof (fd_);
                p_socket_->getsockopt(ZMQ_FD, &fd_, &fdsize);
                LOG_TRACE("ZMQ_FD value: %d", fd_);



            } catch (zmq::error_t &ex) {
                char buffer[256];
                sprintf(buffer, "Exception: %s, error number:%d", ex.what(), ex.num());
                LOG_RET_FALSE(buffer);
            }
            LOG_RET_TRUE("");
        }

        /**
         * write
         * @param message
         * @return 
         */
        ssize_t write(const std::string& message) {
            LOG_IN("message:%s", message.c_str());
            try {
                if (s_send(*p_socket_, message)) {
                    total_bytes_written_ += message.length();
                    total_msg_written_ += 1;
                    LOG_RET("Successfully send message", message.length());
                } else {
                    LOG_ERROR("Failed to send message", -1);
                }
            } catch (zmq::error_t &ex) {
                char buffer[256];
                sprintf(buffer, "Exception: %s, error number:%d", ex.what(), ex.num());
                LOG_RET(buffer, -1);
            }
        }

        /**
         * write
         * @param message
         * @param dealer_id
         * @return 
         */
        ssize_t write(const std::string& message, const std::string& dealer_id) {
            LOG_IN("message:%s, dealer_id:%s", message.c_str(), dealer_id.c_str());
            if (connection_type_ != ZMQ_ROUTER) {
                LOG_RET("Invalid method call. This must be only called for ZMQ_ROUTER connection", -1);
            }
            p_socket_->send(dealer_id.c_str(), dealer_id.length(), ZMQ_SNDMORE);
            LOG_RET("", write(message));
        }
        //read

        ssize_t read_msg(std::string& message) {
            LOG_IN("message: %s", message.c_str());
            try {

                message = s_recv(*p_socket_);
                //                zmq::message_t zmq_msg;             
                //                if(!p_socket_->recv(&zmq_msg, 0)) {
                //                    LOG_RET("Failed to read. try again", 0)
                //                }
                LOG_TRACE("Received: size %d", message.size());
                message.assign(static_cast<const char*> (message.data()), message.size());
                total_bytes_read_ += message.length();
                ++total_msg_read_;
                LOG_RET("", message.length());
            } catch (zmq::error_t &ex) {
                char buffer[256];
                sprintf(buffer, "Exception: %s, error number:%d", ex.what(), ex.num());
                LOG_RET(buffer, -1);
            }
        }

        /**
         * Read message
         * @param buffer
         * @param size
         * @return 
         */
        ssize_t read_msg(char* buffer, unsigned size) {
            LOG_IN("buffer: %p , size:%u", buffer, size);
            ssize_t nbytes = p_socket_->recv(buffer, size, 0);
            if (nbytes >= 0) {
                total_bytes_read_ += nbytes;
                ++total_msg_read_;
                LOG_RET("", nbytes);
            } else if (zmq_errno() == EAGAIN) {
                LOG_RET("try again", 0)
            }
            zmq::error_t error;
            LOG_ERROR("Failed to receive message. Error number :%d, Description :%s", error.num(), error.what());
            LOG_RET("Failed", -1);
        }



    private:

        int get_zmq_connect_type() {
            LOG_IN("");
            switch (connection_type_) {
                case connection_type::zmq_pub:
                {
                    LOG_RET("ZMQ_PUB", ZMQ_PUB);
                }
                case connection_type::zmq_sub:
                {
                    LOG_RET("ZMQ_SUB", ZMQ_SUB);
                }
                case connection_type::zmq_pull:
                {
                    LOG_RET("ZMQ_PULL", ZMQ_PULL);
                }
                case connection_type::zmq_push:
                {
                    LOG_RET("ZMQ_PUSH", ZMQ_PUSH);
                }
                case connection_type::zmq_router:
                {
                    LOG_RET("ZMQ_ROUTER", ZMQ_ROUTER);
                }
                case connection_type::zmq_dealer:
                {
                    LOG_RET("ZMQ_DEALER", ZMQ_DEALER);
                }
                case connection_type::zmq_req:
                {
                    LOG_RET("ZMQ_REQ", ZMQ_REQ);
                }
                case connection_type::zmq_rep:
                {
                    LOG_RET("ZMQ_REP", ZMQ_REP);
                }
                default:
                {
                    LOG_ERROR(utils::format_str("Unsupported connection type:%d", connection_type_).c_str());
                    LOG_RET("", -1);
                }
            }
        }

        /**
         * get zmq context
         * @return 
         */
        static zmq::context_t& context() {
            static zmq::context_t context(ZMQ_IOTHREADS);
            return context;
        }

        /**
         * Start monitoring
         */
        void start_monitoring() {
            LOG_IN("");
            monitor_thread_ = std::thread([&]() {
                LOG_IN("");
                try {

                    monitor_.monitor(*p_socket_, monitor_uri_.c_str());

                } catch (zmq::error_t &ex) {

                    LOG_ERROR("Exception: %s, error number:%d", ex.what(), ex.num());
                }
                LOG_EVENT("event_monitor_stopped");
                        LOG_OUT("");
            });

        }


        connection_type connection_type_;
        zmq::socket_t* p_socket_;

        uint64_t total_msg_written_;
        uint64_t total_msg_read_;
        uint64_t total_bytes_written_;
        uint64_t total_bytes_read_;
        bool monitor_enabled_;
        std::thread monitor_thread_;
        std::string monitor_uri_;
        int32_t fd_;
        monitor_zmq monitor_;

    };

}
#endif	/* CONNECTION_ZMQ_H */

