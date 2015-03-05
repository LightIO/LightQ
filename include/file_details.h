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

namespace prakashq {
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
            offset_across_all_files_ = 0;
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
        ssize_t read_msg(char* buffer, int buf_size, uint32_t offset, bool ntohl) {
            LOG_IN("%s", utils::format_str("buffer: %p, buf_Size: %d, offset: %PRIu64 ntohl: %d", buffer, buf_size, offset, ntohl).c_str());
            
            unsigned size_of_int = sizeof (unsigned);
            ssize_t size = read_buffer_length(buffer, buf_size, offset, ntohl);
            if (size <= 0) {
                LOG_RET("Read length of message 0", size);
            }
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
        ssize_t send_file(int socket, uint64_t offset, uint32_t size) {
            LOG_IN("socket[%d], offset[%lld],  size[%u]", socket, offset, size);
            LOG_TRACE("Current file offset[ %lld]", offset_);
            if(size + offset > offset_) {
                size = offset_ - offset;
                LOG_TRACE("offset [ %lld] + size[%lld] > offset_[%lld]. Setting size as offset_ - offset[%lld] ",
                        offset, size, offset_, size);
            }
            if(size <= 0) {
                LOG_TRACE("size to read is zero. returning");
                LOG_RET("no read", size);
            }
 #ifdef __APPLE__
            off_t size_offset = size;
            LOG_TRACE("Reading %lld  bytes from offset[%lld]", size, offset);
           ssize_t result =  sendfile(fd_, socket, offset, &size_offset, NULL, 0);
           if(result == 0 || result == EAGAIN) {
               LOG_RET("success", size_offset);
           }  
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
                    LOG_TRACE("Write offset is true.  adding sizeof(offset_): %d", sizeof(offset_));
                    size = size + sizeof(offset_); //sizeof(0x0);
                }
                LOG_TRACE("Writting payload length: %u", size);
                length_written = write_length(size);
                if (length_written <= 0) {
                    LOG_RET("failed", length_written);
                }
                
            }
          
            unsigned remaning = msg.length();
            size_t written_buffer = 0;
            if ((written_buffer = write_buffer(msg.c_str(), msg.length())) <= 0) {
                LOG_RET("Error", written_buffer);
            }
           
            unsigned offset_written = 0;
            if ((offset_written = write_offset()) <= 0) {
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
            LOG_TRACE("Creating filename :%s", newfile.c_str());
#ifdef __APPLE__
            int flags = O_RDWR | O_CREAT | O_ASYNC | O_NONBLOCK;
#else
            int flags = O_RDWR | O_CREAT | O_ASYNC | O_NONBLOCK | O_NOATIME | O_LARGEFILE;
#endif
          //  int fd = open(newfile.c_str(), flags, 0644);
            int fd = open("/tmp/abc.out", flags, 0644);
            std::cout << "After open file. fd: " << fd << " \n" << std::flush;
            if (fd < 0) {
                std::cout << "After open file\n" << std::flush;
                LOG_ERROR("Failed to open file: %s.  Error : %d" , newfile.c_str(), errno);
               // LOG_ERROR("Failed to open file: %s.  Error : %s" , newfile.c_str(), strerror(errno));
                return false;
            } else {
                LOG_INFO("File: %s created.",  newfile.c_str()); 
            }
            std::cout << "sucess\n" << std::flush;

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
            LOG_OUT("");
        }

      
    private:

        int fd_;
        //this track offset for total bytes written across all the files
        //e.g if 9 bytes written across 3 files, first fd_details will have 3, 2nd will have 6 and 3rd will have 9
        uint64_t offset_across_all_files_; 
        std::string file_name_;
        uint32_t offset_;
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
            LOG_IN("buffer: %p, buf_size: %d, offset: %lld, ntohl: %d", buffer, buf_size, offset, ntohl);
            int result;
            unsigned bytestoRead = sizeof (unsigned);
            unsigned bytesRead = 0;
            buffer[0]='\0';
            while (bytestoRead > 0) {
                ssize_t result = pread(fd_, buffer + bytesRead, bytestoRead, offset);
                if (result < 1) {
                    LOG_ERROR("Failed to read length of the message");
                    LOG_RET("Error:", -1);
                }
                bytestoRead -= result;
                bytesRead += result;

            }

            if (bytesRead > 0) {  
                unsigned length;
                std::copy(&buffer[0], &buffer[0] + sizeof (unsigned), reinterpret_cast<char*> (&length));
                if (ntohl) {
                    length = ntohl(length); //not needed for file io
                }
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
        ssize_t read_buffer(char* buffer, int buf_size, int length, uint32_t offset) {
            LOG_IN("buffer: %p, buf_size: %d, length: %d, offset: %u", buffer, buf_size, length, offset);
            int result;
            unsigned bytestoRead = length;
            unsigned bytesRead = 0;
            while (bytestoRead > 0) {
                ssize_t result = pread(fd_, buffer + bytesRead, bytestoRead, offset);              
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

