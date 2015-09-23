/* 
 * File:   broker_manager.h
 * Author: Rohit Joshi <rohit.c.joshi@gmail.com>
 *
 * Created on February 26, 2015, 8:41 AM
 */

#ifndef BROKER_MANAGER_H
#define    BROKER_MANAGER_H

#include <iostream>
#include <string>
#include <map>
#include "log.h"
#include "connection.h"
#include "connection_zmq.h"
#include "broker_config.h"
#include "broker.h"
#include "admin_cmd.h"


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
      broker_manager(
          const std::string &admin_uri, const std::string &user_id = "lightq_admin",
          const std::string &password = "T0p$3cr31")
          : admin_uri_(admin_uri), user_id_(user_id), password_(password) {

          LOG_IN("admin_uri[%s], user_id[%s], password[%s]",
                 admin_uri.c_str(), user_id.c_str(), password.c_str());

          stop_ = false;
          p_conn_admin_ = new connection_zmq(
              broker_mgr_topic_,
              admin_uri_, connection::endpoint_type::conn_broker,
              connection_zmq::zmq_rep, connection::bind_socket, true, true);
          LOG_OUT("");
      }

      /**
       * destrct
       */
      ~broker_manager() {
          LOG_IN("");
          delete p_conn_admin_;
          LOG_OUT("");
      }

      /**
       * initialize
       * @return
       */
      bool init() {
          LOG_IN("");

          LOG_INFO("Initializjng zmq_connection with connection type: %d and enabled monitoring",
                   connection_zmq::zmq_rep);
          if (p_conn_admin_->init()) {
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
          while (!stop_) {
              LOG_TRACE("Reading incoming command request...");
              message.clear();
              ssize_t bytes_read = p_conn_admin_->read_msg(message);
              if (bytes_read < 0) {
                  LOG_ERROR("Failed to read from admin connection. Need to reinitialize");
                  LOG_RET_FALSE("failure");
              }
              //if bytes read zero, continue
              if (bytes_read == 0) {
                  LOG_DEBUG("Received zero byte.  continue");
                  continue;
              }
              //sec: validate userinput length
              if(bytes_read > max_read_buffer_) {
                LOG_ERROR("Received command length :%ld excceds the allowed command lenth. returning invalid.", bytes_read);
                 return reply_invalid_cmd(message);
              }
              LOG_DEBUG("Received :%s", message.c_str());
              //  LOG_EVENT("Received command: %s", message.c_str());

              bool result = process_command(
                  message); //PERF:  Check if we need to have a seperate thread to process the command
              LOG_DEBUG("Process Command: %s", result ? "true" : "false");

          }
          LOG_RET_TRUE("");
      }

      /**
       * Process command
       * @param message
       * @return
       */
      ssize_t process_command(std::string &message) {
          LOG_IN("message: %s", message.c_str());
          ssize_t result = 0;
          std::string cmd;
          if (!admin_cmd::get_cmd(message, cmd)) {
              return reply_invalid_cmd(message);
          }

          if (cmd == CMD_PING) {
              return reply_cmd(CMD_PONG);

          } else if (cmd == CMD_JOIN) {
              admin_cmd::join_req req;
              if (!req.from_json(message)) {
                  return reply_invalid_cmd(cmd);
              }
              return reply_to_join_topic(req);
          } else if (cmd == CMD_STATS) {
              admin_cmd::stats_req req;
              if (!req.from_json(message)) {
                  return reply_invalid_cmd(cmd);
              }
              return reply_to_stats(req);
          } else if (cmd == CMD_CREATE_TOPIC) {
              admin_cmd::create_topic_req req;
              if (!req.from_json(message)) {
                  return reply_invalid_cmd(cmd);
              }
              return reply_create_topic(req);
          } else {
              return reply_invalid_cmd(cmd);
          }
          LOG_RET("", result);
      }

      /**
       * reply invalid command
       * @param cmd
       * @return
       */
      ssize_t reply_invalid_cmd(const std::string &cmd) {
          LOG_IN("cmd[%s]", cmd.c_str());
          admin_cmd::common_resp resp;
          resp.cmd_ = cmd;
          resp.status_ = STATUS_ERROR;
          resp.description_ = CMD_INVALID;
          std::string resp_str = resp.to_json();
          LOG_EVENT("Status response: %s", resp_str.c_str());
          return reply_cmd(resp_str);
      }

      /**
       * reply to stats
       * @param message
       * @return
       */
      ssize_t reply_to_stats(admin_cmd::stats_req &req) {
          LOG_IN("req [%s]", req.to_json().c_str()); //FIXME: to_str
          std::map<std::string, broker *>::iterator it = brokers_.find(req.topic_);
          if (it == brokers_.end()) {
              admin_cmd::common_resp resp;
              resp.cmd_ = req.cmd_;
              resp.status_ = STATUS_ERROR;
              resp.description_ = STATUS_TOPIC_NOT_FOUND;
              std::string resp_str = resp.to_json();
              LOG_EVENT("Status response: %s", resp_str.c_str());
              return reply_cmd(resp_str);
          }
          broker_config conf = it->second->get_config();
          if (conf.user_id_ != req.user_id_ || conf.password_ != req.password_) {
              LOG_EVENT("Unauthorized user[%s]", req.user_id_.c_str());
              LOG_TRACE("req.user_id_[%s], req.password_[%s]", req.user_id_.c_str(), req.password_.c_str());
              admin_cmd::common_resp resp;
              resp.cmd_ = req.cmd_;
              resp.status_ = STATUS_ERROR;
              resp.description_ = CMD_UNAUTH;
              std::string resp_str = resp.to_json();
              LOG_EVENT("Status response: %s", resp_str.c_str());
              return reply_cmd(resp_str);
          }
          admin_cmd::stats_resp resp;
          resp.status_ = STATUS_SUCCESS;
          resp.topic_ = req.topic_;
          resp.topic_type_ = it->second->get_config().get_broker_type_to_str();
          resp.queue_size_ = it->second->get_queue_size();
          resp.messages_received_ = it->second->get_total_msg_received();
          resp.messages_sent_ = it->second->get_total_msg_sent();
          resp.total_bytes_written_ = it->second->get_storage().get_file_total_bytes_written();
          resp.total_bytes_read_ = it->second->get_storage().get_total_bytes_read();
          if (it->second->get_producer()) {
              resp.publishers_count_ = it->second->get_producer()->get_num_clients();

          } else {
              resp.publishers_count_ = 0;
          }
          if (it->second->get_consumer()) {
              resp.subscribers_count_ = it->second->get_consumer()->get_num_pub_clients()
                                        + it->second->get_consumer()->get_num_pull_clients();
          } else {
              resp.subscribers_count_ = 0;
          }
          std::string resp_str = resp.to_json();
          LOG_EVENT("Status response: %s", resp_str.c_str());
          return reply_cmd(resp_str);
      }

      /**
       * reply with response
       * @param cmd
       * @return
       */
      inline ssize_t reply_cmd(const std::string &cmd) {
          LOG_IN("cmd: %s", cmd.c_str());
          LOG_EVENT("Replying to client : %s", cmd.c_str());
          ssize_t size = p_conn_admin_->write_msg(cmd);
          if (size <= 0) {
              LOG_ERROR("Failed to send command : %s", cmd.c_str());
          }
          LOG_RET("size written: %d", size);
      }

      /**
       * create a topic
       * @param req
       * @return
       */
      ssize_t reply_create_topic(admin_cmd::create_topic_req &req) {
          LOG_IN("req [%s]", req.to_json().c_str());
          std::map<std::string, broker *>::iterator it = brokers_.find(req.topic_);
          if (it != brokers_.end()) {
              admin_cmd::common_resp resp;
              resp.cmd_ = req.cmd_;
              resp.status_ = STATUS_ERROR;
              resp.description_ = STATUS_TOPIC_EXISTS;
              std::string resp_str = resp.to_json();
              LOG_EVENT("Status response: %s", resp_str.c_str());
              return reply_cmd(resp_str);
          }
          if (user_id_ != req.admin_user_id_ || password_ != req.admin_password_) {
              LOG_EVENT("Unauthorized user[%s]", req.admin_user_id_.c_str());
              LOG_TRACE("req.admin_user_id_[%s], req.admin_password_[%s]", req.admin_user_id_.c_str(),
                        req.admin_password_.c_str());
              admin_cmd::common_resp resp;
              resp.cmd_ = req.cmd_;
              resp.status_ = STATUS_ERROR;
              resp.description_ = CMD_UNAUTH;
              std::string resp_str = resp.to_json();
              LOG_EVENT("Status response: %s", resp_str.c_str());
              return reply_cmd(resp_str);
          }
          broker_config config;
          config.id_ = req.topic_;
          config.user_id_ = req.user_id_;
          config.password_ = req.password_;
          LOG_DEBUG("creating broker_id: %s", config.id_.c_str());

          broker_config::broker_type broker_type = broker_config::broker_queue;
          LOG_DEBUG("Broker type is %s", req.broker_type_.c_str());
          if (req.broker_type_ == "queue") { //FIXME:  Remove hardcoded strings
              broker_type = broker_config::broker_queue;
          } else if (req.broker_type_ == "file") {
              broker_type = broker_config::broker_file;
          } else if (req.broker_type_ == "direct") {
              broker_type = broker_config::broker_direct;
          } else if (req.broker_type_ == "queue_file") {
              broker_type = broker_config::broker_queue_file;
          } else {
              broker_type = broker_config::broker_queue; //default
          }
          config.broker_type_ = broker_type;
          broker *pb = new broker(config);
          if (!pb->init()) {
              admin_cmd::common_resp resp;
              resp.cmd_ = req.cmd_;
              resp.status_ = STATUS_ERROR;
              resp.description_ = "Failed to initialize topic";
              std::string resp_str = resp.to_json();
              LOG_EVENT("Status response: %s", resp_str.c_str());
              return reply_cmd(resp_str);
          }
          brokers_.insert(std::pair<std::string, broker *>(config.id_, pb));
          admin_cmd::common_resp resp;
          resp.cmd_ = req.cmd_;
          resp.status_ = STATUS_SUCCESS;
          resp.description_ = STATUS_TOPIC_CREATED;
          std::string resp_str = resp.to_json();
          LOG_EVENT("Status response: %s", resp_str.c_str());
          return reply_cmd(resp_str);


      }

      /**
       * reply to join broker
       * @param req
       * @return
       */
      ssize_t reply_to_join_topic(admin_cmd::join_req &req) {
          LOG_IN("req [%s]", req.to_json().c_str());
          std::map<std::string, broker *>::iterator it = brokers_.find(req.topic_);
          if (it != brokers_.end()) {
              LOG_DEBUG("Found existing topic[%s]", req.topic_.c_str());
              broker_config conf = it->second->get_config();
              if (conf.user_id_ != req.user_id_ || conf.password_ != req.password_) {
                  LOG_EVENT("Unauthorized user[%s]", req.user_id_.c_str());
                  LOG_TRACE("req.user_id_[%s], req.password_[%s]", req.user_id_.c_str(), req.password_.c_str());
                  admin_cmd::common_resp resp;
                  resp.cmd_ = req.cmd_;
                  resp.status_ = STATUS_ERROR;
                  resp.description_ = CMD_UNAUTH;
                  std::string resp_str = resp.to_json();
                  LOG_EVENT("Status response: %s", resp_str.c_str());
                  return reply_cmd(resp_str);
              }
              admin_cmd::join_resp resp;
              resp.topic_ = req.topic_;
              resp.status_ = STATUS_SUCCESS;

              if (req.type_ == "pub") {
                  LOG_TRACE("request type is pub");
                  if (it->second->get_producer() == NULL) {
                      LOG_TRACE("Producer is null. Initializing...");
                      producer_config prod_conf;
                      prod_conf.id_ = it->second->get_config().id_;
                      prod_conf.producer_bind_uri_ = it->second->get_config().bind_interface;
                      prod_conf.producer_bind_uri_.append(":");
                      prod_conf.producer_bind_uri_.append(std::to_string(broker_config::get_next_port()));
                      prod_conf.producer_stream_type_ = connection::stream_type::stream_zmq;
                      prod_conf.producer_socket_connect_type_ = connection::bind_socket;
                      if (!it->second->init_producer(prod_conf)) {
                          admin_cmd::common_resp cmd_resp;
                          cmd_resp.cmd_ = req.cmd_;
                          cmd_resp.status_ = STATUS_ERROR;
                          cmd_resp.description_ = "Failed to initialize producer endpoint";
                          std::string resp_str = cmd_resp.to_json();
                          LOG_EVENT("Status response: %s", resp_str.c_str());
                          return reply_cmd(resp_str);
                      }
                  } else {
                      LOG_DEBUG("Producer is existing. Reusing..");
                  }
                  if (it->second->get_producer()) {
                      resp.bind_uri_ = it->second->get_producer()->get_bind_uri();
                      utils::replace(resp.bind_uri_, "*", "127.0.0.1"); //FIXME usenetwork interfacd
                      std::string resp_str = resp.to_json();
                      LOG_EVENT("Status response: %s", resp_str.c_str());
                      return reply_cmd(resp_str);
                  } else {
                      LOG_ERROR("THis should not happend")
                      admin_cmd::common_resp cmd_resp;
                      cmd_resp.cmd_ = req.cmd_;
                      cmd_resp.status_ = STATUS_ERROR;
                      cmd_resp.description_ = "internal server error";
                      std::string resp_str = cmd_resp.to_json();
                      LOG_EVENT("Status response: %s", resp_str.c_str());
                      return reply_cmd(resp_str);
                  }
              } else {
                  if (it->second->get_consumer() == NULL) {
                      consumer_config consumer_conf;
                      consumer_conf.id_ = it->second->get_config().id_;
                      consumer_conf.pub_bind_uri_ = it->second->get_config().bind_interface;
                      consumer_conf.pub_bind_uri_.append(":");
                      consumer_conf.pub_bind_uri_.append(std::to_string(broker_config::get_next_port()));
                      consumer_conf.push_bind_uri_ = it->second->get_config().bind_interface;
                      consumer_conf.push_bind_uri_.append(":");
                      consumer_conf.push_bind_uri_.append(std::to_string(broker_config::get_next_port()));
                      connection::stream_type stream = connection::stream_zmq;
                      if (req.connection_type_ == "socket") {
                          stream = connection::stream_socket;
                      }
                      consumer_conf.stream_type_ = stream;
                      consumer_conf.socket_connect_type_ = connection::bind_socket;
                      if (!it->second->init_consumer(consumer_conf)) {
                          admin_cmd::common_resp cmd_resp;
                          cmd_resp.cmd_ = req.cmd_;
                          cmd_resp.status_ = STATUS_ERROR;
                          cmd_resp.description_ = "Failed to initialize consumer endpoint for pull stream";
                          std::string resp_str = cmd_resp.to_json();
                          LOG_EVENT("Status response: %s", resp_str.c_str());
                          return reply_cmd(resp_str);
                      }
                  }
                  if (!it->second->get_consumer()) {
                      LOG_ERROR("THis should not happend")
                      admin_cmd::common_resp cmd_resp;
                      cmd_resp.cmd_ = req.cmd_;
                      cmd_resp.status_ = STATUS_ERROR;
                      cmd_resp.description_ = "internal server error";
                      std::string resp_str = cmd_resp.to_json();
                      LOG_EVENT("Status response: %s", resp_str.c_str());
                      return reply_cmd(resp_str);
                  }
                  if (req.type_ == "sub") {
                      resp.bind_uri_ = it->second->get_consumer()->get_pub_bind_uri();
                  } else {
                      resp.bind_uri_ = it->second->get_consumer()->get_push_bind_uri();
                  }
                  utils::replace(resp.bind_uri_, "*", "127.0.0.1");
                  std::string resp_str = resp.to_json();
                  LOG_EVENT("Status response: %s", resp_str.c_str());
                  return reply_cmd(resp_str);
              }
          } else {
              LOG_ERROR("Topic %s not found", req.topic_.c_str());
              admin_cmd::common_resp resp;
              resp.cmd_ = req.cmd_;
              resp.status_ = STATUS_ERROR;
              resp.description_ = STATUS_TOPIC_NOT_FOUND;
              std::string resp_str = resp.to_json();
              LOG_EVENT("Status response: %s", resp_str.c_str());
              return reply_cmd(resp_str);
          }
          LOG_RET("should not come here", -1);
      }

#if 0

        ssize_t reply_to_join_broker(std::string& message, const std::string& cmd) {
            LOG_IN("message: %s", message.c_str());
            std::vector<std::string> tokens = utils::get_tokens(message, ' ');

            std::string broker_id = tokens[1];
            broker_id.append("_");
            broker_id.append(tokens[2]);
            LOG_DEBUG("broker_id: %s", broker_id.c_str());
            broker_config::broker_type broker_type = broker_config::broker_queue;
            if (tokens.size() > 3) {
                if (tokens[3] == "queue") {
                    broker_type = broker_config::broker_queue;
                } else if (tokens[3] == "file") {
                    broker_type = broker_config::broker_file;
                } else if (tokens[3] == "direct") {
                    broker_type = broker_config::broker_direct;
                } else {
                    broker_type = broker_config::broker_queue;
                }
            }
            connection::stream_type stream = connection::stream_zmq;
            if (tokens.size() > 4) {
                if (tokens[4] == "socket") {
                    stream = connection::stream_socket;
                }
            }
            broker_config config;
            config.user_id_ = user_id_;
            config.password_ = password_;
            producer_config prod_conf;
            consumer_config consumer_conf;
            std::map<std::string, broker*>::iterator it = brokers_.find(broker_id);
            std::string response = "JOIN ";
            response.append(broker_id);
            response.append(" ");
            LOG_TRACE("Command is %s", cmd.c_str());
            if (it == brokers_.end()) {
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



                broker *pb = new broker(config);
                if (pb->init(prod_conf, consumer_conf) && pb->run()) {
                    LOG_EVENT("Broker %s started successfully", config.to_string().c_str());
                    brokers_.insert(std::pair<std::string, broker*>(broker_id, pb));
                    if (cmd == "PUB") {
                        response.append(prod_conf.producer_bind_uri_);
                    } else if (cmd == "SUB") {
                        response.append(consumer_conf.pub_bind_uri_);
                    } else {
                        response.append(consumer_conf.push_bind_uri_);
                    }
                    LOG_TRACE("Sending reply: %s", response.c_str());
                    ssize_t result = reply_cmd(response);
                    LOG_RET("", result);
                } else {
                    delete pb;
                    LOG_ERROR("Failed to start broker %s", config.to_string().c_str());
                    LOG_EVENT("Failed to start broker %s", config.to_string().c_str());
                    ssize_t result = reply_cmd(CMD_SERVER_ERROR);
                    LOG_RET("", result);
                }
            } else {

                config = it->second->get_config();
                LOG_EVENT("Reusing existing broker :%s", config.to_string().c_str());
                if (cmd == "PUB") {
                    response.append(it->second->get_producer()->get_bind_uri());
                } else if (cmd == "SUB") {
                    response.append(it->second->get_consumer()->get_pub_bind_uri());
                } else {
                    response.append(it->second->get_consumer()->get_push_bind_uri());
                }
                LOG_TRACE("Sending reply: %s", response.c_str());
                ssize_t result = reply_cmd(response);
                LOG_RET("", result);
            }
            ssize_t result = reply_cmd(CMD_SERVER_ERROR);
            LOG_RET("should not come here", -1);




        }

#endif


  private:

      connection *p_conn_admin_;
      std::string admin_uri_;
      std::string user_id_;
      std::string password_;
      bool stop_;


      std::map<std::string, broker *> brokers_;

      const std::string broker_mgr_topic_ = "lightq_topic";
      const unsigned max_read_buffer_ = 4 * 1024;
      const std::string CMD_INVALID = "invalid_cmd";
      const std::string CMD_PUB = "pub";
      const std::string CMD_SUB = "sub";
      const std::string CMD_PULL = "pull";
      const std::string CMD_UNSUB = "unsub";
      const std::string CMD_DISCONECT = "disconnected";
      const std::string CMD_PING = "ping";
      const std::string CMD_PONG = "pong";
      const std::string CMD_LOGIN = "login";
      const std::string CMD_UNAUTH = "unauthorized_access";
      const std::string CMD_LOGIN_SUCCESS = "login_success";
      const std::string CMD_SERVER_ERROR = "server_error";
      const std::string CMD_STATS = "stats";
      const std::string CMD_JOIN = "join";
      const std::string CMD_CREATE_TOPIC = "create_topic";
      const std::string STATUS_ERROR = "error";
      const std::string STATUS_SUCCESS = "ok";
      const std::string STATUS_TOPIC_NOT_FOUND = "topic not found";
      const std::string STATUS_TOPIC_EXISTS = "topic already exists";
      const std::string STATUS_TOPIC_CREATED = "topic created successfully";


  };
}

#endif	/* BROKER_MANAGER_H */

