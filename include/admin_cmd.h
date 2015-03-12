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

namespace lightq {

    class admin_cmd {
    public:

        static bool get_cmd(const std::string& json_str, std::string& cmd) {
            LOG_IN("json_str[%s]", json_str.c_str());
            picojson::value v;
            std::string err = picojson::parse(v, json_str);
            if (!err.empty()) {
                LOG_ERROR("Failed to parse json. Error[%s]", err.c_str());
                LOG_RET_FALSE("failed");
            }
           
            cmd = v.get("cmd").get<std::string>();
            LOG_RET_TRUE("");
        }

        struct common_resp {
            std::string cmd_;
            std::string status_; // OK or ERROR
            std::string description_;

            std::string to_json() {
                LOG_IN("");
                picojson::value::object obj;
                obj["cmd"] = picojson::value(cmd_);
                obj["status"] =  picojson::value(status_);
                obj["description"] = picojson::value(description_);
                picojson::value v(obj);
                std::string json_str = v.serialize(true);
                LOG_TRACE("json_str [%s]", json_str.c_str());
                return std::move(json_str);
            }
            bool from_json(const std::string& json_str) {
                LOG_IN("json_str[%s]", json_str.c_str());
                picojson::value v;
                std::string err = picojson::parse(v, json_str);
                if (!err.empty()) {
                    LOG_ERROR("Failed to parse json. Error[%s]", err.c_str());
                    LOG_RET_FALSE("failed");
                }
                if(v.get("cmd").is<std::string>())
                    cmd_ = v.get("cmd").get<std::string>();
                if(v.get("status").is<std::string>())
                    status_ = v.get("status").get<std::string>();
                if(v.get("description").is<std::string>())
                    description_ = v.get("description").get<std::string>();
              
                LOG_RET_TRUE("");
            }
        };

        struct join_req {
            const std::string cmd_ = "join";
            std::string type_; // PUB, SUB, PULL, PUSH
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
                    LOG_ERROR("Failed to parse json. Error[%s]", err.c_str());
                    LOG_RET_FALSE("failed");
                }
                if(v.get("type").is<std::string>())
                    type_ = v.get("type").get<std::string>();
                if(v.get("topic").is<std::string>())
                    topic_ = v.get("topic").get<std::string>();
                if(v.get("user_id").is<std::string>())
                    user_id_ = v.get("user_id").get<std::string>();
                if(v.get("password").is<std::string>())
                    password_ = v.get("password").get<std::string>();
                if(v.get("connection_type").is<std::string>())
                    connection_type_ = v.get("connection_type").get<std::string>();
                LOG_RET_TRUE("");
            }

            /**
             * tojson
             * @return 
             */
            std::string to_json(bool mask_password=false) {
                LOG_IN("");
                picojson::value::object obj;
                obj["cmd"] = picojson::value(cmd_);
                obj["type"] = picojson::value(type_);
                obj["topic"] = picojson::value(topic_);
                obj["user_id"] = picojson::value(user_id_);
                if(mask_password) { 
                    obj["password"] = picojson::value("******");
                } else {
                    obj["password"] = picojson::value(password_);
                }
                obj["connection_type"] = picojson::value(connection_type_);
                picojson::value v(obj);
                std::string json_str = v.serialize(true);
                LOG_TRACE("json_str [%s]", json_str.c_str());
                return std::move(json_str);
            }
        };

        struct join_resp {
            const std::string cmd_ = "join";
            std::string status_; // OK or ERROR
            std::string topic_;
            std::string bind_uri_;

            std::string to_json() {
                LOG_IN("");
                picojson::value::object obj;
                obj["cmd"] = picojson::value(cmd_);
                obj["status"] = picojson::value(status_);
                obj["topic"] = picojson::value(topic_);
                obj["bind_uri"] = picojson::value(bind_uri_);
                picojson::value v(obj);
                std::string json_str = v.serialize(true);
                LOG_TRACE("json_str [%s]", json_str.c_str());
                return std::move(json_str);
            }
            
             bool from_json(const std::string& json_str) {
                LOG_IN("json_str[%s]", json_str.c_str());
                picojson::value v;
                std::string err = picojson::parse(v, json_str);
                if (!err.empty()) {
                    LOG_ERROR("Failed to parse json. Error[%s]", err.c_str());
                    LOG_RET_FALSE("failed");
                }
                if(v.get("topic").is<std::string>())
                    topic_ = v.get("topic").get<std::string>();
                if(v.get("status").is<std::string>())
                    status_ = v.get("status").get<std::string>();
                if(v.get("bind_uri").is<std::string>())
                    bind_uri_ = v.get("bind_uri").get<std::string>();
                LOG_RET_TRUE("");
            }

        };

        struct stats_req {
            const std::string cmd_ = "stats";
            std::string topic_;
            std::string user_id_;
            std::string password_;

            bool from_json(const std::string& json_str) {
                LOG_IN("json_str[%s]", json_str.c_str());
                picojson::value v;
                std::string err = picojson::parse(v, json_str);
                if (!err.empty()) {
                    LOG_ERROR("Failed to parse json. Error[%s]", err.c_str());
                    LOG_RET_FALSE("failed");
                }
                if(v.get("topic").is<std::string>())
                    topic_ = v.get("topic").get<std::string>();
                if(v.get("user_id").is<std::string>())
                    user_id_ = v.get("user_id").get<std::string>();
                if(v.get("password").is<std::string>()) 
                    password_ = v.get("password").get<std::string>();
                LOG_RET_TRUE("");

            }

            std::string to_json(bool mask_password=false) {
                LOG_IN("");
                picojson::value::object obj;
                obj["cmd"] =   picojson::value(cmd_);
                obj["topic"] = picojson::value(topic_);
                obj["user_id"] = picojson::value(user_id_);
                if(mask_password) { 
                    obj["password"] = picojson::value("******");
                } else {
                    obj["password"] = picojson::value(password_);
                }
                picojson::value v(obj);
                std::string json_str = v.serialize(true);
                LOG_TRACE("json_str [%s]", json_str.c_str());
                return std::move(json_str);
            }
        };

        struct stats_resp {
            const std::string cmd_str = "cmd";
            const std::string status_str = "status";
            const std::string queue_size_str = "queue_size";
            const std::string messages_sent_str = "messages_sent";
            const std::string messages_received_str = "messages_received";
            const std::string publishers_count_str = "publishers_count";
            const std::string subscribers_count_str = "subscribers_count";
            const std::string total_bytes_written_str = "total_bytes_written";
            const std::string total_bytes_read_str = "total_bytes_read";
            const std::string cmd_ = "stats";
            std::string status_;
            std::string topic_;
            int64_t queue_size_;
            int64_t messages_sent_;
            int64_t messages_received_;
            int64_t publishers_count_;
            int64_t subscribers_count_;
            int64_t total_bytes_written_;
            int64_t total_bytes_read_;
            
            stats_resp() {
                status_ = "";
                topic_ = "";
                queue_size_ = 0;
                messages_sent_ = 0;
                messages_received_ = 0;
                publishers_count_ = 0;
                subscribers_count_ = 0;
                total_bytes_written_ = 0;
                total_bytes_read_ = 0;
            }

            std::string to_json() {
                picojson::value::object obj;
                obj[cmd_str] = picojson::value(cmd_);
                obj[status_str] = picojson::value(status_);
                obj[queue_size_str] = picojson::value(queue_size_);
                obj[messages_sent_str] = picojson::value(messages_sent_);
                obj[messages_received_str] = picojson::value(messages_received_);
                obj[publishers_count_str] = picojson::value(publishers_count_);
                obj[subscribers_count_str] = picojson::value(subscribers_count_);
                obj[total_bytes_written_str] = picojson::value(total_bytes_written_);
                obj[total_bytes_read_str] = picojson::value(total_bytes_read_);
                picojson::value v(obj);
                std::string json_str = v.serialize(true);
                LOG_TRACE("json_str [%s]", json_str.c_str());
                return std::move(json_str);
            }
            bool from_json(const std::string& json_str) {
                LOG_IN("json_str[%s]", json_str.c_str());
                picojson::value v;
                std::string err = picojson::parse(v, json_str);
                if (!err.empty()) {
                    LOG_ERROR("Failed to parse json. Error[%s]", err.c_str());
                    LOG_RET_FALSE("failed");
                }
                if(v.get(status_str).is<std::string>())
                    status_ = v.get(status_str).get<std::string>();
                if(v.get(queue_size_str).is<int64_t>())
                    queue_size_ = v.get(queue_size_str).get<int64_t>();
                if(v.get(messages_sent_str).is<int64_t>())
                    messages_sent_ = v.get(messages_sent_str).get<int64_t>();
                if(v.get(messages_received_str).is<int64_t>())
                    messages_received_ = v.get(messages_received_str).get<int64_t>();
                if(v.get(publishers_count_str).is<int64_t>())
                    publishers_count_ = v.get(publishers_count_str).get<int64_t>();
                if(v.get(subscribers_count_str).is<int64_t>())
                    subscribers_count_ = v.get(subscribers_count_str).get<int64_t>();
                
                if(v.get(total_bytes_written_str).is<int64_t>())
                    total_bytes_written_ = v.get(total_bytes_written_str).get<int64_t>();
                if(v.get(total_bytes_read_str).is<int64_t>())
                    total_bytes_read_ = v.get(total_bytes_read_str).get<int64_t>();
                LOG_RET_TRUE("");

            }
        };

        //FIXME

        struct create_topic_req {
            const std::string cmd_ = "create_topic";
            std::string topic_;
            std::string broker_type_; //queue, file, direct 
            std::string admin_user_id_;
            std::string admin_password_;
            std::string user_id_;
            std::string password_;

            bool from_json(const std::string& json_str) {
                LOG_IN("json_str[%s]", json_str.c_str());
                picojson::value v;
                std::string err = picojson::parse(v, json_str);
                if (!err.empty()) {
                    LOG_ERROR("Failed to parse json. Error[%s]", err.c_str());
                    LOG_RET_FALSE("failed");
                }
                if(v.get("topic").is<std::string>())
                    topic_ = v.get("topic").get<std::string>();
                if(v.get("broker_type").is<std::string>())
                    broker_type_ = v.get("broker_type").get<std::string>();
                if(v.get("admin_user_id").is<std::string>())
                    admin_user_id_ = v.get("admin_user_id").get<std::string>();
                if(v.get("admin_password").is<std::string>())
                    admin_password_ = v.get("admin_password").get<std::string>();
                if(v.get("user_id").is<std::string>())
                    user_id_ = v.get("user_id").get<std::string>();
                if(v.get("password").is<std::string>())
                    password_ = v.get("password").get<std::string>();
                LOG_RET_TRUE("");

            }

            std::string to_json(bool mask_password=false) {
                LOG_IN("");
                picojson::value::object obj;
                obj["cmd"] = picojson::value(cmd_);
                obj["topic"] = picojson::value(topic_);
                obj["broker_type"] = picojson::value(broker_type_);
                obj["admin_user_id"] = picojson::value(admin_user_id_);
                if(mask_password) { 
                    obj["admin_password"] = picojson::value("******");
                } else {
                    obj["admin_password"] = picojson::value(admin_password_);
                }
                obj["user_id"] = picojson::value(user_id_);
                if(mask_password) { 
                    obj["password"] = picojson::value("******");
                } else {
                    obj["password"] = picojson::value(password_);
                }
                picojson::value v(obj);
                std::string json_str = v.serialize(true);
                LOG_TRACE("json_str [%s]", json_str.c_str());
                return std::move(json_str);
            }
        };

    };
}


#endif	/* ADMIN_CMD_H */

