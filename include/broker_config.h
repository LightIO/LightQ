/* 
 * File:   broker_config.h
 * Author: Rohit Joshi <rohit.c.joshi@gmail.com>
 *
 * Created on February 28, 2015, 10:19 AM
 */

#ifndef BROKER_CONFIG_H
#define	BROKER_CONFIG_H

#include "utils.h"
#include "connection.h"
//#include "broker.h"


namespace lightq {

    class connection;
    //class broker_config
    struct broker_config {
         //broker type
        enum broker_type {
            broker_queue,
            broker_file,
            broker_direct,
            broker_queue_file
        };
        
        std::string id_;
        broker_type broker_type_;
        std::string user_id_;
         std::string password_;
       // producer_config producer_config_;
       // consumer_config consumer_config_;
        uint32_t default_queue_size_ = 1024*1024*5;
        uint32_t max_message_size = 128 * 1048; // make it configurable
        std::string output_directory_ = "/tmp";
        std::string bind_interface = "tcp://*";
        
        
        /**
         * to string
         * @return 
         */
        std::string to_string() {
           /*
            std::string str = utils::format_str( "id: %s, broker_type: %d, producer_bind_uri: %s, "
                "producer_stream_type: %d, consumer_bind_uri: %s, consumer_stream_type: %d, "
                "default_queue_size_: %u, max_message_size: %u, output_directory: %s", 
                    id_.c_str(), broker_type_, producer_config_.producer_bind_uri_.c_str(), producer_config_.producer_stream_type_, 
                    consumer_config_.push_bind_uri_.c_str(), consumer_config_.stream_type_ , default_queue_size_, max_message_size,
                    output_directory_.c_str());
            
           // std::string str (buffer, strlen(buffer));
            LOG_DEBUG("config.to_string(): %s", str.c_str());
            return std::move(str);*/
            return std::string("uncommment");
            
        }
        
        std::string get_broker_type_to_str() {
            switch(broker_type_) {
                case broker_type::broker_direct : return "direct";
                case broker_type::broker_file : return "file";
                case broker_type::broker_queue_file : return "queue_file";
                default:
                   case broker_type::broker_queue : return "queue"; 
            }
            return "queue";
        }
        /**
         * get next port
         * @param initial_port
         * @return 
         */
        inline static unsigned get_next_port(unsigned initial_port=5000) {
            static unsigned port = initial_port;
            return port++;
        }
    };
} 

#endif	/* BROKER_CONFIG_H */

