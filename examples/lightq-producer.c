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
    uint32_t  message_size = 100;
    uint64_t  messages_to_send = 1000000;
   
    while ((c = getopt (argc, argv, "ht:u:p:b:m:s:l:")) != -1)
        
    switch (c)
      {
      case 'h':
        printf("Usage: [%s] [-t topic[%s]] [-u userid[%s]]  [-p password[%s]]  [-b broker_uri[%s]] [-s message_size[%u]] [-m messages_to_send[%llu]] [-l loglevel[event]]\n", 
                argv[0], topic, userid, password, broker_uri, message_size, messages_to_send );
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
      case 'm':
        messages_to_send = strtoull(optarg, NULL, 10);
        break;
      case 's':
        message_size = atoi (optarg);
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
    

    if(topic == NULL) {
            printf("Usage: [%s] -t topic [-u userid[%s]]  [-p password[%s]]  [-b broker_uri[%s]] [-m message_size[%u]] [-s messages_to_send[%llu]] [-l loglevel[event]]\n", 
                argv[0], userid, password, broker_uri, message_size, messages_to_send );
        return 0;
    }
    lightq_loglevel level = str_to_loglevel(loglevel);
    init_log(argv[0], level);

    lightq_producer_conn* p_producer = init_producer(userid, password, topic,broker_uri);
    
    if(p_producer) {
        uint64_t total_bytes_sent = 0;
        unsigned long start_time = 0;
        unsigned long end_time = 0;
        char buffer[message_size+1];
        generate_random_string(buffer, message_size);
        start_time = get_current_time_millsec();
        for(uint64_t i = 0; i < messages_to_send; ++i) {
           int bytes_sent = publish_message(p_producer, buffer, message_size);
           if(bytes_sent > 0) {
               total_bytes_sent += bytes_sent;
           }else {
               printf("Failed to send message.  index [%llu]\n", i);
           }
        }
        
        end_time = get_current_time_millsec();
        unsigned total_time_ms = end_time-start_time;
        float total_time_sec = total_time_ms / 1000;
        printf("Total message sent [%llu] in [%.2f]sec\n", messages_to_send, total_time_sec);
        printf("Average messages sent per second [%.2f]\n", messages_to_send/total_time_sec);
        printf("Average bandwidth sent per second [%.4f]MB\n", total_bytes_sent / (1024*1024*total_time_sec));
        
    }
      
   return (EXIT_SUCCESS);
}

