/* 
 * File:   admin_cmd.h
 * Author: Rohit Joshi <rohit.c.joshi@gmail.com>
 *
 * Created on March 9, 2015, 4:04 PM
 */

#ifndef ADMIN_CMD_H
#define	ADMIN_CMD_H
#include "thirdparty/picojson.h"
#include "log.h"

namespace lightq{
    
    namespace admin_cmd {
        
        
        struct join_req {
            std::string cmd_; // PUB, SUB, PULL, PUSH
            std::string topic_;
            std::string user_id_;
            std::string password_;
            std::string connection_type_; //zmq, nanomsg, raw_socket
            /**
             * from json
             * @param json_str
             * @param resp
             * @return 
             */
            bool from_json(const std::string& json_str) {
                LOG_IN("json_str[%s]", json_str.c_str());
                picojson::value v;
                std::string err = picojson::parse(v, json_str);
                if (!err.empty()) {
                    LOG_ERROR("Failed to parse json. Error[%s]",err.c_str());
                    LOG_RET_FALSE("failed");
                }
                cmd_ = v.get<std::string>("cmd");
                topic_ = v.get<std::string>("topic");
                user_id_ = v.get<std::string>("user");
                password_ = v.get<std::string>("password");
                connection_type_ = v.get<std::string>("password");
            }
        };
        
        struct join_resp {
            std::string status_; // OK or ERROR
            const std::string cmd_ = "JOIN";
            std::string topic_;
            std::string bind_uri_;
            
            std::string to_json() {
                picojson::value v;
                v["cmd"] = cmd_;
                v["status"] = status_;
                v["topic"] = topic_;
                v["bind_uri"] = bind_uri_;
                return v.serialize(true);
            }
            
        };
        
        struct stats_req {
            const std::string cmd_ = "STATS";
            std::string topic_;
            bool from_json(const std::string& json_str) {
                LOG_IN("json_str[%s]", json_str.c_str());
                picojson::value v;
                std::string err = picojson::parse(v, json_str);
                if (!err.empty()) {
                    LOG_ERROR("Failed to parse json. Error[%s]",err.c_str());
                    LOG_RET_FALSE("failed");
                }
                cmd_ = v.get<std::string>("cmd");
                topic_ = v.get<std::string>("topic");

            }
        };
        struct stats_resp {
            const std::string cmd_ = "STATS";
            std::string status_; // OK or ERROR
            std::string topic_;
            uint64_t queue_size_;
            uint16_t num_publisher;
            uint16_t num_subscribers_;
            
            std::string to_json() {
                picojson::value v;
                v["cmd"] = cmd_;
                v["status"] = status_;
                v["queue_size"] = queue_size_;
                v["num_publisher"] = num_publisher;
                v["num_subscribers_"] = num_subscribers_;
                return v.serialize(true);
            }
        };
        
        //FIXME
        struct create_broker_req {
                std::string broker_type_; //queue, file, direct 
                std::string topic_; 
                std::string producer_bind_uri_;
                connection::stream_type producer_stream_type_; 
                std::string consumer_pull_bind_uri_;
                std::string consumer_push_bind_uri_;
                connection::stream_type consumer_stream_type_; 
                
              
          
        };
        
    }
}


#endif	/* ADMIN_CMD_H */

