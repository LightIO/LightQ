/* 
 * File:   broker_storage.h
 * Author: Rohit Joshi <rohit.c.joshi@gmail.com>
 *
 * Created on February 28, 2015, 11:37 AM
 */

#ifndef BROKER_STORAGE_H
#define	BROKER_STORAGE_H

#include "broker_config.h"
#include "thirdparty/readerwriterqueue.h"
#include "connection_socket.h"

namespace prakashq {
    class broker;

    class broker_storage {
    public:

        broker_storage(broker_config& config) : config_(config),total_msg_received_(0),total_msg_sent_(0) {
        }

        bool init(broker_config& config_) {
            //initialize broker storage
            if (config_.broker_type_ == broker_config::broker_queue ||
                    config_.broker_type_ == broker_config::broker_queue_file ||
                    config_.broker_type_ == broker_config::broker_file_queue) {
                p_queue_ = new moodycamel::ReaderWriterQueue<std::string>(config_.default_queue_size_);
            }
            if (config_.broker_type_ == broker_config::broker_file ||
                    config_.broker_type_ == broker_config::broker_queue_file ||
                    config_.broker_type_ == broker_config::broker_file_queue) {
                p_file = new connection_file(config_.output_directory_, config_.id_, "", connection::conn_broker);
            }
        }

        /**
         * add to broker
         * @param message
         * @return 
         */
        bool add_to_storage(const std::string& message, bool write_size=true) {
            LOG_IN("message: [%s],  message length: [%d], write_size[%d]",
                    message.c_str(), message.length(), write_size);
            /* if (config_.broker_type_ == broker_config::broker_direct) {
                 ssize_t bytes_written = p_consumer_socket->write(message);
                 if (bytes_written < 0) {
                     LOG_ERROR("Failed to write to consumer  connection id: %s, consumer_bind_uri: %s",
                             id_.c_str(), consumer_bind_uri_.c_str());
                     LOG_RET_FALSE("failure");
                 }
                 LOG_RET_TRUE("success");

             } else*/
             ++total_msg_received_;
            if (config_.broker_type_ == broker_config::broker_queue) {
                LOG_TRACE("Broker type is queue");
                if (!p_queue_->try_enqueue(message)) {
                    LOG_EVENT("Failed to queue message. Queue size is  %lld. Retrying...", p_queue_->size_approx());
                    while (!p_queue_->try_enqueue(message));
                    
                    LOG_INFO("message  enqueue ");
                    LOG_RET_TRUE("enqueued message");
                } else {
                    LOG_RET_TRUE("enqueued message");
                }
            } else if (config_.broker_type_ == broker_config::broker_queue_file) {
                LOG_TRACE("Broker type is broker_queue_file");
                if (!p_queue_->try_enqueue(message)) {
                    LOG_ERROR("Failed to queue message. Queue size is  %lld. Retrying...", p_queue_->size_approx());
                    while (!p_queue_->try_enqueue(message));
                }
                LOG_INFO("message  enqueue ");
                ssize_t bytes_written = p_file->write_to_file(message, write_size);
                if (bytes_written >= 0) {
                    LOG_INFO("message  written to file ");
                    LOG_TRACE("%d bytes written to file", bytes_written);
                    LOG_RET_TRUE("success")
                } else {
                    LOG_ERROR("broker_queue_file: Failed to write to file");
                }
                LOG_RET_TRUE("enqueued and written to file message");

            } else if (config_.broker_type_ == broker_config::broker_file_queue) {
                LOG_TRACE("Broker type is broker_file_queue");
                ssize_t bytes_written = p_file->write_to_file(message, write_size);
                if (bytes_written >= 0) {
                    LOG_INFO("message  written to file ");
                    LOG_TRACE("%d bytes written to file", bytes_written);
                } else {
                    LOG_ERROR("Failed to write to file");
                }
                if (!p_queue_->try_enqueue(message)) {
                    LOG_ERROR("Failed to queue message. Queue size is  %lld. Retrying...", p_queue_->size_approx());
                    while (!p_queue_->try_enqueue(message));
                    LOG_INFO("message  enqueue ");

                    LOG_RET_TRUE("enqueued and written to file message");
                } else {
                    LOG_RET_TRUE("enqueued and written to file message");
                }
            } else {
                LOG_TRACE("Broker type is file");
                ssize_t bytes_written = p_file->write_to_file(message,write_size);
                if (bytes_written >= 0) {
                    LOG_INFO("message  written to file ");
                    LOG_TRACE("%d bytes written to file", bytes_written);
                    LOG_RET_TRUE("success")
                } else {
                    LOG_ERROR("Failed to write to file");
                }
            }
            LOG_RET_FALSE("failed");

        }

        /**
         * read file and send to socket
         * @return 
         */
        ssize_t file_to_consumer(bool send_file, connection* p_consumer_socket) {
            LOG_IN("");
          //  assert(p_file);
            ssize_t result = 0;
            if (send_file && p_consumer_socket->get_stream_type() == connection::stream_type::stream_socket) {
                    connection_socket* psocket = (connection_socket*)p_consumer_socket;
                    int socket_fd = psocket->get_next_fd();
                    LOG_TRACE("received socket fd: %d", socket_fd);
                    
                    ssize_t offset = utils::read_size(socket_fd, true);
                    LOG_TRACE("Received offset from consumer [%d]", offset);
                    if(offset == -1) {
                        LOG_ERROR("Failed to read offset from consumer");
                        psocket->remove_fd(socket_fd);
                        LOG_RET("", 0);
                    }
                   // uint32_t offset = psocket->get_write_offset();
                    //LOG_TRACE("Write offset: %u", offset);
                    result = 0;
                    while(result <= 0) {
                        result = p_file->send_file(socket_fd, offset, config_.max_message_size);
                        if(result == -1) {
                            psocket->remove_fd(socket_fd);
                            break;
                        }else {
                            psocket->set_write_offset(offset+result);
                        }
                    }
            } else {
                std::string message;
                result = p_file->read_msg(message);
                if (result < 0) {
                    LOG_ERROR("Failed to read from the file : %s", p_file->get_current_file().c_str());
                    LOG_RET_FALSE("Failed to read from file")
                }
                if (result > 0) {
                    result = p_consumer_socket->write(message);
                }
            }
            if (result >= 0) {
                LOG_RET("success", result);
            } else {
                LOG_ERROR("Failed to write to the consumer socket: %s", p_consumer_socket->topic().c_str());
                LOG_RET("Failed to write to the consumer socket", result)
            }

        }

        /**
         * writ
         * @return 
         */
        ssize_t queue_to_consumer(connection* p_consumer_socket) {
            //  LOG_IN("");
            assert(p_queue_);
            std::string message;
            ssize_t result = 0;
            if (p_queue_->try_dequeue(message)) {
                LOG_TRACE("Dequeue message :%s", message.c_str());
                result = p_consumer_socket->write(message);
                if (result >= 0) {
                    ++total_msg_sent_;
                    LOG_RET("success", result);
                } else {
                    LOG_ERROR("Failed to write to the consumer socket: %s", p_consumer_socket->topic().c_str());
                    LOG_RET("Failed to write to the consumer socket", result)
                }
            }
            //  LOG_RET("", result);

        }
        inline uint32_t get_total_msg_sent() {
            return total_msg_sent_;
        }
         inline uint32_t get_total_msg_received() {
            return total_msg_received_;
        }
         inline uint64_t get_queue_size() {
             return p_queue_->size_approx();
         }
    private:
        broker_config config_;
        moodycamel::ReaderWriterQueue<std::string> * p_queue_;
        connection_file *p_file;
        uint32_t  total_msg_received_;
        uint32_t  total_msg_sent_;

    };
}


#endif	/* BROKER_STORAGE_H */

