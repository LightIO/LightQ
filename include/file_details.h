/* 
 * File:   file_details.h
 * Author: Rohit Joshi <rohit.c.joshi@gmail.com>
 *
 * Created on February 25, 2015, 9:37 PM
 */

#ifndef FILE_DETAILS_H
#define	FILE_DETAILS_H

#ifdef __APPLE__
#include <sys/socket.h>
#include <sys/uio.h>
#else
#include <sys/sendfile.h>
#endif
#include "utils.h"

namespace lightq {
    //class connection info
    class file_details {
        friend class connection_file;
    public:

        file_details() {
            LOG_IN("");
            file_name_ = "";
            offset_ = 0;
            write_counter_ = 0;
            fd_ = -1;
            bytes_written_across_all_files_ = 0;
            LOG_OUT("");
        }

        /**
         * destructor
         */
        ~file_details() {
            LOG_IN("");
            
            if (fd_ > 0) {
                close();
            }
            LOG_OUT("");
        }
        /**
         * read message from the file
         * @param buffer
         * @param buf_size
         * @param offset
         * @param ntohl
         * @return 
         */
        ssize_t read_msg(char* buffer, uint32_t buf_size, uint64_t offset, bool ntohl) {
            LOG_IN("%s", utils::format_str("buffer: %p, buf_Size: %d, offset: %llu ntohl: %d", buffer, buf_size, offset, ntohl).c_str());
            
            unsigned size_of_int = sizeof (unsigned);
            ssize_t size = read_buffer_length(buffer, buf_size, offset, ntohl);
            if (size == 0) {
                LOG_RET("Read length of message 0", size);
            }
             if (size < 0) {
                LOG_RET("Failed to read message offset[%u]", offset);
                return -1;
            }
            LOG_TRACE("Read length of message : %d", size);
            //   std::cout << "Reading message with length: " << size << std::endl;
            if (size > buf_size) {
                LOG_ERROR("message size: %d  is larger than buffer size. Dynamic memory allocation not supported", size);
                LOG_RET("Error:Read length of message 0", size);
            }
            ssize_t length = read_buffer(buffer + size_of_int, buf_size - 4, size, offset + size_of_int);
            if (length > 0) {
                LOG_RET("Success",  length + size_of_int);
            } else {
                LOG_RET("Error", size);
            }

        }
        /**
         * send file
         * @param socket
         * @param offset
         * @param size
         * @return 
         */
        ssize_t send_file(int socket, uint64_t offset, uint64_t size) {
            LOG_IN("socket[%d], offset[%llu],  size[%u]", socket, offset, size);
            LOG_DEBUG("Reading %llu  bytes from offset[%llu]", size, offset);
            LOG_DEBUG("Current file offset[ %llu]", offset_);
            if(size > utils::max_msg_size) {
                size = utils::max_msg_size;
            }
            if(size + offset > offset_) {
                size = offset_ - offset;
                LOG_DEBUG("offset [ %llu] + size[%llu] > offset_[%llu]. Setting size as offset_ - offset[%llu] ",
                        offset, size, offset_, size);
            }
            LOG_DEBUG("Reading %llu  bytes from offset[%llu]", size, offset);
            if(size <= 0) {
                LOG_DEBUG("size to read is zero. returning");
                LOG_RET("no read", size);
            }
           
            
            
 #ifdef __APPLE__
            off_t size_offset = size;
            LOG_DEBUG("Reading %llu  bytes from offset[%llu]", size, offset);
           ssize_t result =  sendfile(fd_, socket, offset, &size_offset, NULL, 0);
           if(result == 0 || result == EAGAIN) {
               LOG_DEBUG("Sendfile success, Read number of bytes [%u]", size_offset)
               LOG_RET("success", size_offset);
           }  
           LOG_DEBUG("Reading %llu  bytes from offset[%llu]", size, offset);
           LOG_ERROR("Failed to sendfile. Current fd[%d], socket_fd[%d], errnum[%d] error_desc[%s]",
                   fd_, socket, result, strerror(result));
           LOG_RET("failed", -1);
#else
           ssize_t result = sendfile(socket, fd_, &offset, size);
           if(result >= 0 ) {
               LOG_RET("success", result);
           }else if(result == EAGAIN) {
               LOG_RET("timeout/non-blocking", 0);
           }
           LOG_ERROR("Failed to sendfile. Current fd[%d], socket_fd[%d], errnum[%d] error_desc[%s]",
                   fd_, socket, result, strerror(result));
           
           LOG_RET("failed", -1);
#endif
        }
        /**
         * write message
         * @param msg
         * @return 
         */
        ssize_t write_msg(const std::string& msg, bool include_size=true, bool include_offset=true) {
            LOG_IN("msg: %s", msg.c_str());
            ssize_t length_written = 0;
            if(include_size) {
                uint32_t size = msg.length();
                if(include_offset) {
                    LOG_DEBUG("Write offset is true.  adding sizeof(offset_): %d", sizeof(offset_));
                    size = size + sizeof(offset_); //sizeof(0x0);
                }
                LOG_DEBUG("Writting payload length: %u", size);
                length_written = write_length(size);
                if (length_written <= 0) {
                    LOG_RET("failed", length_written);
                }
                
            }
          
           
            size_t written_buffer = 0;
            if ((written_buffer = write_buffer(msg.c_str(), msg.length())) <= 0) {
                LOG_RET("Error", written_buffer);
            }
           
            unsigned offset_written = 0;
            if (include_offset  && (offset_written = write_offset()) <= 0) {
                LOG_RET("Error", offset_written);
            }
           
             write_counter_++;            
            LOG_RET("Success:", length_written + written_buffer + offset_written);
            
        }
        /**
         * Create a file
         * @param filepath_prefix
         * @param index
         * @return 
         */
        bool create_file(const std::string& directory, const std::string& filename, unsigned index) {
            LOG_IN("directory : %s, filename: %s, index :%u", directory.c_str(), filename.c_str(), index);
            std::string newfile = directory;
            newfile.append("/");
            newfile.append(filename);
            newfile.append("_");
            newfile.append(std::to_string(index));
            newfile.append(".txt");
            LOG_DEBUG("Creating filename :%s", newfile.c_str());
#ifdef __APPLE__
            int flags = O_RDWR | O_CREAT | O_ASYNC | O_NONBLOCK;
#else
            int flags = O_RDWR | O_CREAT | O_ASYNC | O_NONBLOCK | O_NOATIME | O_LARGEFILE;
#endif
          //  int fd = open(newfile.c_str(), flags, 0644);
            int fd = open(newfile.c_str(), flags, 0644);
            
            if (fd < 0) {
               
                LOG_ERROR("Failed to create file[%s].  Error[%d], error description[%s]" , 
                        newfile.c_str(), errno, strerror(errno));
              
                return false;
            } else {
                LOG_EVENT("File[%s] is created.", newfile.c_str());
                LOG_DEBUG("File: %s created.",  newfile.c_str()); 
            }
           

            fd_ = fd;
            file_name_ = newfile;
            offset_ = 0;
            LOG_RET_TRUE("success");

        }
        /**
         * Close file descriptor
         */
        void close() {
            LOG_IN("")
#ifdef __APPLE__
            fsync(fd_);
#else
            fdatasync(f.fd_);
#endif
            ::close(fd_);
            fd_ = -1;
            LOG_EVENT("File[%s] is closed", file_name_.c_str());
            LOG_OUT("");
        }
        
         /**
         * Flush file descriptor
         */
        void flush() {
            LOG_IN("")
#ifdef __APPLE__
            fsync(fd_);
#else
            fdatasync(f.fd_);
#endif
            LOG_EVENT("File[%s] is flushed to disk", file_name_.c_str());
            LOG_OUT("");
        }

      
    private:

        int fd_;
        //this track offset for total bytes written across all the files
        //e.g if 9 bytes written across 3 files, first fd_details will have 3, 2nd will have 6 and 3rd will have 9
        uint64_t bytes_written_across_all_files_; 
        std::string file_name_;
        uint64_t offset_;
        uint32_t write_counter_;

        /**
         * write buffer to the file
         * @param buffer
         * @param size
         * @return 
         */
        ssize_t write_buffer(const char* buffer, unsigned size) {
            LOG_IN("buffer: %p, size: %u", buffer, size);
            unsigned remaning = size;
            ssize_t bytes_written = 0;
            while (remaning > 0) {
                ssize_t result = pwrite(fd_, buffer + bytes_written, remaning, offset_);
                if (result < 0) {
                    LOG_ERROR("Failed to write data to file. Total messages written: %ld", write_counter_ );
                    if (bytes_written > 0) {
                        LOG_RET("Success", bytes_written);
                    }
                    LOG_RET("Error:", -1);
                }
                remaning -= result;
                bytes_written += result;
                offset_ += result;
            }
            LOG_RET("Total bytes written",  bytes_written);
        }

        /**
         * Write the length
         * @param length
         * @return 
         */
        ssize_t write_length(unsigned length) {
            LOG_IN("length: %u", length);
            unsigned remaning = sizeof (length);
            char* buf = (char*) &length;
            ssize_t bytes_written = 0;
            while (remaning > 0) {
                ssize_t result = pwrite(fd_, buf + bytes_written, remaning, offset_);
                if (result < 0) {
                   LOG_ERROR("Failed to write data length to file. Total messages written: %ld",  write_counter_ );
                    return false;
                }
                remaning -= result;
                bytes_written += result;
                offset_ += result;

            }
            LOG_RET("Success", bytes_written);
        }
        /**
         * Write the length
         * @param length
         * @return 
         */
        ssize_t write_offset() {
            LOG_IN("");
            unsigned remaning = sizeof (offset_);
            uint64_t newoffset = offset_;
            char* buf = (char*) &newoffset;
            ssize_t bytes_written = 0;
            while (remaning > 0) {
                ssize_t result = pwrite(fd_, buf + bytes_written, remaning, offset_);
                if (result < 0) {
                   LOG_ERROR("Failed to write data length to file. Total messages written: %ld",  write_counter_ );
                    return false;
                }
                remaning -= result;
                bytes_written += result;
                offset_ += result;

            }
            LOG_RET("Success", bytes_written);
        }
        /**
         * read buffer length
         * @param buffer
         * @param buf_size
         * @param offset
         * @param ntohl
         * @return 
         */
        ssize_t read_buffer_length(char* buffer, int buf_size, uint64_t offset, bool ntohl) {
            LOG_IN("buffer: %p, buf_size: %d, offset: %llu, ntohl: %d", buffer, buf_size, offset, ntohl);
            ssize_t result = 0;
            unsigned bytestoRead = sizeof (unsigned);
            unsigned bytesRead = 0;
            buffer[0]='\0';
            while (bytestoRead > 0) {
                 result = pread(fd_, buffer + bytesRead, bytestoRead, offset);
                if (result < 1) {
                    LOG_ERROR("Failed to read length of the message");
                    LOG_ERROR("offset_across_all_files_[%llu], file offset[%llu]", bytes_written_across_all_files_, offset_);
                    LOG_ERROR("buffer: %p, buf_size: %d, offset: %llu, ntohl: %d", buffer, buf_size, offset, ntohl);
                    LOG_RET("Error:", -1);
                }
                bytestoRead -= result;
                bytesRead += result;

            }

            if (bytesRead > 0) {  
                unsigned length;
                std::copy(&buffer[0], &buffer[0] + sizeof (unsigned), reinterpret_cast<char*> (&length));
                LOG_TRACE("Length before ntohl to read: %u", length);
                if (ntohl) {
                     LOG_TRACE("Length before ntohl to read: %u", length);
                    length = ntohl(length); //not needed for file io
                }
                LOG_TRACE("Length to read: %u", length);
                LOG_RET("Success", length);
            }
            LOG_RET("", bytesRead);

        }
        /**
         * read buffer
         * @param buffer
         * @param buf_size
         * @param length
         * @param offset
         * @return 
         */
        ssize_t read_buffer(char* buffer, int buf_size, int length, uint64_t offset) {
            LOG_IN("buffer: %p, buf_size: %d, length: %d, offset: %llu", buffer, buf_size, length, offset);
            ssize_t result = 0;
            unsigned bytestoRead = length;
            unsigned bytesRead = 0;
            while (bytestoRead > 0) {
                 result = pread(fd_, buffer + bytesRead, bytestoRead, offset);              
                if (result < 1) {
                    LOG_ERROR("Failed to read  of the message");
                    LOG_RET("", -1);
                }
                bytestoRead -= result;
                bytesRead += result;
            }
            LOG_RET("", bytesRead);

        }


    };
}


#endif	/* FILE_DETAILS_H */

