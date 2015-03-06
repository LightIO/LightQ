/* 
 * File:   consumer.h
 * Author: Rohit Joshi <rohit.c.joshi@gmail.com>
 *
 * Created on February 27, 2015, 10:29 PM
 */

#ifndef CONSUMER_H
#define	CONSUMER_H
#include "broker_config.h"
#include "connection.h"
#include "broker_storage.h"
#include "connection_socket.h"

namespace lightq {
    class consumer {
    public:
        /**
         * constructor
         */
        consumer(broker_storage *pstorage, broker_config& config) : p_storage_(pstorage), config_(config),
                consumer_endpoint_type_(connection::endpoint_type::conn_consumer) {
            LOG_IN("broker_storage: %p, config: %s, connection::endpoint_type::conn_consumer",
                    p_storage_, config_.to_string().c_str());
            stop_ = false;  
            p_consumer_socket = NULL;
            LOG_OUT("");
        }
       
        /**
         * destructor
         */
        ~consumer() {
            LOG_IN("");
            if (consumer_tid_.joinable())
                consumer_tid_.join();
            if (p_consumer_socket) {
                delete p_consumer_socket;
                p_consumer_socket = NULL;
            }
            LOG_OUT("");
        }
        /**
         * init
         * @return 
         */
        bool init() {
            LOG_IN("");
            if (config_.consumer_stream_type_ == connection::stream_zmq) {
                p_consumer_socket = new connection_zmq(config_.id_, config_.consumer_bind_uri_, 
                        consumer_endpoint_type_, 
                        connection_zmq::zmq_push,
                        connection::bind_socket,
                        true,
                        false);
                
                 if (!p_consumer_socket->init()) {

                    LOG_RET_FALSE(utils::format_str("Failed to initialize broker: %s, consumer_bind_uri: %s",
                            config_.id_.c_str(), config_.consumer_bind_uri_.c_str()).c_str());

                }
            }else if(config_.consumer_stream_type_ == connection::stream_socket) {
                
                p_consumer_socket = new connection_socket(config_.id_, 
                        config_.consumer_bind_uri_, 
                        consumer_endpoint_type_,
                        connection::bind_socket,
                        true);
               
                if (!p_consumer_socket->init()) {

                    LOG_RET_FALSE(utils::format_str("Failed to initialize broker: %s, consumer_bind_uri: %s",
                            config_.id_.c_str(), config_.consumer_bind_uri_.c_str()).c_str());

                }else {
                    if(!p_consumer_socket->run()) {
                         LOG_RET_FALSE(utils::format_str("Failed to run consumer broker: %s, consumer_bind_uri: %s",
                            config_.id_.c_str(), config_.consumer_bind_uri_.c_str()).c_str());
                    }
                }
            }
            else {
                throw std::runtime_error("broker::init():Not implemented");
            }
            LOG_RET_TRUE("");
        }
        
        static  int process_fds(int fd) {
            LOG_IN("fd: %d", fd);
            LOG_RET_TRUE("");
        }
        
        /**
         * run
         * @return 
         */
        bool run() {
            LOG_IN("");
            consumer_tid_ = std::thread([&]() {
                    process_consumers();

                });
            LOG_RET_TRUE(""); 
        }
        
         /**
         * Process consumers
         */
        void process_consumers() {
            LOG_IN("");
            std::string message;
            message.reserve(config_.max_message_size);
            while (!stop_) {
                //if no messages to consume, sleep
                while(!p_storage_->get_total_msg_received() > 0) {
                    s_sleep(5);
                }
                bool result = false;
                bool use_send_file = false;
                if (p_consumer_socket->get_stream_type() == connection::stream_type::stream_socket) {
                    use_send_file = true;
                } else if (p_consumer_socket->get_stream_type() == connection::stream_type::stream_zmq) {
                    use_send_file = false;
                } else {
                    throw std::runtime_error("broker::process_consumers():Not implemented");
                }
                if(p_storage_->get_total_msg_received() > 0) {
                    if (config_.broker_type_ == broker_config::broker_file) {
                    
                        result = p_storage_->file_to_consumer(use_send_file, p_consumer_socket);
                    
                    } else if (config_.broker_type_ == broker_config::broker_queue ) {
                        result = p_storage_->queue_to_consumer(p_consumer_socket);
                    }
                }
                //fast consumer?
                if (result == 0) {
                    s_sleep(1);
                }
            }
            LOG_OUT("");
        }
        
        /**
         * get_consumer_socket
         * @return 
         */
        connection* get_consumer_socket() {
            return p_consumer_socket;
        }
    private:
        broker_storage *p_storage_;
        broker_config config_;
        connection::endpoint_type consumer_endpoint_type_;
        connection *p_consumer_socket;
        bool stop_;
        std::thread consumer_tid_;
       
    };
}


#endif	/* CONSUMER_H */

