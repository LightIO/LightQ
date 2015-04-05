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
#include "log.h"

#include "connection.h"
#include "monitor_zmq.h"
#include "utils.h"

#define ZMQ_IOTHREADS 4

namespace lightq {

    //zmq connection type

    class connection_zmq : public connection {
    public:

        enum zmq_socket_type {
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


        //constuctor
        connection_zmq(const std::string& topic,
                const std::string& uri,
                endpoint_type ep_type,
                zmq_socket_type zmq_type,
                connection::socket_connect_type socket_connect = connection::bind_socket,
                bool non_blocking = true,
                bool monitor_enabled = false) :
        connection(topic, uri,
        connection::stream_type::stream_zmq,
        ep_type,
        socket_connect,
        non_blocking) {
            LOG_IN("zmq_type[%d]", zmq_type);
            zmq_socket_type_ = zmq_type;
            fd_ = -1;
            p_socket_ = NULL;
            monitor_enabled_ = monitor_enabled;
            LOG_OUT("");
        }
        //destructor
        ~connection_zmq() {
            LOG_IN("");
            try {
                LOG_DEBUG("Stopping monitoring...");
                monitor_.abort();

            } catch (std::exception& ex) {
                LOG_ERROR("Exception thrown: %s", ex.what());
            }

            if (monitor_thread_.joinable()) {
                LOG_DEBUG("Join monitoring thread");
                monitor_thread_.join();
            }

            
            LOG_DEBUG("Delete p_socket");
            delete p_socket_;
           
            LOG_OUT("");
        }

        /**
         * init
         * @param zmq_type
         * @param monitor_enabled
         * @return 
         */
        bool init() {
            LOG_IN("");

            try {
                p_socket_ = new zmq::socket_t(context(), get_zmq_connect_type());
                
                if (socket_connect_type_ == connection::bind_socket) {
                    LOG_INFO("Binding to %s", resource_uri_.c_str());
                    if (monitor_enabled_) {
                        LOG_INFO("Start monitoring..");
                        monitor_uri_ = "inproc://lightq_";
                        monitor_uri_.append(resource_uri_);
                        monitor_uri_.append("_");
                        monitor_uri_.append(std::to_string(fd_));
                        monitor_uri_.append("_");
                        monitor_uri_.append(std::to_string(zmq_socket_type_));
                        start_monitoring();
                    }
                    p_socket_->bind(resource_uri_.c_str());

                } else {
                    LOG_INFO("Connecting to %s", resource_uri_.c_str());
                    if(get_zmq_connect_type() == ZMQ_SUB) {
                        p_socket_->setsockopt(ZMQ_SUBSCRIBE, topic_.c_str(), topic_.length());
                        LOG_TRACE("Subscribing to topic[%s]", topic_.c_str());
                     }
                    p_socket_->connect(resource_uri_.c_str());
                }
                size_t fdsize = sizeof (fd_);
                p_socket_->getsockopt(ZMQ_FD, &fd_, &fdsize);
                LOG_DEBUG("ZMQ_FD value: %d", fd_);

            } catch (zmq::error_t &ex) {
                char buffer[utils::max_small_msg_size];
                sprintf(buffer, "Exception: %s, error number:%d", ex.what(), ex.num());
                LOG_RET_FALSE(buffer);
            }
            LOG_RET_TRUE("initialized");
        }
        
        /**
         * 
         * @return 
         */
        bool run() {
            LOG_IN("")
            LOG_RET_TRUE("dummy");
        }

        /**
         * write
         * @param message
         * @return 
         */
        ssize_t write_msg(const std::string& message) {
            LOG_IN("message:%s", message.c_str());
            try {
                 if(get_zmq_connect_type() == ZMQ_PUB) {
                   s_sendmore(*p_socket_, topic_, true);
                 }
                
                if (s_send(*p_socket_, message, true)) {
                    total_bytes_written_ += message.length();
                    total_msg_written_ += 1;
                    LOG_RET("Successfully send message", message.length());
                } else {
                    LOG_ERROR("Failed to send message", -1);
                }
            } catch (zmq::error_t &ex) {
                char buffer[utils::max_small_msg_size];
                sprintf(buffer, "Exception: %s, error number:%d", ex.what(), ex.num());
                LOG_RET(buffer, -1);
            }
            LOG_RET("failed", -1);
        }
        
        
        /**
         * write
         * @param message
         * @return 
         */
        ssize_t write_msg(const char* message, unsigned length) {
            LOG_IN("message:%p", message);
            try {
                 if(get_zmq_connect_type() == ZMQ_PUB) {
                   s_sendmore(*p_socket_, topic_, true);
                 }
                
                if (s_send(*p_socket_, message, length, true)) {
                    total_bytes_written_ += length;
                    total_msg_written_ += 1;
                    LOG_RET("Successfully send message", length);
                } else {
                    LOG_ERROR("Failed to send message", -1);
                }
            } catch (zmq::error_t &ex) {
                char buffer[utils::max_small_msg_size];
                sprintf(buffer, "Exception: %s, error number:%d", ex.what(), ex.num());
                LOG_RET(buffer, -1);
            }
            LOG_RET("failed", -1);
        }

        /**
         * write
         * @param message
         * @param dealer_id
         * @return 
         */
        ssize_t write(const std::string& message, const std::string& dealer_id) {
            LOG_IN("message:%s, dealer_id:%s", message.c_str(), dealer_id.c_str());
            if (zmq_socket_type_ != ZMQ_ROUTER) {
                LOG_RET("Invalid method call. This must be only called for ZMQ_ROUTER connection", -1);
            }
            p_socket_->send(dealer_id.c_str(), dealer_id.length(), ZMQ_SNDMORE);
            LOG_RET("", write_msg(message));
        }

        /**
         * read message
         * @param message
         * @return 
         */
        ssize_t read_msg(std::string& message) {
            message.clear();
            LOG_IN("message: %s", message.c_str());
            try {
                
                message = s_recv(*p_socket_);
                //                zmq::message_t zmq_msg;             
                //                if(!p_socket_->recv(&zmq_msg, 0)) {
                //                    LOG_RET("Failed to read. try again", 0)
                //                }
                LOG_DEBUG("Received: size %d", message.size());
                message.assign(static_cast<const char*> (message.data()), message.size());
                LOG_TRACE("Received %s", message.c_str());
                total_bytes_read_ += message.length();
                ++total_msg_read_;
                LOG_RET("", message.length());
            } catch (zmq::error_t &ex) {
                char buffer[utils::max_small_msg_size];
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

        uint32_t get_num_connected_clients() {
            return monitor_.num_clients_;
        }
        
        uint64_t get_total_bytes_written() {
            return total_bytes_written_;
        }
        
       

    private:

        /**
         * get zmw connect type
         * @return 
         */
        int get_zmq_connect_type() {
            LOG_IN("");
            switch (zmq_socket_type_) {
                case zmq_socket_type::zmq_pub:
                {
                    LOG_RET("ZMQ_PUB", ZMQ_PUB);
                }
                case zmq_socket_type::zmq_sub:
                {
                    LOG_RET("ZMQ_SUB", ZMQ_SUB);
                }
                case zmq_socket_type::zmq_pull:
                {
                    LOG_RET("ZMQ_PULL", ZMQ_PULL);
                }
                case zmq_socket_type::zmq_push:
                {
                    LOG_RET("ZMQ_PUSH", ZMQ_PUSH);
                }
                case zmq_socket_type::zmq_router:
                {
                    LOG_RET("ZMQ_ROUTER", ZMQ_ROUTER);
                }
                case zmq_socket_type::zmq_dealer:
                {
                    LOG_RET("ZMQ_DEALER", ZMQ_DEALER);
                }
                case zmq_socket_type::zmq_req:
                {
                    LOG_RET("ZMQ_REQ", ZMQ_REQ);
                }
                case zmq_socket_type::zmq_rep:
                {
                    LOG_RET("ZMQ_REP", ZMQ_REP);
                }
                default:
                {
                    LOG_ERROR(utils::format_str("Unsupported connection type:%d", zmq_socket_type_).c_str());
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
            std::thread th = std::thread([&]() {
                LOG_IN("");
                try {

                    monitor_.monitor(*p_socket_, monitor_uri_.c_str());

                } catch (zmq::error_t &ex) {

                    LOG_ERROR("zmq::error_t : %s, error number:%d", ex.what(), ex.num());
                } catch (std::exception ex) {

                    LOG_ERROR("Exception: %s", ex.what());
                }
                LOG_EVENT("event_monitor_stopped");
               
            });
            LOG_TRACE("Swapping thread");
            LOG_DEBUG("monitor_thread_ id[%s], th thread id[%s]",
                    utils::thread_id_to_str(monitor_thread_.get_id()).c_str(),
                    utils::thread_id_to_str(th.get_id()).c_str());
            monitor_thread_.swap(th);
           
           // std::swap(monitor_thread_, th);
            LOG_DEBUG("After swap: monitor_thread_ id[%s], th thread id[%s]",
                    utils::thread_id_to_str(monitor_thread_.get_id()).c_str(),
                    utils::thread_id_to_str(th.get_id()).c_str());
            
            if(th.get_id() == std::thread::id()) {
                LOG_DEBUG("Swap is successfull");
            }else {
                th.detach();
            }
//            if (th.joinable()) {
//                LOG_DEBUG("Join monitoring thread");
//                th.join();
//            }
            LOG_OUT("");

        }


        zmq_socket_type zmq_socket_type_;
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

