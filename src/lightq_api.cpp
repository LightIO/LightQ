/* 
 * File:   lightq_api.cpp
 * Author: Rohit Joshi <rohit.c.joshi@gmail.com>
 *
 * Created on April 2, 2015, 8:37 AM
 */
#include <iostream>
#include "log.h"
#include "broker_manager.h"
#include "lightq_api.h"



using namespace std::chrono;
using namespace std;
using namespace lightq;

/**
 * initialize log
 * @param level
 * @return 
 */
bool init_log(const char* process_name, lightq_loglevel level) {
    bool result = false;
    if (process_name == NULL) {
        std::cerr << "Processname must not be null\n";
        return false;
    }
    try {
        return lightq::log::init(std::string(process_name), (spdlog::level::level_enum)level);
    } catch (std::exception& ex) {
        LOG_ERROR("Error: Exception [%s]", ex.what());
        std::cerr << "Failed to initialize logger. Exception: " << ex.what() << "\n";
    } catch (...) {
    }
    return result;
}

/**
 * set log level
 * @param level
 */
bool set_loglevel(lightq_loglevel level) {
    LOG_IN("level[%d]", level);
    try {
        if (level == lightq_loglevel::LOG_ALERT) {
            lightq::log::logger()->set_level(spdlog::level::alert);
        } else if (level == lightq_loglevel::LOG_CRITICAL) {
            lightq::log::logger()->set_level(spdlog::level::critical);
        } else if (level == lightq_loglevel::LOG_DEBUG) {
            lightq::log::logger()->set_level(spdlog::level::debug);
        } else if (level == lightq_loglevel::LOG_EMERG) {
            lightq::log::logger()->set_level(spdlog::level::emerg);
        } else if (level == lightq_loglevel::LOG_ERROR) {
            lightq::log::logger()->set_level(spdlog::level::err);
        } else if (level == lightq_loglevel::LOG_EVENT) {
            lightq::log::logger()->set_level(spdlog::level::notice);
        } else if (level == lightq_loglevel::LOG_INFO) {
            lightq::log::logger()->set_level(spdlog::level::info);
        } else if (level == lightq_loglevel::LOG_TRACE) {
            lightq::log::logger()->set_level(spdlog::level::trace);
        } else if (level == lightq_loglevel::LOG_WARNING) {
            lightq::log::logger()->set_level(spdlog::level::warn);
        } else if (level == lightq_loglevel::LOG_OFF) {
            lightq::log::logger()->set_level(spdlog::level::off);
        } else {
            lightq::log::logger()->set_level(spdlog::level::notice);
        }
        LOG_RET_TRUE("success");
    } catch (std::exception& ex) {
        LOG_ERROR("Error: Exception [%s]", ex.what());
    } catch (...) {
    }
    LOG_RET_FALSE("error");
}

/**
 * Initialize producer
 * @param topic
 * @param broker_uri
 * @param pubdelay_on_slow_consumer
 * @return 
 */
lightq_producer_conn* init_producer(const char* userid, const char* password, const char* topic, const char* broker_uri) {
    LOG_IN("userid[%s], password[%s], topic[%s], broker_uri[%s]",
            userid, password, topic, broker_uri);
    lightq_producer_conn* p_producer_conn = NULL;
    try {
        lightq::connection_zmq* p_admin_socket = new lightq::connection_zmq(topic, broker_uri,
                lightq::connection::conn_publisher,
                lightq::connection_zmq::zmq_req,
                lightq::connection::connect_socket,
                false,
                false);
        if (!p_admin_socket || !p_admin_socket->init() || !p_admin_socket->run()) {
            LOG_ERROR("Failed to initialize producer for admin connection");
            LOG_RET("error", p_producer_conn);
        }
        LOG_DEBUG("Admin connection to broker[%s] created successfully", broker_uri);

        lightq::admin_cmd::join_req req;
        req.connection_type_ = "zmq";
        req.password_ = password;
        req.user_id_ = userid;
        req.type_ = "pub";
        req.topic_ = topic;
        std::string req_str = req.to_json();
        LOG_DEBUG("Sending request[%s]", req_str.c_str());
        ssize_t size = p_admin_socket->write_msg(req_str);
        if (size <= 0) {
            LOG_ERROR("Failed to create a lightq_conn");
            LOG_RET("error", p_producer_conn);
        }
        std::string response;
        p_admin_socket->read_msg(response);
        LOG_DEBUG("Response received[%s]", response.c_str());
        lightq::admin_cmd::join_resp resp;
        resp.from_json(response);
        if (resp.status_ == "error") {
            LOG_ERROR("Login Failed. response %s", response.c_str());
            LOG_RET("error", p_producer_conn);
        }
        s_sleep(1000);
        lightq::connection_zmq* p_push_socket = new lightq::connection_zmq(resp.topic_, resp.bind_uri_,
                lightq::connection::conn_publisher,
                lightq::connection_zmq::zmq_push,
                lightq::connection::connect_socket,
                false,
                false);
        if (!p_push_socket || !p_push_socket->init()) {
            LOG_ERROR("Failed to initialize producer connection");
            LOG_RET("error", p_producer_conn);
        }

        lightq_conn *p_lightq_conn = new lightq_conn();
        p_lightq_conn->client_conn = static_cast<void*> (p_push_socket);
        p_lightq_conn->admin_conn = static_cast<void*> (p_admin_socket);
        p_lightq_conn->message_counter = 0;
        p_lightq_conn->payload_size_counter = 0;
        strcpy(p_lightq_conn->topic, resp.topic_.c_str());
        strcpy(p_lightq_conn->userid, userid);
        strcpy(p_lightq_conn->password, password);
        strcpy(p_lightq_conn->broker_uri, broker_uri);


        p_producer_conn = new lightq_producer_conn();
        p_producer_conn->conn = p_lightq_conn;
        p_producer_conn->delay_pub_on_slow_consumer = true;
        p_producer_conn->last_queue_size = 0;
        p_producer_conn->pubDelayAlgorithm = publish_delay_algorithm;
        LOG_EVENT("lightq_producer_conn for producer created successfully.");
        s_sleep(1000);
        return p_producer_conn;
    } catch (std::exception& ex) {
        LOG_ERROR("Error: Exception [%s]", ex.what());
    } catch (...) {
    }

    return p_producer_conn;
}

/**
 * Free producer connection
 * @param conn
 */
 void free_producer_conn(lightq_producer_conn* producer_conn){
     LOG_IN("producer_conn[%p]", producer_conn);
     if(producer_conn == NULL) {
         return;
     }
    
     if(producer_conn->conn) {
         if(producer_conn->conn->admin_conn) {
              lightq::connection_zmq* p_admin_socket = static_cast<lightq::connection_zmq*> (producer_conn->conn->admin_conn);
              delete p_admin_socket;
              producer_conn->conn->admin_conn = NULL;
         }
         if(producer_conn->conn->client_conn) {
              lightq::connection_zmq* p_push_socket = static_cast<lightq::connection_zmq*> (producer_conn->conn->client_conn);
              delete p_push_socket;
              producer_conn->conn->client_conn = NULL;
         }
         delete producer_conn->conn;
     }
     delete producer_conn;
     LOG_OUT("");
     
 }
/**
 * Publish message
 * @param conn
 * @param message
 * @param message_length
 * @return 
 */
int publish_message(lightq_producer_conn* p_producer_conn, const char* message, uint32_t message_length) {
    LOG_IN("conn[%p], message[%s], message_length[%u]",
            p_producer_conn, message, message_length);

    if (!p_producer_conn) {
        LOG_ERROR("lightq_conn is null. you must call init_producer() prior to publishing messages");
        LOG_RET("error", -1);
    }

    if (!p_producer_conn->conn || !p_producer_conn->conn || !p_producer_conn->conn->client_conn) {
        LOG_ERROR("lightq_conn is null. you must call init_producer() prior to publishing messages");
        LOG_RET("error", -1);
    }
    if (message_length > utils::max_msg_size) {
        LOG_ERROR("Message length %u is larger than allowed message length %u. Can't send message", message_length, utils::max_msg_size);
        return -1;
    }
    int bytes_sent = -1;
    try {
        lightq::connection_zmq *pub_conn = static_cast<lightq::connection_zmq*> (p_producer_conn->conn->client_conn);
        bytes_sent = pub_conn->write_msg(message, message_length);
        if (bytes_sent < 0) {
            LOG_ERROR("Failed to send message");
            LOG_RET("error", bytes_sent);
        }
        p_producer_conn->conn->message_counter += 1;
        p_producer_conn->conn->payload_size_counter += bytes_sent;

        if (p_producer_conn->delay_pub_on_slow_consumer && p_producer_conn->pubDelayAlgorithm) {
            p_producer_conn->pubDelayAlgorithm(p_producer_conn);
        }
    } catch (std::exception& ex) {
        LOG_ERROR("Error: Exception [%s]", ex.what());
    } catch (...) {
    }
    LOG_RET("success", bytes_sent);
}

/**
 * publish delay algorithm
 * @param conn
 */
void publish_delay_algorithm(void *pconn) {
    LOG_IN("conn[%p]", pconn);
    try {
        if (pconn == NULL) {
            LOG_ERROR("pconn is null. Can't run");
            return;
        }
        lightq_producer_conn* p_producer_conn = static_cast<lightq_producer_conn *> (pconn);
        if (p_producer_conn->conn == NULL) {
            LOG_ERROR("p_producer_conn->conn is null. Can't run");
            return;
        }
        topic_stats stats;
        if (p_producer_conn->conn->message_counter % 250000 == 0) {
            if (!get_stats(p_producer_conn->conn, &stats)) {
                LOG_ERROR("Failed to get stats to determine queue depth");
                return;
            }

            if (stats.queue_size > 10000) {
                LOG_DEBUG("No subscribers are connecting. Waiting for subscribers to join");
                s_sleep(stats.queue_size / 1000);
            } else if (p_producer_conn->last_queue_size < stats.queue_size) {
                s_sleep((stats.queue_size - p_producer_conn->last_queue_size) / 100);
            }
            p_producer_conn->last_queue_size = stats.queue_size;
        }
    } catch (std::exception& ex) {
        LOG_ERROR("Error: Exception [%s]", ex.what());
    } catch (...) {
    }

    LOG_OUT("");
}

/**
 * Initialize consumer
 * @param topic
 * @param broker_uri
 * @return 
 */
lightq_consumer_conn* init_consumer(const char* userid, const char* password, const char* topic, const char* broker_uri, consumer_socket_type consumer_type) {
    LOG_IN("userid[%s], password[%s], topic[%s], broker_uri[%s]",
            userid, password, topic, broker_uri);
    lightq_consumer_conn* p_consumer_conn = NULL;
    try {
        lightq::connection_zmq* p_admin_socket = new lightq::connection_zmq(topic, broker_uri,
                lightq::connection::conn_consumer,
                lightq::connection_zmq::zmq_req,
                lightq::connection::connect_socket,
                false,
                false);
        if (!p_admin_socket || !p_admin_socket->init() || !p_admin_socket->run()) {
            LOG_ERROR("Failed to initialize producer for admin connection");
            LOG_RET("error", p_consumer_conn);
        }
        std::string response;
        lightq::admin_cmd::join_req req;
        if (consumer_type == consumer_socket_type::zmq_consumer) {
            req.connection_type_ = "zmq";
        } else if (consumer_type == consumer_socket_type::socket_consumer) {
            req.connection_type_ = "socket";
        }
        req.password_ = password;
        req.user_id_ = userid;
        req.type_ = "pull";
        req.topic_ = topic;
        std::string req_str = req.to_json();
        LOG_DEBUG("Sending request [%s]", req_str.c_str());
        ssize_t size = p_admin_socket->write_msg(req_str);
        if (size <= 0) {
            LOG_ERROR("Failed to create a lightq_conn");
            LOG_RET("error", p_consumer_conn);
        }
        p_admin_socket->read_msg(response);
        lightq::admin_cmd::join_resp resp;
        resp.from_json(response);
        if (resp.status_ == "error") {
            LOG_ERROR("Login Failed. response %s", response.c_str());
            LOG_RET("error", p_consumer_conn);
        }
        s_sleep(1000);
        lightq::connection* p_consumer_socket = NULL;
        if (consumer_type == consumer_socket_type::zmq_consumer) {
            p_consumer_socket = new lightq::connection_zmq(resp.topic_, resp.bind_uri_,
                    lightq::connection::conn_consumer,
                    lightq::connection_zmq::zmq_pull,
                    lightq::connection::connect_socket,
                    false,
                    false);
        } else {
            p_consumer_socket = new lightq::connection_socket(resp.topic_,
                    resp.bind_uri_,
                    connection::conn_consumer,
                    connection::connect_socket, false);
        }
        if (!p_consumer_socket->init()) {
            LOG_ERROR("Failed to initialize producer connection");
            LOG_RET("error", p_consumer_conn);
        }
        lightq_conn *pconn = new lightq_conn();
        pconn->client_conn = static_cast<void*> (p_consumer_socket);
        pconn->admin_conn = static_cast<void*> (p_admin_socket);
        pconn->message_counter = 0;
        pconn->payload_size_counter = 0;
        strcpy(pconn->topic, resp.topic_.c_str());
        strcpy(pconn->userid, userid);
        strcpy(pconn->password, password);
        strcpy(pconn->broker_uri, broker_uri);

        p_consumer_conn = new lightq_consumer_conn();
        p_consumer_conn->p_lightq_conn = pconn;
        p_consumer_conn->socket_type = consumer_type;

        LOG_EVENT("lightq_consumer_conn[%p] for consumer created successfully.", p_consumer_conn);
        s_sleep(1000);
        return p_consumer_conn;

    } catch (std::exception& ex) {
        LOG_ERROR("Error: Exception [%s]", ex.what());
    } catch (...) {
    }
    return p_consumer_conn;
}

/**
 * Free producer connection
 * @param conn
 */
 void free_consumer_conn(lightq_consumer_conn* consumer_conn){
     LOG_IN("consumer_conn[%p]", consumer_conn);
     if(consumer_conn == NULL) {
         return;
     }
     
     if(consumer_conn->p_lightq_conn) {
         if(consumer_conn->p_lightq_conn->admin_conn) {
              lightq::connection_zmq* p_admin_socket = static_cast<lightq::connection_zmq*> (consumer_conn->p_lightq_conn->admin_conn);
              delete p_admin_socket;
              consumer_conn->p_lightq_conn->admin_conn = NULL;
         }
         if(consumer_conn->p_lightq_conn->client_conn) {
              if(consumer_conn->socket_type == consumer_socket_type::zmq_consumer) {
                lightq::connection_zmq* p_push_socket = static_cast<lightq::connection_zmq*> (consumer_conn->p_lightq_conn->client_conn);
                delete p_push_socket;
              }else {
                lightq::connection_socket* p_push_socket = static_cast<lightq::connection_socket*> (consumer_conn->p_lightq_conn->client_conn);
                delete p_push_socket;
              }
              consumer_conn->p_lightq_conn->client_conn = NULL;
         }
         
         delete consumer_conn->p_lightq_conn;
     }
     delete consumer_conn;
     LOG_OUT("");
 }
/**
 * Receive message - consumer
 * @param conn
 * @param buffer
 * @param buffer_length
 * @return 
 */
int receive_message(lightq_consumer_conn* p_consumer_conn, char* buffer, uint32_t buffer_length) {
    LOG_IN("p_consumer_conn[%p], buffer[%s], buffer_length[%u]", p_consumer_conn, buffer, buffer_length);
    int bytes_read = -1;
    try {
      
        if (p_consumer_conn->socket_type == consumer_socket_type::zmq_consumer) {
            connection_zmq *p_conn_sock = static_cast<connection_zmq*> (p_consumer_conn->p_lightq_conn->client_conn);
            bytes_read = p_conn_sock->read_msg(buffer, buffer_length);
            LOG_DEBUG("buffer read size[%d], data[%s]", bytes_read, buffer);
        } else {
            connection_socket *p_conn_sock = static_cast<connection_socket*> (p_consumer_conn->p_lightq_conn->client_conn);
            bytes_read = p_conn_sock->read_msg(buffer, buffer_length, false);
             LOG_DEBUG("buffer read size[%d], data[%s]", bytes_read, buffer);
        }
    } catch (std::exception& ex) {
        LOG_ERROR("Error: Exception [%s]", ex.what());
    } catch (...) {
    }
    LOG_RET("", bytes_read);

}

/**
 * Get stats 
 * @param conn
 * @return 
 */
bool get_stats(lightq_conn* p_lightq_conn, topic_stats* stats) {
    LOG_IN("p_lightq_conn[%p]", p_lightq_conn);
    try {
        lightq::admin_cmd::stats_req stat_req;
        stat_req.password_ = p_lightq_conn->password;
        stat_req.user_id_ = p_lightq_conn->userid;
        stat_req.topic_ = p_lightq_conn->topic;
        lightq::connection_zmq *admin_conn = static_cast<lightq::connection_zmq*> (p_lightq_conn->admin_conn);
        ssize_t size = admin_conn->write_msg(stat_req.to_json());
        if (size <= 0) {
            LOG_ERROR("Failed to send for stats request");
            strcpy(stats->status, "error");
            LOG_RET_FALSE("Failed");
        }
        std::string response;
        admin_conn->read_msg(response);
        LOG_EVENT("Stats :%s ", response.c_str());
        lightq::admin_cmd::stats_resp resp;
        resp.from_json(response);
        if (resp.status_ == "error") {
            LOG_ERROR("Failed to get stat response. response %s", response.c_str());
            strcpy(stats->status, "error");
            LOG_RET_FALSE("Failed");
        }
        std::cout << "Stats Response:" << response << std::endl;
        stats->messages_received = resp.messages_received_;
        stats->messages_sent = resp.messages_sent_;
        stats->publishers_count = resp.publishers_count_;
        stats->queue_size = resp.queue_size_;
        strcpy(stats->status, resp.status_.c_str());
        stats->subscribers_count = resp.subscribers_count_;
        strcpy(stats->topic, resp.topic_.c_str());
        stats->total_bytes_read = resp.total_bytes_read_;
        stats->total_bytes_written = resp.total_bytes_written_;
        LOG_RET_TRUE("success");
    } catch (std::exception& ex) {
        LOG_ERROR("Error: Exception [%s]", ex.what());
    } catch (...) {
    }
    LOG_RET_FALSE("failed");

}

/**
 * initialize broker
 * @param bind_uri
 * @param start_port
 * @return 
 */
lightq_broker_mgr* init_broker(const char* admin_userid, const char* admin_password, const char* transport,
        const char* bind_ip, unsigned bind_port) {
    LOG_IN("admin_userid[%s], admin_password[%u], transport[%s], bind_ip[%s], bind_port[%d]",
            admin_userid, admin_password, transport, bind_ip, bind_port);
    try {
        ;
        std::string bind_uri = transport;
        bind_uri.append("://");
        bind_uri.append(bind_ip);
        bind_uri.append(":");
        bind_uri.append(std::to_string(broker_config::get_next_port(bind_port)));
        broker_manager *p_mgr = new broker_manager(bind_uri, admin_userid, admin_password);
        if (!p_mgr->init()) {
            LOG_ERROR("Failed to initialize broker at bind uri[%s]", bind_uri.c_str());
            LOG_RET("failed", NULL);
        }
        lightq_broker_mgr *p_lightq_mgr = new lightq_broker_mgr();
        p_lightq_mgr->broker = static_cast<void*> (p_mgr);
        strcpy(p_lightq_mgr->broker_uri, bind_uri.c_str());
         s_sleep(1000);
        LOG_RET("success", p_lightq_mgr);
    } catch (std::exception& ex) {
        LOG_ERROR("Error: Exception [%s]", ex.what());
    } catch (...) {
    }
    LOG_RET("failed", NULL);
}

/**
 * Free producer connection
 * @param conn
 */
 void free_broker_mgr(lightq_broker_mgr* broker_mgr){
     LOG_IN("broker_mgr[%p]", broker_mgr);
     if(broker_mgr == NULL) {
         return;
     }
     
     if(broker_mgr->broker) {
         broker_manager* mgr = static_cast<broker_manager*>(broker_mgr->broker);
         delete mgr; 
     }
     delete broker_mgr;
     LOG_OUT("");
 }
/**
 * run broker
 * @param broker
 * @param block
 * @return 
 */
bool run_broker(lightq_broker_mgr* p_broker_mgr, bool block) {
    LOG_IN("p_broker_mgr[%p], block[%d]", p_broker_mgr, block);

    bool result = false;
    try {
        broker_manager *p_broker = static_cast<broker_manager*> (p_broker_mgr->broker);
        if (block) {
            std::thread t = std::thread([&] {
                p_broker->run();
            });
            t.join();
            result = true;
        } else {
            result = p_broker->run();
        }
    } catch (std::exception& ex) {
        LOG_ERROR("Error: Exception [%s]", ex.what());
    } catch (...) {
    }
    LOG_RET("result", result);

}

/**
 * create a topic
 * @param broker_uri
 * @param topic
 * @param admin_userid
 * @param admin_password
 * @param userid
 * @param password
 * @param storage_type
 * @return 
 */
bool create_topic(const char* broker_uri, const char* topic,  const char* admin_userid, const char* admin_password,
        const char* userid, const char* password, broker_storage_type storage_type) {
    LOG_IN("broker_uri[%s], topic[%s], admin_userid[%s], admin_password[%s], userid[%s], password[%s], storage_type[%d]",
            broker_uri, topic,  admin_userid, admin_password, userid, password, storage_type);
    try {
        lightq::connection_zmq admin_socket(topic, broker_uri,
                connection::conn_publisher,
                connection_zmq::zmq_req,
                connection::connect_socket,
                false,
                false);
        if (!admin_socket.init() && admin_socket.run()) {
            LOG_ERROR("Failed to initialize  admin connection");
            return 0;
        }
       
            admin_cmd::create_topic_req req;
            req.admin_password_ = admin_password;
            req.admin_user_id_ = admin_userid;
            if (storage_type == broker_storage_type::file_type) {
                req.broker_type_ = "file";
            } else {
                req.broker_type_ = "queue";
            }
           
            req.topic_ = topic;
            req.user_id_ = userid;
            req.password_ = password;
            std::string response;
            LOG_INFO("Sending %s", req.to_json().c_str())
            ssize_t size = admin_socket.write_msg(req.to_json());
            if (size <= 0) {
                LOG_ERROR("Failed to create topic ");
                return false;
            }
            admin_socket.read_msg(response);
            LOG_EVENT("topic [%s] creation success", req.topic_.c_str());
        LOG_RET_TRUE("success");
    } catch (std::exception& ex) {
        LOG_ERROR("Error: Exception [%s]", ex.what());
    } catch (...) {
    }
    LOG_RET_FALSE("failed");


}

/**
 * str to log level
 * @param log_level
 * @return 
 */
lightq_loglevel str_to_loglevel(const char* log_level) {

    std::string level(log_level);
    if (level == "trace") {
        return LOG_TRACE;
    } else if (level == "info") {
        return LOG_INFO;
    } else if (level == "debug") {
        return LOG_DEBUG;
    } else if (level == "event") {
        return LOG_EVENT;
    } else if (level == "warn") {
        return LOG_WARNING;
    } else if (level == "error") {
        return LOG_ERROR;
    } else if (level == "critical") {
        return LOG_CRITICAL;
    } else if (level == "alert") {
        return LOG_ALERT;
    } else if (level == "emerg") {
        return LOG_EMERG;
    } else if (level == "off") {
        return LOG_OFF;
    } else {
        return LOG_EVENT;
    }
}

/**
 * Get current time in mill second
 * @return 
 */
unsigned long get_current_time_millsec() {
    try {
        high_resolution_clock::time_point t = chrono::high_resolution_clock::now();
        return utils::get_currenttime_milliseconds(t);
    } catch (std::exception& ex) {
        LOG_ERROR("Error: Exception [%s]", ex.what());
    } catch (...) {
    }
    return 0;
}

/**
 * generate random string for a given size. make sure you pass preallocated buffer
 * @param buffer
 * @param size
 */
void generate_random_string(char* buffer, unsigned size) {
    std::string message = utils::random_string(size);
    LOG_DEBUG("generate_random_string: message[%s]", message.c_str());
    strcpy(buffer, message.c_str());
   // buffer[message.length()] = '\0';
   
}





