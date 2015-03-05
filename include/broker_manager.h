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
#include "thirdparty/log.hpp"
#include "connection.h"
#include "connection_zmq.h"
#include "broker_config.h"
#include "broker.h"


// //  zmq::context_t &context, const std::string& topic, 
 //     const std::string& uri,  endpoint_type conn_type
namespace prakashq {
    
    //broker manager
    class broker_manager {
    public:
         const std::string broker_mgr_topic_ = "prakashq_topic";
         const unsigned max_read_buffer_ = 4*1024;
         const std::string CMD_INVALID = "INVALID COMMAND";
         const std::string CMD_PUB = "PUB";
         const std::string CMD_SUB = "SUB";
         const std::string CMD_UNSUB = "UNSUB";
         const std::string CMD_DISCONECT = "DISCONNECT";
         const std::string CMD_PING = "PING";
         const std::string CMD_PONG = "PONG";
         const std::string CMD_LOGIN = "LOGIN";
         const std::string CMD_UNAUTH = "UNAUTHORIZED ACCESS";
         const std::string CMD_LOGIN_SUCCESS = "LOGIN SUCCESS";
         const std::string CMD_SERVER_ERROR = "SERVER ERROR";
         const std::string CMD_STATS = "STATS";
   
        broker_manager(const std::string& admin_uri) : 
            admin_uri_(admin_uri){
                LOG_IN("admin_uri: %s", admin_uri.c_str());
                stop_ = false;
                 p_conn_admin_ = new connection_zmq(broker_mgr_topic_, admin_uri_, connection::endpoint_type::conn_broker);
                LOG_OUT("");
        }
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
            connection_zmq *p_zmq_conn = (connection_zmq*)p_conn_admin_;
            LOG_INFO("Initializjng zmq_connection with connection type: %d and enabled monitoring", connection_zmq::zmq_rep);
            if(p_zmq_conn->init(connection_zmq::zmq_rep, true)) {
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
                ssize_t bytes_read = p_conn_admin_->read_msg(message);
                if(bytes_read < 0) {
                    LOG_ERROR("Failed to read from admin connection. Need to reinitialize");
                    LOG_RET_FALSE("failure");
                }
                //if bytes read zero, continue
                if(bytes_read == 0) {
                    LOG_TRACE("Received zero byte.  continue");
                    continue;
                }
                LOG_INFO("Received :%s", message.c_str());
                LOG_EVENT("Received command: %s", message.c_str());
                
               
                bool result = process_command(message); //PERF:  Check if we need to have a seperate thread to process the command
                LOG_TRACE("Process Command: %d" , result);

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
            }else if(strncmp(message.c_str(), CMD_STATS.c_str(), CMD_STATS.length()) == 0) {
                result = reply_to_stats(message);
            }else {
                LOG_ERROR("Invalid command: %s", message.c_str());
                LOG_RET("invalid command", -1);
            }
            LOG_RET("", result);
        }
        
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
             std::string stats = utils::format_str("STATS:  TotalMessagesReceived: %u, "
                     "TotalMessagesSent: %u,  QueueSize: %lld",
                      it->second->get_total_msg_received(),  it->second->get_total_msg_sent(),
                      it->second->get_queue_size());
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
            ssize_t size =  p_conn_admin_->write(cmd);
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
            ssize_t result =  p_conn_admin_->write(CMD_INVALID);
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
            LOG_TRACE("broker_id: %s", broker_id.c_str());
            broker_config::broker_type broker_type = broker_config::broker_queue;
            if(tokens.size() > 3) {
                if(tokens[3] == "queue") {
                    broker_type = broker_config::broker_queue;
                }else if(tokens[3] == "file") {
                     broker_type = broker_config::broker_file;
                }else if(tokens[3] == "queue_file") {
                     broker_type = broker_config::broker_queue_file;
                }else if(tokens[3] == "file_queue") {
                     broker_type = broker_config::broker_file_queue;
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
            std::map<std::string, broker*>::iterator it = brokers_.find(broker_id);
            if( it == brokers_.end()) {
              //  broker_config config;
                config.id_ = broker_id;
                config.broker_type_ = broker_type;
                config.producer_bind_uri_ = config.bind_interface;
                config.producer_bind_uri_.append(":");
                config.producer_bind_uri_.append(std::to_string(broker_config::get_next_port()));          
                config.producer_stream_type_ = connection::stream_type::stream_zmq;
            
                config.consumer_bind_uri_ = config.bind_interface;
                config.consumer_bind_uri_.append(":");
                config.consumer_bind_uri_.append(std::to_string(broker_config::get_next_port()));
                config.consumer_stream_type_ = stream;
            
            
                broker  *pb = new broker(config);
                if(pb->init() && pb->run()) {
                    LOG_EVENT("Broker %s started successfully", config.to_string().c_str());
                    brokers_.insert(std::pair<std::string, broker*>(broker_id, pb));
                }else {
                    delete pb;
                    LOG_ERROR("Failed to start broker %s", config.to_string().c_str());
                    LOG_EVENT("Failed to start broker %s", config.to_string().c_str());
                    ssize_t result = reply_cmd(CMD_SERVER_ERROR);
                    LOG_RET("", result);
                }
            }else {
               
                config = it->second->config_;
                 LOG_EVENT("Reusing existing broker :%s",  config.to_string().c_str());
            }
            std::string response = "JOIN ";
            response.append(config.id_);
            response.append(" ");
            if(cmd == "PUB") {
                response.append(config.producer_bind_uri_);
            }else {
                response.append(config.consumer_bind_uri_);
                
            }
            ssize_t result = reply_cmd(response);
            LOG_RET("", result);
           
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
                reply_cmd(CMD_LOGIN_SUCCESS);
            }
            
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
            LOG_RET_TRUE("Login success");
            if(user_ == user && password_ == password) {
                LOG_RET_TRUE("Login success");
            }
            LOG_RET_FALSE("Login failed");
        }
        
        
        
    private:
       
        connection * p_conn_admin_;
        std::string  admin_uri_;
        bool stop_;
        const std::string  password_;
        const std::string  user_;
        std::map<std::string, broker*> brokers_;
       

    };
}

#endif	/* BROKER_MANAGER_H */

