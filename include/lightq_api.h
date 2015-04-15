/* 
 * File:   lightq_api.h
 * Author: Rohit Joshi <rohit.c.joshi@gmail.com>
 *
 * Created on April 2, 2015, 8:16 PM
 */

#ifndef LIGHTQ_API_H
#define    LIGHTQ_API_H

#include <stdint.h>

#ifdef    __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
/* in case the compiler is a C++ compiler */
#define DEFAULT_VALUE(value) = value
#else
    /* otherwise, C compiler, do nothing */
    #define DEFAULT_VALUE(value)
    #define true 1
    #define false 0
    typedef int bool;
#endif
/**
     Log Level
     */
typedef enum {

    LOG_TRACE = 0,
    LOG_DEBUG = 1,
    LOG_INFO = 2,
    LOG_EVENT = 3,
    LOG_WARNING = 4,
    LOG_ERROR = 5,
    LOG_CRITICAL = 6,
    LOG_ALERT = 7,
    LOG_EMERG = 8,
    LOG_OFF = 9
}lightq_loglevel;

typedef enum {
    queue_type,
    file_type,
    direct_type,
    queue_file_type
}broker_storage_type;

/**
 * Topic statistics
 */
typedef struct {
    char status[128];
    char topic[256];
    char topic_type[256];
    uint64_t queue_size;
    uint64_t messages_sent;
    uint64_t messages_received;
    uint64_t publishers_count;
    uint64_t subscribers_count;
    uint64_t total_bytes_written;
    uint64_t total_bytes_read;
}topic_stats;


/**
 * LightQ connection handle
 */
typedef struct {
    void *client_conn;
    void *admin_conn;
    char topic[256];
    char userid[128];
    char password[128];
    char broker_uri[256];
    uint64_t message_counter;
    uint64_t payload_size_counter;

}lightq_conn;

typedef struct {
    lightq_conn *conn;
    uint64_t last_queue_size; //only used for determining if queue depth increasing
    char topic_type[256];
    bool delay_pub_on_slow_consumer;

    void (*pubDelayAlgorithm)(void *);
}lightq_producer_conn;

/**
 * consumer_socket_type
 */
typedef enum {
    zmq_consumer,
    socket_consumer
}consumer_socket_type;

//consumer connection
typedef struct {
    lightq_conn *p_lightq_conn;
    consumer_socket_type socket_type;
}lightq_consumer_conn;


/**
 * Broker manager
 */
typedef struct {
    void *broker;
    char broker_uri[256];
}lightq_broker_mgr;


/**
 * initialize log
 * @param level
 * @return
 */
bool init_log(
    const char *logdir, const char *process_name, lightq_loglevel level DEFAULT_VALUE(lightq_loglevel::LOG_EVENT));

/**
 * set log level
 * @param level
 */
bool set_loglevel(lightq_loglevel level);

lightq_producer_conn *init_producer(
    const char *userid, const char *password, const char *topic,
    const char *broker_uri);

/**
 * Free producer connection
 * @param producer_conn
 */
void free_producer_conn(lightq_producer_conn *producer_conn);
/**
 * Initialize producer
 * @param topic
 * @param broker_uri
 * @param pubdelay_on_slow_consumer
 * @return
 */
int publish_message(lightq_producer_conn *conn, const char *message, uint32_t message_length);

/**
 * Initialize consumer
 * @param topic
 * @param broker_uri
 * @return
 */
lightq_consumer_conn *init_consumer(
    const char *userid, const char *password, const char *topic, const char *broker_uri,
    consumer_socket_type type DEFAULT_VALUE(consumer_socket_type::zmq_consumer));

/**
 * free consumer connection
 * @param consumer_conn
 */
void free_consumer_conn(lightq_consumer_conn *consumer_conn);
/**
 * Receive message - consumer
 * @param conn
 * @param buffer
 * @param buffer_length
 * @return
 */
int receive_message(lightq_consumer_conn *p_consumer_conn, char *buffer, uint32_t buffer_length);
/**
 * Get stats
 * @param conn
 * @return
 */
bool get_stats(lightq_conn *conn, topic_stats *stats);

/**
 * publish delay algorithm function: default implementation
 * @param
 */
void publish_delay_algorithm(void *);


/**
 * init_broker
 * @param admin_userid
 * @param admin_password
 * @param transport
 * @param bind_ip
 * @param bind_port
 * @return
 */
lightq_broker_mgr *init_broker(
    const char *admin_userid, const char *admin_password, const char *transport,
    const char *bind_ip, unsigned bind_port);

/**
 * Free broker manager
 * @param broker_mgr
 */
void free_broker_mgr(lightq_broker_mgr *broker_mgr);
/**
 * Run broker.  This is a blocking call if block is enabled
 * @param broker
 * @param block using it's own thread
 * @return
 */
bool run_broker(lightq_broker_mgr *broker, bool block DEFAULT_VALUE(true));

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
bool create_topic(
    const char *broker_uri, const char *topic, const char *admin_userid, const char *admin_password,
    const char *userid, const char *password, broker_storage_type storage_type);


/**
 * str to log level
 * @param log_level
 * @return
 */
lightq_loglevel str_to_loglevel(const char *log_level);


/**
 * get current time in mill second
 * @return
 */
unsigned long get_current_time_millsec();

/**
 * generate random string for a given size. make sure you pass preallocated buffer size+1 (for null termination)
 * @param buffer
 * @param size
 */
void generate_random_string(char *buffer, unsigned size);


/**
 * sleep in mill second
 * @param
 */
void sleep_ms(unsigned);


#ifdef    __cplusplus
}
#endif

#endif	/* LIGHTQ_API_H */

