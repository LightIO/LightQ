/* 
 * File:   broker_manager.h
 * Author: Rohit Joshi <rohit.c.joshi@gmail.com>
 *
 * Created on February 26, 2015, 8:41 AM
 */

#ifndef BROKER_MANAGER_H
#define	BROKER_MANAGER_H
#include <iostream>
#include <string>
#include <map>
#include "log.h"
#include "connection.h"
#include "connection_zmq.h"
#include "broker_config.h"
#include "broker.h"


// //  zmq::context_t &context, const std::string& topic, 
 //     const std::string& uri,  endpoint_type conn_type
namespace lightq {
    
    //broker manager
    class broker_manager {
    public:
         
   
         /**
          * constructor
          * @param admin_uri
          * @param user_id
          * @param password
          */
        broker_manager(const std::string& admin_uri, const std::string& user_id="lightq_admin", 
                const std::string& password="T0p$3cr31")
                : admin_uri_(admin_uri), user_id_(user_id), password_(password){
            
                LOG_IN("admin_uri[%s], user_id[%s], password[%s]", 
                        admin_uri.c_str(), user_id.c_str(), password.c_str());
                
                stop_ = false;
                p_conn_admin_ = new connection_zmq(broker_mgr_topic_, 
                        admin_uri_, connection::endpoint_type::conn_broker,
                        connection_zmq::zmq_rep,connection::bind_socket,true, true);
                LOG_OUT("");
        }
        /**
         * destrct
         */
        ~broker_manager() {
             LOG_IN("");
            if(p_conn_admin_) {
                delete p_conn_admin_;
                p_conn_admin_ = NULL;
            }
              LOG_OUT("");
        }
        /**
         * initialize
         * @return 
         */
        bool init() {
            LOG_IN("");
           
            LOG_INFO("Initializjng zmq_connection with connection type: %d and enabled monitoring", connection_zmq::zmq_rep);
            if(p_conn_admin_->init()) {
                LOG_EVENT("Broker manager is initialized with bind uri %s", admin_uri_.c_str());
                std::string l = utils::format_str("Broker Manager is bind to %s", admin_uri_.c_str());
                LOG_RET_TRUE(l.c_str());
            }
             LOG_EVENT("Failed to initialize Broker manager at bind uri %s", admin_uri_.c_str());
            LOG_RET_FALSE(utils::format_str("Broker Manager failed to listen on %s", admin_uri_.c_str()).c_str());
        }
       
        /**
         * 
         * @return 
         */
        bool run() {
            LOG_IN("");
            std::string message;
            message.reserve(max_read_buffer_);
            while(!stop_) {
                LOG_TRACE("Reading incoming command request...");
                message.clear();
                ssize_t bytes_read = p_conn_admin_->read_msg(message);
                if(bytes_read < 0) {
                    LOG_ERROR("Failed to read from admin connection. Need to reinitialize");
                    LOG_RET_FALSE("failure");
                }
                //if bytes read zero, continue
                if(bytes_read == 0) {
                    LOG_DEBUG("Received zero byte.  continue");
                    continue;
                }
                LOG_INFO("Received :%s", message.c_str());
                LOG_EVENT("Received command: %s", message.c_str());
                
               
                bool result = process_command(message); //PERF:  Check if we need to have a seperate thread to process the command
                LOG_DEBUG("Process Command: %s" , result?"true":"false");

            }
            LOG_RET_TRUE("");
        }
        /**
         * Process command
         * @param message
         * @return 
         */
        ssize_t process_command(std::string& message) {
            LOG_IN("message: %s", message.c_str());
            ssize_t result = 0;
            if(strncmp(message.c_str(), CMD_LOGIN.c_str(), CMD_LOGIN.length()) == 0) {
                result = reply_login(message);
                
            }else if(strncmp(message.c_str(), CMD_PING.c_str(), CMD_PING.length()) == 0) {
                result =  reply_cmd(CMD_PONG);
                
            }else if(strncmp(message.c_str(), CMD_PUB.c_str(), CMD_PUB.length()) == 0) {
                result =  reply_to_join_broker(message, CMD_PUB);
                
            }else if(strncmp(message.c_str(), CMD_SUB.c_str(), CMD_SUB.length()) == 0) {
                result = reply_to_join_broker(message, CMD_SUB);
            }else if(strncmp(message.c_str(), CMD_PULL.c_str(), CMD_PULL.length()) == 0) {
                result = reply_to_join_broker(message, CMD_PULL);
            }else if(strncmp(message.c_str(), CMD_STATS.c_str(), CMD_STATS.length()) == 0) {
                result = reply_to_stats(message);
            }else {
                LOG_ERROR("Invalid command: %s", message.c_str());
                LOG_RET("invalid command", -1);
            }
            LOG_RET("", result);
        }
        
        /**
         * reply to stats
         * @param message
         * @return 
         */
        inline ssize_t reply_to_stats( std::string& message) {
            std::vector<std::string> tokens = utils::get_tokens(message, ' ');
            if(!validate_topic_partition(tokens[1], tokens[2])) {
                 return reply_cmd(CMD_INVALID);
            }
            std::string broker_id = tokens[1];
            broker_id.append("_");
            broker_id.append(tokens[2]);
            
             std::map<std::string, broker*>::iterator it = brokers_.find(broker_id);
            if( it == brokers_.end()) {
                return reply_invalid_cmd();
            }
             std::string stats = utils::format_str("STATS:  TotalMessagesReceived:%u, "
                     "TotalMessagesSent:%u,  QueueSize:%lld, NumSubClients:%u, NumPullClients:%u",
                      it->second->get_total_msg_received(),  it->second->get_total_msg_sent(),
                      it->second->get_queue_size(),
                      it->second->p_consumer_->get_num_pub_clients(), 
                      it->second->p_consumer_->get_num_pull_clients());
             LOG_EVENT("Status response: %s", stats.c_str());
             return reply_cmd(stats);
             
        }
        
       
        /**
         * reply with response
         * @param cmd
         * @return 
         */
        inline ssize_t reply_cmd(const std::string& cmd) {
            LOG_IN("cmd: %s", cmd.c_str());
            LOG_EVENT("Replying to client : %s", cmd.c_str());
            ssize_t size =  p_conn_admin_->write_msg(cmd);
            if(size <=0) {
                LOG_ERROR("Failed to send command : %s", cmd.c_str());
            }
            LOG_RET("size written: %d", size);
        }
        /**
         * invalid command
         * @return 
         */
        inline ssize_t reply_invalid_cmd() {
            LOG_IN("");
            ssize_t result =  p_conn_admin_->write_msg(CMD_INVALID);
            LOG_RET("", result);
        }
        
       
        
        inline ssize_t reply_to_join_broker(std::string& message, const std::string& cmd) {
            LOG_IN("message: %s", message.c_str());
            std::vector<std::string> tokens = utils::get_tokens(message, ' ');
            if(!validate_topic_partition(tokens[1], tokens[2])) {
                 return reply_cmd(CMD_INVALID);
            }
            std::string broker_id = tokens[1];
            broker_id.append("_");
            broker_id.append(tokens[2]);
            LOG_DEBUG("broker_id: %s", broker_id.c_str());
            broker_config::broker_type broker_type = broker_config::broker_queue;
            if(tokens.size() > 3) {
                if(tokens[3] == "queue") {
                    broker_type = broker_config::broker_queue;
                }else if(tokens[3] == "file") {
                     broker_type = broker_config::broker_file;
                }else if(tokens[3] == "direct") {
                     broker_type = broker_config::broker_direct;
                }else {
                    broker_type = broker_config::broker_queue;
                }
            }
            connection::stream_type stream = connection::stream_zmq;
            if(tokens.size() > 4) {
                if(tokens[4] == "socket") {
                    stream = connection::stream_socket;
                }
            }
            broker_config config;
            producer_config prod_conf;
            consumer_config consumer_conf;
            std::map<std::string, broker*>::iterator it = brokers_.find(broker_id);
            std::string response = "JOIN ";
            response.append(broker_id);
            response.append(" ");
            LOG_TRACE("Command is %s", cmd.c_str());
            if( it == brokers_.end()) {
                LOG_DEBUG("broker with %s not found. Creating new..", broker_id.c_str());
              //  broker_config config;
                config.id_ = broker_id;
                config.broker_type_ = broker_type;
                prod_conf.id_ = broker_id;
                prod_conf.producer_bind_uri_ = config.bind_interface;
                prod_conf.producer_bind_uri_.append(":");
                prod_conf.producer_bind_uri_.append(std::to_string(broker_config::get_next_port()));          
                prod_conf.producer_stream_type_ = connection::stream_type::stream_zmq;
                prod_conf.producer_socket_connect_type_ = connection::bind_socket;
                
                consumer_conf.id_ = broker_id;
                consumer_conf.push_bind_uri_ = config.bind_interface;
                consumer_conf.push_bind_uri_.append(":");
                consumer_conf.push_bind_uri_.append(std::to_string(broker_config::get_next_port()));
                consumer_conf.pub_bind_uri_ = config.bind_interface;
                consumer_conf.pub_bind_uri_.append(":");
                consumer_conf.pub_bind_uri_.append(std::to_string(broker_config::get_next_port()));
                consumer_conf.stream_type_ = stream;
                consumer_conf.socket_connect_type_ = connection::bind_socket;
            
              
            
                broker  *pb = new broker(config);
                if(pb->init(prod_conf, consumer_conf) && pb->run()) {
                    LOG_EVENT("Broker %s started successfully", config.to_string().c_str());
                    brokers_.insert(std::pair<std::string, broker*>(broker_id, pb));
                    if(cmd == "PUB") {
                        response.append(prod_conf.producer_bind_uri_);
                    }else if(cmd == "SUB") {
                        response.append(consumer_conf.pub_bind_uri_);
                    }else {
                        response.append(consumer_conf.push_bind_uri_);
                    }
                     LOG_TRACE("Sending reply: %s", response.c_str());
                    ssize_t result = reply_cmd(response);
                    LOG_RET("", result);
                }else {
                    delete pb;
                    LOG_ERROR("Failed to start broker %s", config.to_string().c_str());
                    LOG_EVENT("Failed to start broker %s", config.to_string().c_str());
                    ssize_t result = reply_cmd(CMD_SERVER_ERROR);
                    LOG_RET("", result);
                }
            }else {
               
                config = it->second->get_config();
                LOG_EVENT("Reusing existing broker :%s",  config.to_string().c_str());
                if(cmd == "PUB") {
                        response.append(it->second->get_producer()->get_bind_uri());
                    }else if(cmd == "SUB") {
                        response.append(it->second->get_consumer()->get_pub_bind_uri());
                    }else {
                        response.append(it->second->get_consumer()->get_push_bind_uri());
                    }
                     LOG_TRACE("Sending reply: %s", response.c_str());
                    ssize_t result = reply_cmd(response);
                    LOG_RET("", result);
            }
            ssize_t result = reply_cmd(CMD_SERVER_ERROR);
            LOG_RET("should not come here", -1);
           
            
           
           
        }
        /**
         * validate if we decided to restrict the token name and partition
         * @param topic
         * @param partition
         * @return 
         */
        bool validate_topic_partition(const std::string& topic, const std::string& partition) {
            LOG_IN("");
            LOG_IN("topic:%s, partition:%s", topic.c_str(), partition.c_str());
            LOG_RET_TRUE("validated");
        }
        
        /**
         * reply to login command
         * @param message
         * @return 
         */
        inline ssize_t reply_login(std::string& message) {
            LOG_IN("");
            std::vector<std::string> tokens = utils::get_tokens(message, ' ');
            if(tokens.size() < 3) {
                return reply_cmd(CMD_INVALID);
            }
            if(!authenticate(tokens[1], tokens[2])) {
                return reply_cmd(CMD_INVALID);
            }else {
                return reply_cmd(CMD_LOGIN_SUCCESS);
            }
            LOG_RET("shouldn't come here", -1);
            
        }
        /**
         * authenticate 
         * @param user
         * @param password
         * @return 
         * FIXME:  need some advance login/password, may be encrypted + per producer/consumer, login/password
         */
        bool authenticate(const std::string& user, const std::string& password){
            LOG_IN("user: %s, password: %s", user.c_str(), password.c_str());
            LOG_TRACE("user_id_[%s], password_[%s]", user_id_.c_str(), password_.c_str());
            if(user_id_ == user && password_ == password) {
               
                LOG_RET_TRUE("Login success");
            }
            LOG_RET_FALSE("Login failed");
        }
        
        
        
    private:
       
        connection * p_conn_admin_;
        std::string  admin_uri_;
        std::string user_id_;
        std::string password_;
        bool stop_;

        std::map<std::string, broker*> brokers_;
        
         const std::string broker_mgr_topic_ = "prakashq_topic";
         const unsigned max_read_buffer_ = 4*1024;
         const std::string CMD_INVALID = "INVALID COMMAND";
         const std::string CMD_PUB = "PUB";
         const std::string CMD_SUB = "SUB";
         const std::string CMD_PULL = "PULL";
         const std::string CMD_UNSUB = "UNSUB";
         const std::string CMD_DISCONECT = "DISCONNECT";
         const std::string CMD_PING = "PING";
         const std::string CMD_PONG = "PONG";
         const std::string CMD_LOGIN = "LOGIN";
         const std::string CMD_UNAUTH = "UNAUTHORIZED_ACCESS";
         const std::string CMD_LOGIN_SUCCESS = "LOGIN_SUCCESS";
         const std::string CMD_SERVER_ERROR = "SERVER_ERROR";
         const std::string CMD_STATS = "STATS";
       

    };
}

#endif	/* BROKER_MANAGER_H */

