/**
 * File:   utils.h
 * Author: Rohit Joshi <rohit.c.joshi@gmail.com>
 *
 * Created on February 26, 2015, 9:39 PM
 */

#ifndef UTILS_H
#define	UTILS_H
#include <netinet/in.h>
#include <iostream>
#include <zlib.h>
#include <string>
#include <chrono>
#include <vector>
#include "log.h"
using namespace std::chrono;
namespace lightq {

    class utils {
    public:
        enum message_size {
            max_msg_size = 1024 * 1024,
            max_small_msg_size = 256

        };

        inline static std::string format_str(const char * buf, ...) {
            va_list args;
            va_start(args, buf);
            std::string msg = lightq::log::format_arg_list(buf, args);
            va_end(args);
            return msg;
        }

        static bool replace(std::string& str, const std::string& from, const std::string& to) {
            size_t start_pos = str.find(from);
            if (start_pos == std::string::npos)
                return false;
            str.replace(start_pos, from.length(), to);
            return true;
        }

        /**
         * Get tokens
         * @param buffer
         * @param size
         * @param tokens
         */
        static std::vector<std::string> get_tokens(const std::string& message, char del = ' ') {
            LOG_IN("message: %s,  del: %c", message.c_str(), del);
            std::string msg(message);
            msg.erase(std::remove(msg.begin(), msg.end(), '\r'), msg.end());
            msg.erase(std::remove(msg.begin(), msg.end(), '\n'), msg.end());
            std::istringstream ss(msg);
            std::string token;
            std::vector<std::string> tokens;
            while (std::getline(ss, token, del)) {
                tokens.push_back(token);
                LOG_DEBUG("Token: %s", token.c_str());
            }
            LOG_DEBUG("Number of tokens received :%d", tokens.size());
            return std::move(tokens);
        }

        /**
         * convert uri to host_port
         * @param uri
         * @param host
         * @param port
         * @return 
         */
        static bool convert_uri_host_port(const std::string& uri, std::string& host, uint32_t& port) {
            LOG_IN("uri[%s]", uri.c_str());
            std::vector<std::string> tokens = utils::get_tokens(uri, ':');
            if (tokens.size() != 3) {
                LOG_ERROR("Invalid uri %s", uri.c_str());
                LOG_RET_FALSE("");
            }
            port = (unsigned int) std::strtoul(tokens[2].c_str(), NULL, 10);
            // tokens.clear();
            host = tokens[1];
            utils::replace(host, "/", "");

            LOG_DEBUG("Converted %s to host: %s, port: %u", uri.c_str(), host.c_str(), port);
            LOG_RET_TRUE("");


        }

        /**
         * 
         * @return 
         */
        static std::string get_current_threadid() {
            static std::string thread_id = "";
            if (thread_id.length() == 0) {
                std::stringstream ss;
                ss << std::this_thread::get_id();
                thread_id = ss.str();
            }
            return thread_id;
        }

        /**
         * thread id to string
         * @param id
         * @return 
         */
        static std::string thread_id_to_str(std::thread::id id) {
            std::stringstream ss;
            ss << id;
            return ss.str();
        }

        inline static unsigned long get_currenttime_milliseconds() {
            high_resolution_clock::time_point t1 = high_resolution_clock::now();
            return (t1.time_since_epoch() / std::chrono::milliseconds(1));
        }

        inline static unsigned long get_currenttime_milliseconds(high_resolution_clock::time_point &t1) {
            return (t1.time_since_epoch() / std::chrono::milliseconds(1));
        }

        /**
         * random string
         * @param length
         * @return 
         */
        static std::string random_string(size_t length) {
            static auto randchar = []() -> char {
                const char charset[] =
                        "0123456789"
                        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                        "abcdefghijklmnopqrstuvwxyz";
                const size_t max_index = (sizeof (charset) - 1);
                return charset[ rand() % max_index ];
            };
            std::string str(length, 0);
            std::generate_n(str.begin(), length, randchar);
            return std::move(str);
        }

        /**
         * compress zlib data
         * @param in_data
         * @param in_data_size
         * @param out_data
         * @return 
         */
        static bool zlib_compress_buffer(void *in_data, size_t in_data_size, std::string &out_data) {
            const size_t BUFSIZE = 128 * 1024;

            std::string buffer;

            buffer.reserve(BUFSIZE);

            uint8_t temp_buffer[BUFSIZE];

            z_stream strm;
            strm.zalloc = 0;
            strm.zfree = 0;
            strm.next_in = reinterpret_cast<uint8_t *> (in_data);
            strm.avail_in = in_data_size;
            strm.next_out = temp_buffer;
            strm.avail_out = BUFSIZE;

            deflateInit(&strm, Z_BEST_COMPRESSION);

            while (strm.avail_in != 0) {
                int res = deflate(&strm, Z_NO_FLUSH);
                if (res != Z_OK) {
                    LOG_ERROR("Error: %s", zlib_error_str(res).c_str());
                    LOG_RET_FALSE("zlib error");
                }
                if (strm.avail_out == 0) {
                    buffer.insert(buffer.end(), temp_buffer, temp_buffer + BUFSIZE);
                    strm.next_out = temp_buffer;
                    strm.avail_out = BUFSIZE;
                }
            }

            int deflate_res = Z_OK;
            while (deflate_res == Z_OK) {
                if (strm.avail_out == 0) {
                    buffer.insert(buffer.end(), temp_buffer, temp_buffer + BUFSIZE);
                    strm.next_out = temp_buffer;
                    strm.avail_out = BUFSIZE;
                }
                deflate_res = deflate(&strm, Z_FINISH);
            }

            if (deflate_res != Z_STREAM_END) {
                LOG_ERROR("Error: %s", zlib_error_str(deflate_res).c_str());
                LOG_RET_FALSE("zlib error");
            }
            buffer.insert(buffer.end(), temp_buffer, temp_buffer + BUFSIZE - strm.avail_out);
            deflateEnd(&strm);

            out_data.swap(buffer);
            LOG_RET_TRUE("");
        }

        /**
         * decompress zlib data
         * @param in_data
         * @param in_data_size
         * @param out_data
         * @return 
         */
        static bool zlib_decompress_buffer(void *in_data, ssize_t in_data_size, std::string &out_data) {

            z_stream d_stream; /* decompression stream */

            out_data.clear();
            out_data.reserve((uint32_t) in_data_size / 1.5);

            d_stream.zalloc = Z_NULL;
            d_stream.zfree = Z_NULL;
            d_stream.opaque = Z_NULL;
            d_stream.avail_in = 0;
            d_stream.next_in = Z_NULL;
            int res = inflateInit(&d_stream);
            if (res != Z_OK) {
                LOG_ERROR("Error: %s", zlib_error_str(res).c_str());
                LOG_RET_FALSE("zlib error");
            }

            d_stream.avail_in = in_data_size;
            d_stream.next_in = (unsigned char*) in_data;

            for (;;) {
                uint8_t d_buffer[10] = {};
                d_stream.next_out = &d_buffer[0];
                d_stream.avail_out = 10;

                res = inflate(&d_stream, Z_NO_FLUSH);

                if (res == Z_STREAM_END) {
                    for (unsigned i = 0; i < (10 - d_stream.avail_out); i++)
                        out_data.push_back(d_buffer[i]);
                    if (d_stream.avail_in == 0)
                        break;
                }

                if (res != Z_OK) {
                    LOG_ERROR("Error: %s", zlib_error_str(res).c_str());
                    LOG_RET_FALSE("zlib error");
                }


                for (unsigned i = 0; i < (10 - d_stream.avail_out); i++)
                    out_data.push_back(d_buffer[i]);
            }
            res = inflateEnd(&d_stream);
            if (res != Z_OK) {
                LOG_ERROR("Error: %s", zlib_error_str(res).c_str());
                LOG_RET_FALSE("zlib error");
            }

            LOG_RET_TRUE("success");
        }

        /**
         * Read size
         * @param buffer
         * @param buf_size
         * @param offset
         * @param ntohl
         * @return 
         */
        static ssize_t read_size(int fd, bool ntohl = false) {
            LOG_IN("fd[%d], ntohl[%d]", fd, ntohl);

            uint32_t bytestoRead = sizeof (uint32_t);
            char buffer[bytestoRead + 1];
            buffer[0] = '\0';
            uint32_t bytesRead = 0;
            ssize_t result = -1;
            while (bytestoRead > 0) {
                result = read(fd, buffer + bytesRead, bytestoRead);

                if (result < 0) {
                    LOG_ERROR("Failed  to read size Err: %d, ErrDesc: %s",
                            errno, strerror(errno));
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        LOG_RET("", 0);
                    } else {
                        LOG_ERROR("Failed to read length of the message");
                        LOG_RET("Error:", -1);
                    }
                } else if (result == 0) {
                    LOG_RET("no data to read", 0);
                }
                bytestoRead -= result;
                bytesRead += result;

            }
            if (bytesRead > 0) {
                uint32_t length;
                std::copy(&buffer[0], &buffer[0] + sizeof (uint32_t), reinterpret_cast<char*> (&length));
                if (ntohl) {
                    length = ntohl(length); //not needed for file io
                }
                LOG_RET("Success", length);
            }
            LOG_RET("", bytesRead);

        }

        /**
         * read buffer
         * @param fd
         * @param buffer
         * @param buf_size
         * @param length
         * @return 
         */
        static ssize_t read_buffer(int fd, char* buffer, uint32_t buf_size, uint32_t length) {
            LOG_IN("fd[%d], buffer[%p], buf_size[%u], length[%u]", fd, buffer, buf_size, length);

            uint32_t bytestoRead = length;
            uint32_t bytesRead = 0;
            ssize_t result = -1;

            while (bytestoRead > 0) {
                result = read(fd, (void*)(buffer + bytesRead), bytestoRead);
                //if failed to read
                if (result < 0) {
                    //if read some of the buffer
                    if (bytesRead > 0) {
                        LOG_RET("partial", bytesRead);
                    }
                    LOG_RET("Failed", -1);
                }
                bytestoRead -= result;
                bytesRead += result;

            }
            LOG_RET("success", bytesRead);

        }

        /**
         * write_buffer
         * @param fd
         * @param buffer
         * @param size
         * @return 
         */
        static ssize_t write_buffer(int fd, const char* buffer, uint32_t size) {
            LOG_IN("fd[%d], buffer[%p], size[%u]", fd, buffer, size);
            uint32_t remaning = size;
            ssize_t bytes_written = 0;

            while (remaning > 0) {
                ssize_t result = write(fd, buffer + bytes_written, remaning);
                if (result < 0) {
                    if (bytes_written > 0) {
                        LOG_RET("partial", bytes_written);
                    }
                    LOG_RET("Failed", -1);
                }
                remaning -= result;
                bytes_written += result;
            }
            LOG_RET("success", bytes_written);
        }

        /**
         * write uint32
         * @param length
         * @return 
         */
        static ssize_t write_size(int fd, uint32_t length, bool htonl = false) {
            LOG_IN("fd[%d], length[%u], htonl[%d]", fd, length, htonl);
            if (htonl) {
                length = ntohl(htonl); //not needed for file io
            }
            uint32_t remaning = sizeof (length);
            char* buf = (char*) &length;
            ssize_t bytes_written = 0;
            while (remaning > 0) {
                ssize_t result = write(fd, buf + bytes_written, remaning);
                if (result < 0) {
                    if (bytes_written > 0) {
                        LOG_RET("partial", bytes_written);
                    }
                    LOG_RET("failed", -1);
                }
                remaning -= result;
                bytes_written += result;
            }
            LOG_RET("Success", bytes_written);
        }

        /**
         * read line
         * @param fd
         * @param buffer
         * @param buf_size
         * @return 
         */
        static ssize_t read_line(int fd, char* buffer, uint32_t buf_size) {

            if (buf_size == 0 || buffer == NULL) {
                return 0;
            }


            ssize_t numRead; /* # of bytes fetched by last read() */
            size_t totRead; /* Total bytes read so far */


            char ch;
            bool found_r = false;


            totRead = 0;
            for (;;) {
                numRead = read(fd, &ch, 1);

                if (numRead == -1) {
                    if (errno == EINTR) /* Interrupted --> restart read() */
                        continue;
                    else
                        return -1; /* Some other error */

                } else if (numRead == 0) { /* EOF */
                    if (totRead == 0) /* No bytes read; return 0 */
                        return 0;
                    else /* Some bytes read; add '\0' */
                        break;

                } else { /* 'numRead' must be 1 if we get here */
                    if (totRead < buf_size - 1) { /* Discard > (n - 1) bytes */
                        totRead++;
                        *buffer++ = ch;
                    }
                    if (ch == '\r') {
                        found_r = true;
                    } else if (ch == '\n' && found_r) {
                        break;
                    } else {
                        found_r = false;
                    }
                }
            }

            *buffer = '\0';
            return totRead;
        }

        /**
         * create topic id from topic and partition
         * @param topic
         * @param partition
         * @return 
         */
        static std::string create_topic_id(const std::string& topic, unsigned partition) {
            std::string topic_id(topic);
            topic_id.append("_");
            topic_id.append(std::to_string(partition));
            return std::move(topic_id);
        }
    private:

        /**
         * convert zlib error code to string
         * @param ret
         * @return 
         */
        static std::string zlib_error_str(int ret) {
            switch (ret) {
                case Z_OK:
                    return std::string("success");
                    break;
                case Z_ERRNO:
                    return std::string("Failed to read or write buffer");
                    break;
                case Z_STREAM_ERROR:
                    return std::string("invalid compression level");
                    break;
                case Z_DATA_ERROR:
                    return std::string("invalid or incomplete deflate data");
                    break;
                case Z_MEM_ERROR:
                    return std::string("out of memory");
                    break;
                case Z_VERSION_ERROR:
                    return std::string("zlib version mismatch!");
                default:
                {
                    return std::string("Unknown error");
                }
            }
        }
    };
}


#endif	/* UTILS_H */

