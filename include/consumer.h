/* 
 * File:   consumer.h
 * Author: Rohit Joshi <rohit.c.joshi@gmail.com>
 *
 * Created on February 27, 2015, 10:29 PM
 */

#ifndef CONSUMER_H
#define    CONSUMER_H

#include "broker_config.h"
#include "connection.h"
#include "broker_storage.h"
#include "connection_socket.h"
#include "connection_zmq.h"

namespace lightq {

  struct consumer_config {
      std::string id_;
      std::string push_bind_uri_;
      std::string pub_bind_uri_;
      connection::stream_type stream_type_;
      connection::socket_connect_type socket_connect_type_;

      /**
       * to string
       * @return
       */
      std::string to_string() {

          std::string str = utils::format_str(
              "id: %s,  push_bind_uri: %s, pub_bind_uri: %s, stream_type: %d",
              id_.c_str(), push_bind_uri_.c_str(), pub_bind_uri_.c_str(),
              stream_type_);

          // std::string str (buffer, strlen(buffer));
          LOG_DEBUG("config.to_string(): %s", str.c_str());
          return std::move(str);

      }
  };

  class consumer {
  public:

      /**
       * constructor
       */
      consumer(broker_storage *pstorage, consumer_config &config) :
          p_storage_(pstorage), config_(config),
          consumer_endpoint_type_(connection::endpoint_type::conn_consumer) {
          LOG_IN("broker_storage: %p, config: %s", p_storage_, config_.to_string().c_str());
          stop_ = false;
          p_consumer_socket_ = NULL;
          p_consumer_pub_socket = NULL;
          running_ = false;
          LOG_OUT("");
      }

      /**
       * destructor
       */
      ~consumer() {
          LOG_IN("");
          if (consumer_tid_.joinable())
              consumer_tid_.join();

          delete p_consumer_socket_;
          delete p_consumer_pub_socket;
          LOG_OUT("");
      }

      /**
       * init
       * @return
       */
      bool init() {
          LOG_IN("");
          if (config_.stream_type_ == connection::stream_zmq) {
              if (!config_.push_bind_uri_.empty()) {
                  p_consumer_socket_ = new connection_zmq(
                      config_.id_, config_.push_bind_uri_,
                      consumer_endpoint_type_,
                      connection_zmq::zmq_push,
                      connection::bind_socket,
                      false,
                      true);

                  if (!p_consumer_socket_->init()) {

                      LOG_RET_FALSE(utils::format_str(
                          "Failed to initialize broker: %s, consumer_bind_uri: %s",
                          config_.id_.c_str(), config_.push_bind_uri_.c_str()).c_str());
                  }
              }
              if (!config_.pub_bind_uri_.empty()) {
                  p_consumer_pub_socket = new connection_zmq(
                      config_.id_, config_.pub_bind_uri_,
                      consumer_endpoint_type_,
                      connection_zmq::zmq_pub,
                      connection::bind_socket,
                      true,
                      true);

                  if (!p_consumer_pub_socket->init()) {

                      LOG_RET_FALSE(utils::format_str(
                          "Failed to initialize broker: %s, consumer_bind_uri: %s",
                          config_.id_.c_str(), config_.pub_bind_uri_.c_str()).c_str());
                  }
              }
          } else if (config_.stream_type_ == connection::stream_socket) {

              p_consumer_socket_ = new connection_socket(
                  config_.id_,
                  config_.push_bind_uri_,
                  consumer_endpoint_type_,
                  connection::bind_socket,
                  true);
              connection_socket *psocket = (connection_socket *) p_consumer_socket_;
              if (!psocket->init(p_storage_)) {

                  LOG_RET_FALSE(utils::format_str(
                      "Failed to initialize broker: %s, consumer_bind_uri: %s",
                      config_.id_.c_str(), config_.push_bind_uri_.c_str()).c_str());

              } else {
                  if (!p_consumer_socket_->run()) {
                      LOG_RET_FALSE(utils::format_str(
                          "Failed to run consumer broker: %s, consumer_bind_uri: %s",
                          config_.id_.c_str(), config_.push_bind_uri_.c_str()).c_str());
                  }
              }
          } else {
              throw std::runtime_error("broker::init():Not implemented");
          }
          LOG_RET_TRUE("");
      }

      static int process_fds(int fd) {
          LOG_IN("fd: %d", fd);
          LOG_RET_TRUE("");
      }

      /**
       * run
       * @return
       */
      bool run() {
          LOG_IN("");
          if (running_) {
              return running_;
          }
          running_ = true;
          consumer_tid_ = std::thread(
              [&]() {
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
          message.reserve(utils::max_msg_size);
          while (!stop_) {
              if (p_storage_->get_broker_type() == broker_config::broker_file &&
                  p_consumer_socket_->get_stream_type() == connection::stream_type::stream_socket) {
                  connection_socket *psocket = (connection_socket *) p_consumer_socket_;
                  if (psocket->is_consumer_pull_messages()) {
                      LOG_INFO("Consumer is socket and directly pulling messages from file.");
                      LOG_OUT("");
                  }
              }
              //if no messages to consume, sleep

              message.clear();
              bool result = false;

              //based on consumer socket type, either use send file or write buffer
              if (p_storage_->get_broker_type() == broker_config::broker_file ||
                  p_storage_->get_broker_type() == broker_config::broker_queue_file) {
                  if (p_consumer_socket_->get_stream_type() == connection::stream_type::stream_socket) {
                      connection_socket *psocket = (connection_socket *) p_consumer_socket_;
                      while (p_storage_->get_file_total_bytes_written() <
                             psocket->get_write_offset() + sizeof(uint32_t)) {
                          utils::sleep_ms(utils::queue_poll_wait); //define magic number fixme
                      }
                      LOG_DEBUG("Data are available for read");
                      result = sendfile_to_socket();

                  } else {
                      while (p_storage_->get_file_total_bytes_written() <=
                             p_storage_->get_total_bytes_read() + sizeof(uint32_t)) {
                          utils::sleep_ms(
                              utils::queue_poll_wait); //define magic number fixme may be implement condition variabl
                      }
                      LOG_TRACE("file_total_bytes_written[%lld], file_total_bytes_read[%lld]",
                                p_storage_->get_file_total_bytes_written(), p_storage_->get_total_bytes_read());
                      result = p_storage_->file_to_consumer(p_consumer_socket_, false);

                  }

              } else if (p_storage_->get_broker_type() == broker_config::broker_queue) {
                  while (p_storage_->get_queue_size() <= 0) {
                      utils::sleep_ms(utils::queue_poll_wait); //define magic number fixme
                  }

                  result = p_storage_->get_message_from_queue(message);
                  if (result > 0) {
                      //write to push socket
                      if (p_consumer_socket_) {
                          connection_zmq *psocket = (connection_zmq *) p_consumer_socket_;
                          LOG_TRACE("number of connected pull clients: %u", psocket->get_num_connected_clients());
                          if (psocket->get_num_connected_clients() > 0) {
                              result = psocket->write_msg(message.c_str(), message.length());
                          } else {
                              LOG_DEBUG("No clients are connected to push socket. Not sending message");
                          }
                      }
                      //write to pub socket
                      if (p_consumer_pub_socket) {
                          connection_zmq *psocket = (connection_zmq *) p_consumer_pub_socket;
                          LOG_TRACE("number of connected pub clients: %u", psocket->get_num_connected_clients());
                          if (psocket->get_num_connected_clients() > 0) {
                              result = psocket->write_msg(message.c_str(), message.length());
                          } else {
                              LOG_DEBUG("No clients are connected to pub socket. Not sending message");
                          }
                      }
                  }
              }

              if (result == 0) {
                  utils::sleep_ms(utils::queue_poll_wait);
              }
          }
          LOG_OUT("");
      }

      /**
       * send file to socket
       * @param p_consumer_socket
       * @return
       */
      ssize_t sendfile_to_socket(uint32_t bytes_to_send = utils::max_msg_size) {
          LOG_IN("");
          ssize_t result = 0;
          connection_socket *psocket = (connection_socket *) p_consumer_socket_;
          int socket_fd = psocket->get_next_fd();
          LOG_DEBUG("received socket fd: %d", socket_fd);

          if (socket_fd < 0) {
              LOG_ERROR("No consumer to send data");
              LOG_RET("no consumer to send data", 0);
          }

          uint64_t offset = psocket->get_write_offset();

          LOG_DEBUG("Write offset: %llu", offset);
          result = -1;
          while (result < 0) {
              result = p_storage_->get_file_connection()->send_file(socket_fd, offset, bytes_to_send);
              if (result == -1) {
                  LOG_ERROR("Failed to read file for socket fd:%d", socket_fd);
                  psocket->remove_fd(socket_fd);
                  break;
              } else {
                  LOG_DEBUG("Read %u bytes", result);
                  psocket->set_write_offset(offset + result);
                  p_storage_->add_total_bytes_read(result);
              }
          }
          LOG_RET("", result);
      }

      /**
       * send file to socket
       * @param p_consumer_socket
       * @return
       */
      ssize_t sendfile_to_socket(int socket_fd, uint32_t offset, uint32_t bytea_to_send = utils::max_msg_size) {
          LOG_IN("socket_fd[%d], offset[%u], bytea_to_send[%u]",
                 socket_fd, offset, bytea_to_send);
          ssize_t result = 0;
          connection_socket *psocket = (connection_socket *) p_consumer_socket_;
          result = -1;
          while (result < 0) {
              result = p_storage_->get_file_connection()->send_file(socket_fd, offset, bytea_to_send);
              if (result == -1) {
                  LOG_ERROR("Failed to read file for socket fd:%d", socket_fd);
                  psocket->remove_fd(socket_fd);
                  break;
              } else {
                  LOG_DEBUG("Read %u bytes", result);
                  psocket->set_write_offset(offset + result);
                  p_storage_->add_total_bytes_read(result);
              }
          }
          LOG_RET("", result);
      }

      /**
       * get_consumer_socket
       * @return
       */
      connection *get_consumer_socket() {
          return p_consumer_socket_;
      }

      std::string get_pub_bind_uri() {
          return config_.pub_bind_uri_;
      }

      std::string get_push_bind_uri() {
          return config_.push_bind_uri_;
      }

      unsigned get_num_pub_clients() {
          if (p_consumer_pub_socket) {
              connection_zmq *psocket = (connection_zmq *) p_consumer_pub_socket;
              return psocket->get_num_connected_clients();
          } else {
              return 0;
          }
      }

      unsigned get_num_pull_clients() {
          if (p_consumer_socket_) {
              if (config_.stream_type_ == connection::stream_zmq) {
                  connection_zmq *psocket = (connection_zmq *) p_consumer_socket_;
                  return psocket->get_num_connected_clients();
              } else if (config_.stream_type_ == connection::stream_socket) {
                  connection_socket *psocket = (connection_socket *) p_consumer_socket_;
                  return psocket->get_total_connected_clients();
              }
          } else {
              return 0;
          }
          return 0;
      }

  private:
      broker_storage *p_storage_;
      consumer_config config_;
      connection::endpoint_type consumer_endpoint_type_;
      connection *p_consumer_socket_;
      connection *p_consumer_pub_socket; //zqm only
      bool stop_;
      std::thread consumer_tid_;
      bool running_;

  };
}


#endif	/* CONSUMER_H */

