/* 
 * File:   main.c
 * Author: Rohit Joshi <rohit.c.joshi@gmail.com>
 *
 * Created on April 2, 2015, 9:46 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <string.h>
#include <pthread.h> 
#include "lightq_api.h"

typedef struct {
    char userid[256];
    char password[256];
    char topic[256];
    char broker_uri[256];
    consumer_socket_type type;
    uint64_t messages_to_receive;
    lightq_consumer_conn* p_consumer;
} consumer_info;

/**
 * execute consumer in a thread
 * @param p_con_info
 */
static void execute_consumer(void* p_con_info) {

    unsigned buffer_size = 1024 * 1024;
    char buffer[buffer_size];
    consumer_info *p_info = (consumer_info*) p_con_info;


    if (p_info->p_consumer) {

        uint64_t total_bytes_received = 0;
        unsigned long start_time = 0;
        unsigned long end_time = 0;
        unsigned long producer_start_time = 0;

        for (uint64_t i = 0; i < p_info->messages_to_receive; ++i) {
            strcpy(buffer, "");
            int bytes_received = receive_message(p_info->p_consumer, buffer, buffer_size);
            if (bytes_received > 0) {
                total_bytes_received += bytes_received;
            } else {
                printf("Failed to received message.  index [%llu]\n", i);
            }
            if (i == 0) {
                start_time = get_current_time_millsec();
                buffer[bytes_received] = '\0';
                producer_start_time = strtoul(buffer, NULL, 10);
               
                
            }
        }

        end_time = get_current_time_millsec();
        printf("Consumer last message received consumer endtime[%lu]\n", end_time);
        unsigned total_time_ms = end_time - start_time;
        float total_time_sec = total_time_ms / 1000;
        printf("topic[%s], Total message received [%llu] in [%.2f]sec\n", p_info->topic, p_info->messages_to_receive, total_time_sec);
        printf("topic[%s],Average messages received per second [%.2f]\n", p_info->topic, p_info->messages_to_receive / total_time_sec);
        printf("topic[%s],Average bandwidth received per second [%.4f]MB\n", p_info->topic, total_bytes_received / (1024 * 1024 * total_time_sec));
        printf("topic[%s], Average latency[%2f]ms\n", p_info->topic, (end_time - producer_start_time)/p_info->messages_to_receive);
        printf("First message producer start time[%lu]\n", producer_start_time);
        printf("Consumer first message  start time[%lu]\n", start_time);
        printf("Consumer first message latency[%lu]ms\n", start_time - producer_start_time);
    } else {
        printf("Failed to initialize consumer\n");
    }

}

/*
 * start producer
 */
int main(int argc, char** argv) {

    int c;
    char *userid = "test_admin";
    char *password = "T0p$3cr31";
    char *broker_uri = "tcp://127.0.0.1:5500";
    char *loglevel = "event";
    char * topic = "test";
    char * consumer_type = "zmq";
    uint64_t messages_to_receive = 1000000;
    unsigned num_partitions = 1;

    while ((c = getopt(argc, argv, "ht:u:p:b:c:m:n:l:")) != -1) {

        switch (c) {
            case 'h':
                printf("Usage: [%s] [-t topic[%s]] [-u userid[%s]]  [-p password[%s]]  [-b broker_uri[%s]] [-c consumer_type[%s]] [-m messages_to_receive[%llu]] [-n num_partitions[%u]] [-l loglevel[event]]\n",
                        argv[0], topic, userid, password, broker_uri, consumer_type, messages_to_receive, num_partitions);
                return 0;
            case 't':
                topic = optarg;
                printf("%c %s\n", c, topic);
                break;
            case 'u':
                userid = optarg;
                break;
            case 'p':
                password = optarg;
                break;
            case 'b':
                broker_uri = optarg;
                break;
            case 'c':
                consumer_type = optarg;
                break;
            case 'm':
                messages_to_receive = strtoull(optarg, NULL, 10);
                break;
            case 'n':
                num_partitions = atoi(optarg);
                break;
            case 'l':
                loglevel = optarg;
                break;
            case '?':
                if (isprint(optopt))
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf(stderr,
                        "Unknown option character `\\x%x'.\n",
                        optopt);
                return 1;
            default:
                break;
        }
    }


     printf(" [%s] [-t topic[%s]] [-u userid[%s]]  [-p password[%s]]  [-b broker_uri[%s]] "
             "[-c consumer_type[%s]] [-m messages_to_receive[%llu]] [-n num_partitions[%u]] [-l loglevel[%s]]\n",
             argv[0], topic, userid, password, broker_uri, 
             consumer_type, messages_to_receive, num_partitions, loglevel);

    lightq_loglevel level = str_to_loglevel(loglevel);
    if (!init_log(argv[0], level)) {
        printf("Failed to initialize logging\n");
        return -1;
    } else {
        printf("Log initialized successfully for process[%s]\n", argv[0]);
    }

    consumer_socket_type type = zmq_consumer;
    if (!strcmp(consumer_type, "socket")) {
        type = socket_consumer;
        printf("Using consumer socket as tcp socket\n");
    }//


    pthread_t tid [10];
    consumer_info coninfo[10];
    char topic_buffer[256];
    for (unsigned i = 0; i < num_partitions; ++i) {
        strcpy(topic_buffer, "");
        sprintf(topic_buffer, "%s_%u", topic, i + 1);
        strcpy(coninfo[i].userid, userid);
        strcpy(coninfo[i].password, password);
        strcpy(coninfo[i].topic, topic_buffer);
        strcpy(coninfo[i].broker_uri, broker_uri);
        coninfo[i].type = type;
        coninfo[i].messages_to_receive = messages_to_receive;
        coninfo[i].p_consumer = init_consumer(userid, password, topic_buffer, broker_uri, type);

    }



    for (unsigned i = 0; i < num_partitions; ++i) {
        // execute_consumer(coninfo[i]);
        int err = pthread_create(&(tid[i]), NULL, (void*) &execute_consumer, (void*) &coninfo[i]);
        if (err != 0)
            printf("\ncan't create thread :[%s]", strerror(err));
        //else
           // printf("\n Thread created successfully\n");
    }

    for (unsigned i = 0; i < num_partitions; ++i) {
      //  printf("Waiting for joininh thread\n");
        pthread_join(tid[i], NULL);
    }
    for (unsigned i = 0; i < num_partitions; ++i) {
        free_consumer_conn(coninfo[i].p_consumer);
    }



    return (EXIT_SUCCESS);
}

