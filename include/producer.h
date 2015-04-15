/* 
 * File:   producer.h
 * Author: Rohit Joshi <rohit.c.joshi@gmail.com>
 *
 * Created on February 27, 2015, 10:27 PM
 */

#ifndef PRODUCER_H
#define    PRODUCER_H

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "log.h"
#include "connection.h"
#include "broker_config.h"
//#include "broker.h"
#include "broker_storage.h"
#include "connection_zmq.h"
#include "connection_socket.h"

namespace lightq {
  //class producer
  struct producer_config {
      std::string id_;
      std::string producer_bind_uri_;
      connection::stream_type producer_stream_type_;
      connection::socket_connect_type producer_socket_connect_type_;
  };

  class producer {
  public:

      /**
       * constructor
       */
      producer(broker_storage *pstorage, producer_config &config) : p_storage_(pstorage), config_(config),
                                                                    producer_endpoint_type_(
                                                                        connection::endpoint_type::conn_publisher) {
          LOG_IN("broker_storage: %p, config: %s, pconnection::endpoint_type::conn_publisher",
                 p_storage_, config.producer_bind_uri_.c_str());
          stop_ = false;
          p_producer_socket = NULL;
          LOG_OUT("");
      }

      /**
       * Destructor
       */
      ~producer() {
          LOG_IN("");
          if (producer_tid_.joinable())
              producer_tid_.join();

          delete p_producer_socket;


          LOG_OUT("");
      }

      /**
       * init
       * @return
       */
      bool init() {
          LOG_IN("");
          if (config_.producer_stream_type_ == connection::stream_type::stream_zmq) {
              p_producer_socket = new connection_zmq(
                  config_.id_,
                  config_.producer_bind_uri_,
                  connection::endpoint_type::conn_publisher,
                  connection_zmq::zmq_pull,
                  config_.producer_socket_connect_type_,
                  true, true);
              // connection_zmq* p_zmq_producer = (connection_zmq*)p_producer_socket;
              if (!p_producer_socket->init()) {

                  LOG_RET_FALSE(utils::format_str(
                      "Failed to initialize broker: %s, producer_bind_uri: %s",
                      config_.id_.c_str(), config_.producer_bind_uri_.c_str()).c_str());
              }
          } else if (config_.producer_stream_type_ == connection::stream_socket) {

              p_producer_socket = new connection_socket(
                  config_.id_,
                  config_.producer_bind_uri_,
                  connection::endpoint_type::conn_publisher,
                  connection::bind_socket,
                  true);

              connection_socket *psocket = (connection_socket *) p_producer_socket;

              if (!psocket->init(p_storage_)) {

                  LOG_RET_FALSE(utils::format_str(
                      "Failed to initialize broker: %s, consumer_bind_uri: %s",
                      config_.id_.c_str(), config_.producer_bind_uri_.c_str()).c_str());

              } else {

                  if (!p_producer_socket->run()) {
                      LOG_RET_FALSE(utils::format_str(
                          "Failed to run consumer broker: %s, consumer_bind_uri: %s",
                          config_.id_.c_str(),
                          config_.producer_bind_uri_.c_str()).c_str());
                  }
              }
          } else {
              throw std::runtime_error("producer::init():Not implemented");
          }
          LOG_RET_TRUE("");
      }




      /**
       *
       * @return
       */
      bool run() {
          LOG_IN("");
          producer_tid_ = std::thread(
              [&]() {
                  process_producers();
              });
          LOG_RET_TRUE("");
      }
      /*
      ssize_t process_socket_producers() {
          connection_socket* psocket = (connection_socket*)p_producer_socket;
          int fdset_size = 0;
          char buffer[utils::max_msg_size];
          while(!stop_) { //FIXME:  If no producer connected, this will be in loop
               std::vector<int> active_fds = psocket->get_active_fds();
               if(!active_fds.size()) {
                   utils::sleep_ms(utils::queue_poll_wait);
                   continue;
               }
               fd_set read_fd_set;
               FD_ZERO(&read_fd_set);
               for(unsigned i= 0; i < active_fds.size(); ++i) {
                   FD_SET(active_fds[i], &read_fd_set);
                   if(active_fds[i] > fdset_size) {
                     fdset_size =  active_fds[i];
                   }
               }
               if (select(fdset_size, &read_fd_set, NULL, NULL, NULL) < 0) {
                  LOG_ERROR("Failed to select on read fd");
                  continue;
              }
                for (int i = 0; i < FD_SETSIZE; ++i) {
                    int fd = active_fds[i];
                  if (FD_ISSET(fd, &read_fd_set)) {
                      buffer[0] = '\0';
                      ssize_t bytes_read = utils::read_size(fd, true);
                      if(bytes_read < 0) {
                          psocket->remove_fd(i);
                          continue;
                      }else if(bytes_read == 0) {
                          continue;
                      }
                      bytes_read = utils::read_buffer(buffer, utils::max_msg_size,bytes_read);
                      if(bytes_read < 0) {
                          psocket->remove_fd(i);
                          continue;
                      }
                      if (!p_storage_->add_to_storage(buffer, bytes_read, true)) {
                          LOG_RET_FALSE("failure");
                      }

                  }
                }


          }


          LOG_RET_TRUE("loop exit");
      }*/



      /**
       * process producers
       * @return
       */
      bool process_producers() {
          LOG_IN("");
          std::string message;
          message.reserve(utils::max_msg_size);
          while (!stop_) {
              message.clear();
              ssize_t bytes_read = p_producer_socket->read_msg(message);
              if (bytes_read < 0) {
                  LOG_ERROR("Failed to read from producer connection id: %s, producer_bind_uri: %s",
                            config_.id_.c_str(), config_.producer_bind_uri_.c_str());
                  LOG_RET_FALSE("failure");
              }
              LOG_DEBUG("Read message with size: %d", bytes_read);
              //if bytes read zero, continue
              if (bytes_read == 0) continue;
              bool write_message_size = true;
              //if producer is socket, we expect producer to have message size included in the payload
              if (p_producer_socket->get_stream_type() == connection::stream_socket) {
                  write_message_size = true;
              }
              if (!p_storage_->add_to_storage(message, write_message_size)) {
                  LOG_RET_FALSE("failure");
              }


          }
          LOG_RET_TRUE("done");
      }

      std::string get_bind_uri() {
          return config_.producer_bind_uri_;
      }

      unsigned get_num_clients() {
          if (p_producer_socket) {
              connection_zmq *psocket = (connection_zmq *) p_producer_socket;
              return psocket->get_num_connected_clients();
          } else {
              return 0;
          }
      }

  private:
      broker_storage *p_storage_;
      producer_config config_;
      connection::endpoint_type producer_endpoint_type_;
      connection *p_producer_socket;
      bool stop_;
      std::thread producer_tid_;


  };
}


#endif	/* PRODUCER_H */

