/* 
 * File:   connection.h
 * Author: Rohit Joshi <rohit.c.joshi@gmail.com>
 *
 * Created on February 25, 2015, 8:39 AM
 */

#ifndef CONNECTION_H
#define	CONNECTION_H

#include "log.h"
//#include "connection.h"

namespace lightq {

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
        //socket connect type
         enum socket_connect_type {
            bind_socket,
            connect_socket,
            create_file
        };
        /**
         * destructor
         */
        virtual ~connection(){}
    protected:
        /**
         * connection
         * @param topic
         * @param uri
         * @param stype
         * @param ep_type
         * @param connect_type
         */
        connection(const std::string& topic, const std::string& uri, stream_type stype, 
                endpoint_type ep_type, socket_connect_type connect_type, bool non_blocking=true) {
            
            LOG_IN("topic[%s], uri[%s], stream_type[%d], endpoint_type[%d], socket_connect_type[%d]", 
                    topic.c_str(), uri.c_str(), stype, ep_type, connect_type);
            
            topic_ = topic;
            resource_uri_ = uri;
            stream_type_ = stype;
            endpoint_type_ = ep_type;
            socket_connect_type_ = connect_type;
            non_blocking_ = non_blocking;
        }
        
        
    public:
        /**
         * init
         * @return 
         */
        virtual bool init() = 0;
        /**
         * run
         * @return 
         */
        virtual bool run() = 0;

        /**
         * write message
         * @param message
         * @return 
         */
        virtual ssize_t write_msg(const std::string& message) = 0;

        
        /**
         * read message
         * @param message
         * @return 
         */
        virtual  ssize_t read_msg(std::string& message) = 0;

        
        /**
         * get stream type
         * @return 
         */
        inline stream_type get_stream_type() {
            return stream_type_;
        }
        /**
         * get endpoint type
         * @return 
         */
        inline endpoint_type get_endpoint_type() {
            return endpoint_type_;
        }
        /**
         * get topic
         * @return 
         */
        inline const std::string& topic() const {
            return topic_;
        }
        
        inline std::string get_resource_uri_() {
            return resource_uri_;
        }
    protected:
        std::string resource_uri_;
        std::string topic_;
        stream_type stream_type_;
        endpoint_type endpoint_type_;
        socket_connect_type socket_connect_type_;
        bool non_blocking_;

    };

}
#endif	/* CONNECTION_H */

