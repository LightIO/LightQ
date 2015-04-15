/* 
 * File:   main.c
 * Author: Rohit Joshi <rohit.c.joshi@gmail.com>
 *
 * Created on April 2, 2015, 9:46 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#ifndef __APPLE__
#include <getopt.h>
#endif
#include <ctype.h>
#include <string.h>
#include <pthread.h>
#include "lightq_api.h"

typedef struct {
    char userid[256];
    char password[256];
    char topic[256];
    char broker_uri[256];
    unsigned message_size;
    uint64_t messages_to_send;
    lightq_producer_conn *p_producer;
}publisher_info;

static void execute_publisher(void *p_pub) {
    publisher_info *pub = (publisher_info *) p_pub;


    char buffer[pub->message_size + 1];

    if (pub->p_producer) {
        uint64_t total_bytes_sent = 0;
        unsigned long start_time = 0;
        unsigned long end_time = 0;
        int bytes_sent = 0;

        generate_random_string(buffer, pub->message_size);
        printf("Sending message [%s]\n", buffer);
        start_time = get_current_time_millsec();
        for (uint64_t i = 0; i < pub->messages_to_send; ++i) {
            if (i == 0) {
                sprintf(buffer, "%lu", start_time);
                bytes_sent = publish_message(pub->p_producer, buffer, strlen(buffer));
                printf("Topic[%s], Producer: first message sent timestamp [%lu]\n", pub->topic, start_time);
            } else {
                bytes_sent = publish_message(pub->p_producer, buffer, pub->message_size);
            }

            if (bytes_sent > 0) {
                total_bytes_sent += bytes_sent;
            } else {
                printf("Failed to send message.  index [%llu]\n", i);
            }
        }

        end_time = get_current_time_millsec();
        printf("Topic[%s], Producer: last message sent timestamp [%lu]\n", pub->topic, end_time);
        unsigned total_time_ms = end_time - start_time;
        float total_time_sec = total_time_ms / 1000;

        printf(
            "Topic[%s], Total message sent [%llu] in [%.2f] sec\n", pub->topic, pub->messages_to_send,
            total_time_sec);
        printf(
            "Topic[%s], Average messages sent per second [%.2f]\n", pub->topic,
            pub->messages_to_send / total_time_sec);
        printf(
            "Topic[%s], Average bandwidth sent per second [%.4f] MB\n", pub->topic,
            total_bytes_sent / (1024 * 1024 * total_time_sec));

    }
}

/*
 * start producer
 */
int main(int argc, char **argv) {

    int c;
    char *userid = "test_admin";
    char *password = "T0p$3cr31";
    char *broker_uri = "tcp://127.0.0.1:5500";
    char *loglevel = "event";
    char *topic = "test";
    uint32_t message_size = 100;
    uint64_t messages_to_send = 1000000;
    unsigned num_partitions = 1;

    while ((c = getopt(argc, argv, "ht:u:p:b:m:s:n:l:")) != -1)

        switch (c) {
            case 'h':
                printf(
                    "Usage: [%s] [-t topic[%s]] [-u userid[%s]]  [-p password[%s]]  [-b broker_uri[%s]] [-s message_size[%u]] [-m messages_to_send[%llu]] [-n num_partitions[%u]] [-l loglevel[event]]\n",
                    argv[0], topic, userid, password, broker_uri, message_size, messages_to_send, num_partitions);
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
            case 'm':
                messages_to_send = strtoull(optarg, NULL, 10);
                break;
            case 's':
                message_size = atoi(optarg);
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
                    fprintf(
                        stderr,
                        "Unknown option character `\\x%x'.\n",
                        optopt);
                return 1;
            default:
                break;
        }


    if (topic == NULL) {
        printf(
            "Usage: [%s] -t topic [-u userid[%s]]  [-p password[%s]]  [-b broker_uri[%s]] [-m message_size[%u]] [-s messages_to_send[%llu]] [-n num_partitions[%u]] [-l loglevel[event]]\n",
            argv[0], userid, password, broker_uri, message_size, messages_to_send, num_partitions);
        return 0;
    }

    printf(
        " [%s] [-t topic[%s]] [-u userid[%s]]  [-p password[%s]]  [-b broker_uri[%s]] [-s message_size[%u]] [-m messages_to_send[%llu]] [-n num_partitions[%u]] [-l loglevel[%s]]\n",
        argv[0], topic, userid, password, broker_uri, message_size, messages_to_send, num_partitions, loglevel);


    lightq_loglevel level = str_to_loglevel(loglevel);
    if (!init_log("logs", argv[0], level)) {
        printf("Failed to initialize logging");
        return -1;
    }

    pthread_t tid[10];
    publisher_info pubs[10];
    for (unsigned i = 0; i < num_partitions; ++i) {
        char topic_buffer[256];
        topic_buffer[0] = '\0';
        sprintf(topic_buffer, "%s_%u", topic, i + 1);
        strcpy(pubs[i].userid, userid);
        strcpy(pubs[i].password, password);
        strcpy(pubs[i].topic, topic_buffer);
        strcpy(pubs[i].broker_uri, broker_uri);
        pubs[i].message_size = message_size;
        pubs[i].messages_to_send = messages_to_send;
        pubs[i].p_producer = init_producer(userid, password, topic_buffer, broker_uri);

    }
    sleep(3);
    for (unsigned i = 0; i < num_partitions; ++i) {
        int err = pthread_create(&(tid[i]), NULL, (void *) &execute_publisher, (void *) &pubs[i]);
        if (err != 0)
            printf("\ncan't create thread :[%s]", strerror(err));
        // else
        //     printf("\n Thread created successfully\n");
    }
    for (unsigned i = 0; i < num_partitions; ++i) {
        pthread_join(tid[i], NULL);
    }
    sleep(5);
    for (unsigned i = 0; i < num_partitions; ++i) {
        free_producer_conn(pubs[i].p_producer);
    }

    return (EXIT_SUCCESS);
}

