/* 
 * File:   connection_file.h
 * Author: Rohit Joshi <rohit.c.joshi@gmail.com>
 *
 * Created on February 25, 2015, 2:48 PM
 */

#ifndef CONNECTION_FILE_H
#define	CONNECTION_FILE_H

#include <cstdio>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#ifdef __APPLE__
#include <sys/socket.h>
#include <sys/uio.h>
#else
#include <sys/sendfile.h>
#endif
#include "connection.h"
#include "file_details.h"

namespace lightq {

    //class connection type file

    class connection_file :  public connection {
    public:

        /**
         * constructor
         * @param filepath_prefix
         * @param topic
         * @param uri
         * @param conn_type
         */
        connection_file(const std::string& directory, 
                const std::string& topic,
                const std::string& uri, 
                endpoint_type conn_type,
                bool non_blocking= true)
        : connection(topic, uri, connection::stream_type::stream_file, 
                conn_type, 
                connection::socket_connect_type::create_file,
                non_blocking) {
            LOG_IN("directory: %s", directory_.c_str());
            directory_ = directory;
            current_fd_index_ = 0;
            total_bytes_writen_ = 0;
            msg_counter_ = 0;
            max_file_size_ = 2000000000;
            file_fds_.reserve(10);
            
        }
        /**
         * init
         * @return 
         */
         bool init() {
             LOG_IN("");
             LOG_RET_TRUE("success");
         }
         
         /**
         * run
         * @return 
         */
         bool run() {
             LOG_IN("");
             LOG_RET_TRUE("success");
         }

        /**
         * Destructor
         */
        ~connection_file() {
            LOG_IN("");
            close_all();
            for (unsigned i = 0; i < file_fds_.size(); ++i) {
                if (file_fds_[i]) {
                    delete file_fds_[i];
                    file_fds_[i] = NULL;
                }
            }
            LOG_OUT("");
        }

        /**
         * get total bytes written
         * NOTE: you can use this and increment.  It is returned as integer value
         * @return 
         */
        inline uint64_t get_total_bytes_writen() const {
            LOG_IN("");
            LOG_RET("", (unsigned long long)total_bytes_writen_);
        }

        /**
         * set max file size
         * @param size
         */
        inline void set_max_file_size(uint64_t size) {
            LOG_IN("");
            max_file_size_ = size;
            LOG_OUT("");
        }

        /**
         * get message counter
         * @return 
         */
        inline uint64_t get_msg_counter() {
            LOG_IN("");
            LOG_RET("", msg_counter_);
        }

        /**
         * Read buffer from given offset
         * @param buffer
         * @param size_of_buffer
         * @param offset
         * @param ntohl
         * @return 
         */
        ssize_t read(char* buffer, unsigned size_of_buffer, uint64_t offset, bool ntohl = false) {
            LOG_IN("buffer: %p, size_of_buffer :%u, offset:%lld, ntohl: %d",
                    buffer, size_of_buffer, offset, ntohl);

            for (unsigned i = 0; i < file_fds_.size(); ++i) {
                if (file_fds_[i]->offset_across_all_files_ < offset) {
                    continue; //offset is larger than total bytes written to this file. move to next

                }

                ssize_t bytes_read = file_fds_[i]->read_msg(buffer, size_of_buffer, offset, ntohl);
                if (bytes_read > 0) {
                    LOG_RET("Success", bytes_read);
                }
            }
            LOG_RET("Error", -1);
        }

        /**
         * write string to the file
         * @param msg
         * @return 
         */
        ssize_t write_to_file(const std::string& msg, bool write_msg_size=true) {
            LOG_IN("msg: %s", msg.c_str());
            if(!set_current_file()) {
                LOG_RET("failed", -1);
            }
            bool include_offset = false;
            if(write_msg_size) {
                include_offset = true;
            }
            int bytes_written = file_fds_[current_fd_index_]->write_msg(msg, write_msg_size, include_offset);
            if (bytes_written > 0) {
                msg_counter_++;
                total_bytes_writen_ += bytes_written;
                file_fds_[current_fd_index_]->offset_across_all_files_ = total_bytes_writen_;
                LOG_RET("Success: ", bytes_written);
            }
             LOG_RET("Error: ", bytes_written);
        }
        
        /**
         * write string to the file
         * @param msg
         * @return 
         */
        ssize_t write_to_file(std::string&& msg, bool write_msg_size=true) {
            LOG_IN("msg: %s", msg.c_str());
            set_current_file();
            bool include_offset = false;
            if(write_msg_size) {
                include_offset = true;
            }
            int bytes_written = file_fds_[current_fd_index_]->write_msg(msg, write_msg_size, include_offset );
            if (bytes_written > 0) {
                msg_counter_++;
                total_bytes_writen_ += bytes_written;
                file_fds_[current_fd_index_]->offset_across_all_files_ = total_bytes_writen_;
                LOG_RET("Success: ", bytes_written);
            }
             LOG_RET("Error: ", bytes_written);
        }

        /**
         * Write array of string to the file
         * @param values
         * @param size
         * @return 
         */
        bool write_to_file(const std::string *values, unsigned size) {
            LOG_IN("values :%p, size:%u", values, size);
            std::string buffer;
            buffer.reserve(size * 1024);
            for (unsigned i = 0; i < size; ++i) {
                buffer.append(values[i]);
            }
            if (!write_to_file(buffer)) {
                LOG_RET_FALSE("Error");
            }
            LOG_RET_TRUE("Success");
        }
        /**
         * send file
         * @param fd
         * @param offset
         * @param size
         * @return 
         */
        ssize_t send_file(int fd, uint64_t offset, uint32_t size) {
            LOG_IN("fd[%d], offset[%u], size[%u]", fd, offset, size);
            for (unsigned i = 0; i < file_fds_.size(); ++i) {
                LOG_DEBUG("i[%d], offset[%u]", i, offset);
                if (file_fds_[i]->offset_across_all_files_ < offset) {
                    LOG_DEBUG("Skipping the fd index [%d]", i);
                    continue; //offset is larger than total bytes written to this file. move to next
                }
                ssize_t bytes_read = file_fds_[i]->send_file(fd, offset, size);
                if (bytes_read > 0) {
 
                    LOG_RET("Success", bytes_read);
                }else {
                    LOG_RET("failed", -1);
                }
            }
           LOG_RET("", 0);
        }
        
        ssize_t read_msg(std::string& message) {
             LOG_IN("message [%s]", message.c_str());
           throw std::runtime_error("connection_file::read_msg():not implemented");
           LOG_RET("", -1);
        }
       
       ssize_t write_msg(const std::string& message) {
           LOG_IN("message [%s]", message.c_str());
           throw std::runtime_error("connection_file::write_msg():not implemented");
           LOG_RET("", -1);
        }
        const std::string& get_current_file() const {
           LOG_IN("");
           throw std::runtime_error("connection_file::get_current_file():not implemented");
           
        }

        void close_all() {
            LOG_IN("");
            for (unsigned i = 0; i < file_fds_.size(); ++i) {
                file_fds_[i]->close();
            }
            LOG_OUT("");
        }


    private:

        std::string directory_;
        std::vector<file_details*> file_fds_;
        unsigned current_fd_index_;
        uint64_t max_file_size_;
        std::atomic<uint64_t> total_bytes_writen_; //FIXME: Do we need as atomic
        uint64_t msg_counter_;


        /**
         * Set and possibly create a file
         * @return 
         */
        bool set_current_file() {
            LOG_IN("");
             LOG_DEBUG("current file fd size: %u", file_fds_.size());
            if (file_fds_.size() == 0) {
               
                file_details *pInfo = new file_details();
                LOG_DEBUG("Creating a file");
                if (pInfo->create_file(directory_,topic_, current_fd_index_)) {
                    LOG_DEBUG("File created successfully");
                    file_fds_.push_back(pInfo);
                    LOG_RET_TRUE("Success");
                }else {
                    LOG_ERROR("Failed to create a file for index :%u", current_fd_index_)
                }
                LOG_RET_FALSE("Failed to create file");
            }
             LOG_DEBUG("File already exist.");
            unsigned fd_to_use = total_bytes_writen_ / max_file_size_;
            LOG_INFO("fd_to_use: %d, current_fd_index_:% u ", fd_to_use, current_fd_index_);
            if (fd_to_use > current_fd_index_) {
                LOG_INFO("fd_to_use: %d", fd_to_use);
                file_fds_[current_fd_index_]->offset_across_all_files_ = total_bytes_writen_;
                // file_fds_[current_fd_index_]->close(); we need to provide support to read

                current_fd_index_ = fd_to_use;
                file_details *pInfo = new file_details();
                if (pInfo->create_file(directory_, topic_,current_fd_index_)) {
                    file_fds_.push_back(pInfo);
                    LOG_RET_TRUE("Success");
                }
            }
            LOG_RET_TRUE("Success");

        }



    };

    
}

#endif	/* CONNECTION_FILE_H */

