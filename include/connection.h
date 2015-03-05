/* 
 * File:   connection.h
 * Author: Rohit Joshi <rohit.c.joshi@gmail.com>
 *
 * Created on February 25, 2015, 8:39 AM
 */

#ifndef CONNECTION_H
#define	CONNECTION_H

#include "thirdparty/log.hpp"
//#include "connection.h"

namespace prakashq {

    //connection
    class connection {
    public:

        //stream typr
        enum  stream_type{
            stream_zmq,
            stream_nanomsg,
            stream_file,
            stream_socket
        };
        //endpoint type
         enum  endpoint_type{
            conn_consumer,
            conn_publisher,
            conn_broker
        };
    protected:
        connection(const std::string& topic, const std::string& uri, stream_type stype, endpoint_type conn_type) {
            LOG_IN("topic:%s, uri:%s, stream_type:%d, endpoint_type:%d", 
                    topic.c_str(), uri.c_str(), stype, conn_type);
            
            topic_ = topic;
            resource_uri_ = uri;
            stream_type_ = stype;
            connection_type_ = conn_type;
        }
    public:
        /**
         * init
         * @return 
         */
        virtual bool init() {
            LOG_IN("");
            throw std::runtime_error("connection::init():Not implemented");
            LOG_RET_TRUE("");
        }

        virtual ssize_t write(const std::string& message) {
               LOG_IN("");
            throw std::runtime_error("connection::write():Not implemented");
            LOG_RET_TRUE("");
        }

        virtual ssize_t read(ssize_t size = -1) {
               LOG_IN("");
            throw std::runtime_error("connection::read():Not implemented");
            LOG_RET_TRUE("");
        }
        virtual  ssize_t read_msg(std::string& message) {
               LOG_IN("");
            throw std::runtime_error("connection::read_msg():Not implemented");
            LOG_RET_TRUE("");
        }

        virtual ssize_t send(uint64_t offset, uint64_t size, const connection& pipe) {
              LOG_IN("");
            throw std::runtime_error("connection::send():Not implemented");
            LOG_RET_TRUE("");
        }
        
        stream_type get_stream_type() {
            return stream_type_;
        }
        endpoint_type get_endpoint_type() {
            return connection_type_;
        }
        inline const std::string& topic() const {
            return topic_;
        }
    protected:
        std::string resource_uri_;
        std::string topic_;
        stream_type stream_type_;
        endpoint_type connection_type_;

    };

}
#endif	/* CONNECTION_H */

