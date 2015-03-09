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
#include "connection_file.h"

namespace lightq {
    class broker;

    class broker_storage {
    public:

        broker_storage(broker_config& config) : config_(config),
        total_enqueued_messages_(0), total_dequeued_messages_(0){
            p_consumer_socket_ = NULL;
        }

        bool init(broker_config& config_) {
            LOG_IN("config [%p]", &config_);
            //initialize broker storage
            if (config_.broker_type_ == broker_config::broker_queue) {
                p_queue_ = new moodycamel::ReaderWriterQueue<std::string>(config_.default_queue_size_);
                LOG_RET_TRUE("success");
            } else if (config_.broker_type_ == broker_config::broker_file) {

                p_file = new connection_file(config_.output_directory_, config_.id_, "", connection::conn_broker, true);
                LOG_RET_TRUE("success");
            }
            LOG_RET_FALSE("Not supported");
        }

        /**
         * add to broker
         * @param message
         * @return 
         */
        bool add_to_storage(const std::string& message, bool write_size = true) {
            LOG_IN("message: [%s],  message length: [%d], write_size[%d]",
                    message.c_str(), message.length(), write_size);
            if (config_.broker_type_ == broker_config::broker_direct) {
                LOG_DEBUG("Broker type is direct");
                return direct_write_consumer(message);

            } else if (config_.broker_type_ == broker_config::broker_queue) {
                LOG_DEBUG("Broker type is queue");
                return write_to_queue(message);
            }
            else if (config_.broker_type_ == broker_config::broker_file) {
                LOG_DEBUG("Broker type is file");

            }
            LOG_RET_FALSE("failed");

        }

        /**
         * read file and send to socket
         * @return 
         */
        ssize_t file_to_consumer(connection* p_consumer_socket) {
            LOG_IN("");
            assert(p_file);
            ssize_t result = 0;
            std::string message;
            result = p_file->read_msg(message);
            if (result < 0) {
                    LOG_ERROR("Failed to read from the file : %s", p_file->get_current_file().c_str());
                    LOG_RET_FALSE("Failed to read from file")
            }
            if (result > 0) {
                result = p_consumer_socket->write_msg(message);
            }
            
            if (result >= 0) {
                LOG_RET("success", result);
            } else {
                LOG_ERROR("Failed to write to the consumer socket: %s", p_consumer_socket->topic().c_str());
                LOG_RET("Failed to write to the consumer socket", result)
            }

        }
        
        /**
         * send file to socket
         * @param p_consumer_socket
         * @return 
         */
        ssize_t sendfile_to_socket(connection* p_consumer_socket) {
            LOG_IN("");
            ssize_t result = 0;
            connection_socket* psocket = (connection_socket*) p_consumer_socket;
            int socket_fd = psocket->get_next_fd();
            LOG_DEBUG("received socket fd: %d", socket_fd);
            
            uint32_t offset = psocket->get_write_offset();
           
            LOG_DEBUG("Write offset: %u", offset);
            result = 0;
            while (result < 0) {
                result = p_file->send_file(socket_fd, offset, config_.max_message_size);
                if (result == -1) {
                    psocket->remove_fd(socket_fd);
                    break;
                } else {
                    psocket->set_write_offset(offset + result);
                }
            }
            LOG_RET("", result);
        }


        /**
         * writ
         * @return 
         */
        ssize_t get_message_from_queue(std::string& message) {
            //  LOG_IN("");
            assert(p_queue_);
            ssize_t result = 0;
            if (p_queue_->try_dequeue(message)) {
                LOG_DEBUG("Dequeue message :%s", message.c_str());
                ++total_dequeued_messages_;
                LOG_RET("success", message.length());
            }
            LOG_RET("", result);

        }

        inline uint64_t get_total_dequeued_messages() {
            return total_dequeued_messages_;
        }

        inline uint64_t get_total_enqueued_messages() {
            return total_enqueued_messages_;
        }

        inline uint64_t get_queue_size() {
          //  LOG_IN("");
           // LOG_RET("%lld", );
            return total_enqueued_messages_ - total_dequeued_messages_;
        }

        inline uint64_t get_queue_size_approx() {
            if (config_.broker_type_ == broker_config::broker_queue) {
                return p_queue_->size_approx();
            } else {
                return 0;
            }
        }

        inline void set_consumer_socket(connection *p_socket) {
            LOG_IN("p_socket[%p]", p_socket);
            p_consumer_socket_ = p_socket;
            LOG_OUT("");
        }

        inline broker_config::broker_type get_broker_type() {
            return config_.broker_type_;
        }
        
        inline uint64_t get_file_total_bytes_written() {
            return p_file->get_total_bytes_writen();
        }
       
    private:

        bool direct_write_consumer(const std::string& message) {
            LOG_IN("");
            if (p_consumer_socket_ == NULL) {
                LOG_ERROR("Consumer socket must be set for broker type direct");
                LOG_RET_FALSE("invalid initialization");
            }
            ssize_t bytes_written = p_consumer_socket_->write_msg(message);
            if (bytes_written < 0) {
                LOG_ERROR("Failed to write to consumer connection id: %s, consumer_bind_uri: %s",
                        config_.id_.c_str(), p_consumer_socket_->get_resource_uri_().c_str());
                LOG_RET_FALSE("failure");
            }
            LOG_RET_TRUE("success");
        }

        bool write_to_queue(const std::string& message) {
            LOG_IN("message: %u", message.length());

            while (!p_queue_->try_enqueue(message)) { 
                s_sleep(3);// 
                LOG_TRACE("Retrying to enqueue message");
            } 
            ++total_enqueued_messages_;
            LOG_DEBUG("message  enqueue. Total messages in the queue: %lld ", total_enqueued_messages_);
            LOG_RET_TRUE("enqueued message");
        }

        bool write_to_file(std::string& message, bool write_size) {
            ssize_t bytes_written = p_file->write_to_file(message, write_size);
            if (bytes_written > 0) {
                LOG_INFO("message  written to file ");
                LOG_DEBUG("%d bytes written to file", bytes_written);
                LOG_RET_TRUE("success")
            } else {
                LOG_ERROR("Failed to write to file");
            }
        }
        


        broker_config config_;

        moodycamel::ReaderWriterQueue<std::string> * p_queue_;
        connection_file *p_file;
        connection* p_consumer_socket_;
        uint64_t total_enqueued_messages_;
        uint64_t total_dequeued_messages_;
       


    };
}


#endif	/* BROKER_STORAGE_H */

