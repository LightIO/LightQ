/* 
 * File:   main.c
 * Author: Rohit Joshi <rohit.c.joshi@gmail.com>
 *
 * Created on April 4, 2015, 9:07 PM
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include "lightq_api.h"

/*
 * 
 */
int main(int argc, char** argv) {

    int c;
    char *admin_userid = "lightq_admin";
    char *admin_password = "T0p$3cr31";
    char *userid = "test_admin";
    char *password = "T0p$3cr31";
    char *bind_uri = "tcp://127.0.0.1:5500";
    char *topic = "test";
    char* storage = "queue";

    char *loglevel = "event";
    unsigned num_partitions = 1;


    while ((c = getopt(argc, argv, "ht:a:d:b:u:p:s:l:n:")) != -1) {
        switch (c) {
            case 'h':
                printf("Usage: [%s] [-t topic[%s]] [-a admin_userid[%s]] [-d admin_password[%s]] [-b bind_uri[%s]] [-u userid[%s]] [-p password[%s]] [-s storage[%s]] [-n num_partitions[%u]] [-l loglevel[event]]\n",
                        argv[0], topic, admin_userid, admin_password, bind_uri, userid, password, storage, num_partitions);
                return 1;

            case 't':
                topic = optarg;
                break;
            case 'a':
                admin_userid = optarg;
                break;
            case 'd':
                admin_password = optarg;
                break;
            case 'b':
                bind_uri = optarg;
                break;
            case 'u':
                userid = optarg;
                break;
            case 'p':
                password = optarg;
                break;

            case 's':
                storage = optarg;
                break;

            case 'l':
                loglevel = optarg;
                break;
             case 'n':
                num_partitions = atoi(optarg);
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

    printf("topic[%s] admin_userid[%s] admin_password[%s] bind_uri[%s] userid[%s] password[%s] storage[%s] num_partitions[%u] loglevel[%s]\n",
            topic, admin_userid, admin_password, bind_uri, userid, password, storage, num_partitions, loglevel);

    lightq_loglevel level = str_to_loglevel(loglevel);
    printf("initiaze logging\n");
    init_log(argv[0], level);
    broker_storage_type type = queue_type;
    if (!strcmp(storage, "file")) {
        type = file_type;
    }
    char topic_buffer[256];
    strcpy(topic_buffer, "");
   
    for(unsigned i = 0; i < num_partitions;++i) {
        sprintf(topic_buffer, "%s_%u", topic, i+1);
        printf("creating topic %s\n", topic_buffer);
        if (create_topic(bind_uri, topic_buffer, admin_userid, admin_password, userid, password, type)) {
            printf("topic %s created successfully\n", topic);
        } else {
            printf("Failed to create topic %s\n", topic);
        }
    }
    return (EXIT_SUCCESS);
}

