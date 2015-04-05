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
#include "lightq_api.h"
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
    uint64_t  messages_to_receive = 1000000;
   
    while ((c = getopt (argc, argv, "ht:u:p:b:c:m:l:")) != -1)
        
    switch (c)
      {
      case 'h':
        printf("Usage: [%s] [-t topic[%s]] [-u userid[%s]]  [-p password[%s]]  [-b broker_uri[%s]] [-c consumer_type[%s]] [-m messages_to_receive[%llu]] [-l loglevel[event]]\n", 
                argv[0], topic, userid, password, broker_uri, consumer_type,  messages_to_receive );
        return 0;
      case 't':
        topic = optarg;
        printf("%c %s\n",c, topic);
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
      
      case 'l':
        loglevel = optarg;
        break;
      case '?':
        if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr,
                   "Unknown option character `\\x%x'.\n",
                   optopt);
        return 1;
      default:
        break;
      }
    

   
    lightq_loglevel level = str_to_loglevel(loglevel);
    init_log(argv[0], level);

    consumer_socket_type type = zmq_consumer;
    if(!strcmp(consumer_type, "socket")) {
        type = socket_consumer;
    }
    lightq_consumer_conn* p_consumer = init_consumer(userid, password, topic,broker_uri,  type);
    
   
    if(p_consumer) {
        uint64_t total_bytes_received = 0;
        unsigned long start_time = 0;
        unsigned long end_time = 0;
        unsigned buffer_size = 0;
        char buffer[buffer_size];
       
        
        for(uint64_t i = 0; i < messages_to_receive; ++i) {
            buffer[0] = '\0';
           int bytes_received = receive_message(p_consumer, buffer,  buffer_size);
           if(bytes_received > 0) {
               total_bytes_received += bytes_received;
           }else {
               printf("Failed to received message.  index [%llu]\n", i);
           }
           if(i == 0) {
               start_time = get_current_time_millsec();
           }
        }
        end_time = get_current_time_millsec();
        unsigned total_time_ms = end_time-start_time;
        float total_time_sec = total_time_ms / 1000;
        printf("Total message received [%llu] in [%.2f]sec\n", messages_to_receive, total_time_sec);
        printf("Average messages sent per second [%.2f]\n", messages_to_receive/total_time_sec);
        printf("Average bandwidth sent per second [%.4f]MB\n", total_bytes_received / (1024*1024*total_time_sec));
        
    }
      
   return (EXIT_SUCCESS);
}

