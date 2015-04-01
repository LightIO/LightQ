/* 
 * File:   main.cpp
 * Author: Rohit Joshi <rohit.c.joshi@gmail.com>
 *
 * Created on February 25, 2015, 8:37 AM
 */

//#include <cstdlib>
#include <iostream>
#include <chrono>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "include/log.h"
#include "include/broker_manager.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT __FILE__ ":" TOSTRING(__LINE__)
#define TID pthread_self()
#define PID getpid()

#define stringify( name ) # name
using namespace std::chrono;
using namespace std;
using namespace lightq;
//namespace lightq {

void producer_client(uint64_t counter, uint32_t payload_size, bool compress = false) {
    LOG_IN("producer num_messages[%lld] message_size[%u], compress[%d]", counter, payload_size, compress);
    connection_zmq admin_socket("lightq_topic", "tcp://127.0.0.1:5000",
            connection::conn_publisher,
            connection_zmq::zmq_req,
            connection::connect_socket,
            false,
            false);
    if (!admin_socket.init() && admin_socket.run()) {
        LOG_ERROR("Failed to initialize producer for admin connection");
        return;
    }
    sleep(2);

    std::string response;
    admin_cmd::join_req req;
    req.connection_type_ = "zmq";
    req.password_ = "T0p$3cr31";
    req.user_id_ = "test_admin";
    req.type_ = "pub";
    req.topic_ = "test";
    ssize_t size = admin_socket.write_msg(req.to_json());
    if (size <= 0) {
        LOG_ERROR("Producer: Failed to PUB ");
        return;
    }
    admin_socket.read_msg(response);
    admin_cmd::join_resp resp;
    resp.from_json(response);
    if (resp.status_ == "error") {

        LOG_ERROR("Login Failed. response %s", response.c_str());
        return;
    }

    std::string uri = resp.bind_uri_;
    boost::replace_all(uri, "*", "127.0.0.1");
    connection_zmq push_socket(resp.topic_, uri, connection::conn_publisher,
            connection_zmq::zmq_push,
            connection::connect_socket,
            false,
            false);
    if (!push_socket.init()) {
        LOG_ERROR("Failed to initialize producer push connection");
        return;
    }
    sleep(2); //wait for successful connection
    std::string message = utils::random_string(payload_size);
    LOG_ERROR("Random generated string: size: %u,  %s", message.length(), message.c_str());
    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    if (compress) {
        std::string compressed;
        utils::zlib_compress_buffer((void*) message.c_str(), message.length(), compressed);
        LOG_EVENT("Compressed size: %d", compressed.length());
    }
    uint64_t num_bytes_sent = 0;
    ssize_t result = 0;
    admin_cmd::stats_req stat_req;
    stat_req.password_ = "T0p$3cr31";
    stat_req.user_id_ = "test_admin";
    stat_req.topic_ = "test";
    while (true) {

        size = admin_socket.write_msg(stat_req.to_json());
        if (size <= 0) {
            LOG_ERROR("Failed to request for stats");
            return;
        }
        std::string response;
        admin_socket.read_msg(response);
        LOG_EVENT("Stats :%s ", response.c_str());
        LOG_DEBUG("Stats :%s ", response.c_str());
        admin_cmd::stats_resp resp;
        resp.from_json(response);
        if (resp.status_ == "error") {

            LOG_ERROR("Failed to get stat response. response %s", response.c_str());
            return;
        }
        if (resp.subscribers_count_ > 0) {
            LOG_DEBUG("Number of subscribers [%lld]", resp.subscribers_count_);
            break;
        } else {
            LOG_DEBUG("No subscribers are connecting. Waiting for subscribers to join");
            s_sleep(1000); //wait 1 sec
        }
    }


    uint32_t last_queue_size = 0;
    for (uint64_t i = 0; i < counter; i++) {
        result = push_socket.write_msg(message);
        num_bytes_sent += result;
        if (i % 500000 == 0) {
            // LOG_ERROR("i= %lld", i);
            ssize_t size = 0;
            size = admin_socket.write_msg(stat_req.to_json());
            if (size <= 0) {
                LOG_ERROR("Failed to request for stats");
                return;
            }
            std::string response;
            admin_socket.read_msg(response);
            LOG_EVENT("Stats :%s ", response.c_str());
            // LOG_DEBUG("Stats :%s ", response.c_str());
            std::cout << "Stats : " << response << std::endl;
            admin_cmd::stats_resp resp;
            resp.from_json(response);
            if (resp.status_ == "error") {
                LOG_ERROR("Failed to get stat response. response %s", response.c_str());
                continue;
            }
            if (resp.queue_size_ > 10000) {
                LOG_DEBUG("No subscribers are connecting. Waiting for subscribers to join");
                s_sleep(resp.queue_size_ / 1000);
            } else if (last_queue_size < resp.queue_size_) {
                s_sleep((resp.queue_size_ - last_queue_size) / 100);
            }
            last_queue_size = resp.queue_size_;


        }
    }
    push_socket.write_msg("STOP");
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    push_socket.write_msg("STOP");
    push_socket.write_msg("STOP");



    std::cout.unsetf(ios::hex);

    duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
    std::cout << "Total Messages:" << counter << ", Time Taken:" << time_span.count() << " seconds." << std::endl;
    std::cout << "Start Time: " << utils::get_currenttime_milliseconds(t1)
            << ", End Time:" << utils::get_currenttime_milliseconds(t2) << std::endl;

    std::cout << (uint64_t) (counter / time_span.count()) << " messages per seconds." << std::endl;
    std::cout << num_bytes_sent << " bytes sent" << std::endl;
    std::cout << std::fixed << std::setprecision(4) << num_bytes_sent / (1024 * 1024 * time_span.count()) << " MB per second." << std::endl;


    while (true) {
        ssize_t size = 0;
        size = admin_socket.write_msg(stat_req.to_json());
        if (size <= 0) {
            LOG_ERROR("Failed to request for stats");
            return;
        }
        std::string response;
        admin_socket.read_msg(response);
        LOG_EVENT("Stats :%s ", response.c_str());
        LOG_DEBUG("Stats :%s ", response.c_str());
        std::cout << "Stats : " << response << std::endl;
        sleep(10);
    }


    sleep(50000);
    LOG_OUT("");

}

void consumer_client(const std::string& broker_type, const std::string& socket_type, const std::string& sub_mod) {
    LOG_IN("consumer broker_type[%s] socket_type[%s]", broker_type.c_str(), socket_type.c_str());
    connection_zmq admin_socket("lightq_topic", "tcp://127.0.0.1:5000",
            connection::conn_consumer,
            connection_zmq::zmq_req,
            connection::connect_socket,
            false,
            false);
    if (!admin_socket.init()) {
        LOG_ERROR("Failed to initialize producer for admin connection");
        return;
    }
    sleep(2);
    std::string response;
    admin_cmd::join_req req;
    req.connection_type_ = socket_type;
    req.password_ = "T0p$3cr31";
    req.user_id_ = "test_admin";
    req.type_ = sub_mod;
    req.topic_ = "test";
    ssize_t size = admin_socket.write_msg(req.to_json());
    if (size <= 0) {
        LOG_ERROR("Producer: Failed to PUB ");
        return;
    }
    admin_socket.read_msg(response);
    admin_cmd::join_resp resp;
    resp.from_json(response);
    if (resp.status_ == "error") {

        LOG_ERROR("Login Failed. response %s", response.c_str());
        return;
    }
    LOG_INFO("Login success");


    std::string uri = resp.bind_uri_;

    boost::replace_all(uri, "*", "127.0.0.1");
    connection *p_pull_socket = NULL;
    if (socket_type == "socket") {
        p_pull_socket = new connection_socket(resp.topic_, uri, connection::conn_consumer, connection::connect_socket, false);
        if (!((connection_socket*) p_pull_socket)->init()) {
            LOG_ERROR("Failed to initialize consumer pull connection");
            return;
        }
    } else {
        if (sub_mod == "pull") {
            std::cout << "Connecting using pull" << std::endl;
            p_pull_socket = new connection_zmq(resp.topic_, uri, connection::conn_consumer,
                    connection_zmq::zmq_pull, connection::connect_socket, false, false);
        } else {
            p_pull_socket = new connection_zmq(resp.topic_, uri, connection::conn_consumer,
                    connection_zmq::zmq_sub, connection::connect_socket, false, false);
            // p_pull_socket..setsockopt(ZMQ_SUBSCRIBE, "", 0);
            std::cout << "Connecting using sub to topic:" << resp.topic_ << std::endl;
        }
        if (!p_pull_socket->init()) {
            LOG_ERROR("Failed to initialize consumer pull connection");
            return;
        }
    }
    //  std::cout << "Waiting for 10 seconds" << std::endl;
    sleep(1); //wait for successful connection
    std::string message;
    message.reserve(utils::max_msg_size);
    uint64_t counter = 0;
    high_resolution_clock::time_point t1;
    ssize_t result = 0;
    uint64_t offset = 0;
    uint64_t num_bytes_received = 0;
    char buffer[utils::max_msg_size];
    bool socket_type_socket = false;
    if (socket_type == "socket") {
            socket_type_socket = true;
    }
    bool use_buffer = true;
    while (true) {
        if (socket_type_socket) {
           

            connection_socket *p_conn_sock = (connection_socket*) p_pull_socket;
            //                ssize_t off_sent = p_conn_sock->send_offset(offset);
            //                if (off_sent <= 0) {
            //                    s_sleep(5);
            //                    LOG_DEBUG("offset sent size: %u", off_sent);
            //                    continue;
            //                }
            result = 0;
            while (result <= 0) {
                buffer[0] = '\0';
               //  if ((result = p_conn_sock->read_msg(message, false)) < 0) {
                if ((result = p_conn_sock->read_msg(buffer, utils::max_msg_size, false)) < 0) {
                    LOG_ERROR("Failed to read message");
                    std::cout << "Failed to read message" << std::endl;
                    continue;
                } else if (result == 0) {
                    //  LOG_ERROR("timeout.reting after 1 ms");
                    s_sleep(5);
                }
                if(use_buffer)
                    buffer[result] = '\0'; //remove last 4 bytes which is offset

            }
        } else {
            //// while(result < )
            if ((result = ((connection_zmq*) p_pull_socket)->read_msg(message)) < 0) {
                LOG_ERROR("Failed to read message");
                continue;
            } else if (result == 0) {
                //  LOG_ERROR("timeout.reting after 1 ms");
                s_sleep(1);
            }
        }

        unsigned msg_length = 0;
        if (use_buffer) {
            msg_length = strlen(buffer);
        } else {
            msg_length = message.length();
        }
        LOG_TRACE("Received length: %u", msg_length);

        ++counter;
        offset += result; //FIXME: read offset from payload
        num_bytes_received += result;
        if (counter == 1) {
            t1 = high_resolution_clock::now();
        }
       // if(num_bytes_received > 2560000000) {
       //     std::cout << " message:" << message<< std::endl;
       // }
        
        if (msg_length) {
            if (use_buffer) {
                LOG_TRACE("Received %s", buffer);
                if (msg_length == 4 && std::strstr(buffer, "STOP")) {
                    break;
                } else if (msg_length == 8 && std::strstr(buffer, "STOP")) {
                    break;
                }
            } else {
                LOG_TRACE("Received: %s", message.c_str());
                if (msg_length == 4 && std::strstr(message.c_str(), "STOP")) {
                    break;
                } else if (msg_length == 8 && std::strstr(message.c_str(), "STOP")) {
                    break;
                }
            }
        }

    }

    high_resolution_clock::time_point t2 = high_resolution_clock::now();

    std::cout.unsetf(ios::hex);
    duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
    std::cout << "Total Messages:" << counter << ", Time Taken:" << time_span.count() << " seconds." << std::endl;
    std::cout << "Start Time: " << utils::get_currenttime_milliseconds(t1)
            << ", End Time:" << utils::get_currenttime_milliseconds(t2) << std::endl;
    std::cout << (uint64_t) (counter / time_span.count()) << " messages per seconds." << std::endl;
    std::cout << num_bytes_received << " bytes received" << std::endl;
    std::cout << std::fixed << std::setprecision(4) << num_bytes_received / (1024 * 1024 * time_span.count()) << " MB per second." << std::endl;
    delete p_pull_socket;


    sleep(50000);
    LOG_OUT("");

}

void enabled_loglevel(const std::string& level) {
    LOG_EVENT("Enabling log level : %s", level.c_str());
    log::event_logger()->set_level(spdlog::level::notice);

    if (level == "trace") {
        log::logger()->set_level(spdlog::level::trace);
    } else if (level == "info") {
        log::logger()->set_level(spdlog::level::info);
    } else if (level == "debug") {
        log::logger()->set_level(spdlog::level::debug);
    } else if (level == "event") {
        log::logger()->set_level(spdlog::level::notice);
    } else if (level == "warn") {
        log::logger()->set_level(spdlog::level::warn);
    } else {
        log::logger()->set_level(spdlog::level::trace);
    }
}

/*
 * 
 */
int main(int argc, char** argv) {

    log::init(spdlog::level::trace);

    std::string type;
    if (argc > 1) {
        type = argv[1];
    }
    //producer
    if (type == "producer") {
        if (argc < 2) {
            std::cout << "Usage:" << argv[0] << "producer num_messages[1000000] msg_size[256] log_level[event] compress[false]" << std::cout;
            return 0;
        }
        uint64_t counter = 1000000;
        if (argc > 2) {
            counter = boost::lexical_cast<uint64_t>(argv[2]);
        }
        uint32_t payload_size = 256;
        if (argc > 3) {
            payload_size = boost::lexical_cast<uint64_t>(argv[3]);
        }
        if (argc > 4) {
            enabled_loglevel(argv[4]);
        }
        bool compress = false;
        if (argc > 5) {
            std::string cmpr = argv[5];
            if (cmpr == "true") {
                compress = true;
            }
        }
        producer_client(counter, payload_size, compress);


    } else if (type == "consumer") {
        if (argc < 2) {
            std::cout << "Usage:" << argv[0] << " consumer broker_type[queue/file] socket_type[zmq/socket] [log_level]" << std::cout;
            return 0;
        }
        std::string broker_type = "queue";
        std::string socket_type = "zmq";
        std::string sub_mode = "pull";
        if (argc > 2) {
            broker_type = argv[2];
        }
        if (argc > 3) {
            socket_type = argv[3];
        }
        if (argc > 4) {
            sub_mode = argv[4];
        }
        if (argc > 5) {
            enabled_loglevel(argv[5]);
        }
        consumer_client(broker_type, socket_type, sub_mode);
    } else {
        std::string broker_type = "queue";
        if (argc > 1) {
            broker_type = argv[1];
        }
        if (argc > 2) {
            enabled_loglevel(argv[2]);
        }
        broker_manager mgr("tcp://*:" + std::to_string(broker_config::get_next_port()));
        if (!mgr.init()) {
            LOG_ERROR("Failed to initialize");
        }
        std::thread t = std::thread([&] {
            mgr.run();
        });

        connection_zmq admin_socket("lightq_topic", "tcp://127.0.0.1:5000",
                connection::conn_publisher,
                connection_zmq::zmq_req,
                connection::connect_socket,
                false,
                false);
        if (!admin_socket.init() && admin_socket.run()) {
            LOG_ERROR("Failed to initialize producer for admin connection");
            return 0;
        }
        admin_cmd::create_topic_req req;
        req.admin_password_ = "T0p$3cr31";
        req.admin_user_id_ = "lightq_admin";
        req.broker_type_ = broker_type;
        req.topic_ = "test";
        req.user_id_ = "test_admin";
        req.password_ = "T0p$3cr31";
        std::string response;
        LOG_INFO("Sending %s", req.to_json().c_str())
        ssize_t size = admin_socket.write_msg(req.to_json());
        if (size <= 0) {
            LOG_ERROR("Failed to create topic ");
            return 0;
        }
        admin_socket.read_msg(response);
        LOG_INFO("topic creation response :%s ", response.c_str());

        t.join();

    }


    return 0;
}


//}

