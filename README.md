# LightQ 

[![Build Status](https://travis-ci.org/LightIO/LightQ.svg?branch=master)](https://travis-ci.org/LightIO/LightQ)

It is a high performance,  brokered messaging queue which supports transient (1M msg/sec with microseconds latency)  and durable (~300K msg/sec with milliseconds latency) queues. Durable queues are similar to <B>Kafka</B>  where data are written to the file and consumers consume from the file.

###Features:
1. [High Performance - 1M+ msg/sec](https://github.com/LightIO/LightQ/blob/master/README.md#performance)
2. [Low Latency - in microseconds](https://github.com/LightIO/LightQ/blob/master/LatencyTests.md)
3. Transient and durable queue (similar to <B>Kafka</B> where producer writes to the file, consumer reads from the file)
4. Authentication per topic (userid/password validation)
5. Header only project (embed within your project) 
6. Consumer in load balancing mode(pipeline):  One of the consumer gets a message mostly in round robin)
7. Consumer as Subscribers (Each consumer gets a copy of a message)
8. Both subscriber and pipelining mode are supported for a single topic
9. Multi Producers/Consumers for a single topic
10. Unlimited* topics per broker
11. JSON protocol to create topic and join topic (at runtime)
12. C++11 support/require
13. Logging support
14. Dynamic port allocation for topic and consumer/producer bind uri
15. Apache License
16. Cluster support (todo)
17. Client API (todo): C, Go, Java, Rust, Lua, Ruby 


It is mostly header only project with main.cpp as an example for broker, producer and consumer.

NOTE: This is an initial version and may not be ready for production use.


 
##Protocol: 

### Create a Topic: 

(Admin userid and password must be passed to create a topic. Also we need to define userid/password per topic which consumer/produder need to pass for authentication)
 

     Send a request  to the broker
    {
     "admin_password": "T0p$3cr31",
     "admin_user_id": "lightq_admin",
     "broker_type": "queue",
     "cmd": "create_topic",
     "password": "T0p$3cr31",
     "topic": "test",
     "user_id": "test_admin"
    }


    Response: 
    {
       "cmd": "create_topic",
       "description": "topic created successfully",
       "status": "ok"
    }
    

###Join Topic (Consumer):
(Need to pass userid/password for topic 'test')

    Request:
    {
       "cmd": "join",
       "connection_type": "zmq",
       "password": "T0p$3cr31",
       "topic": "test",
       "type": "pull",
       "user_id": "test_admin"
    }
    Response: 
    {
       "bind_uri": "tcp://127.0.0.1:5002",
       "cmd": "join",
       "status": "ok",
       "topic": "test"
    }
 
### Join Topic (Producer): 
(Need to pass userid/password for topic 'test')

    Request:
    {
       "cmd": "join",
       "connection_type": "zmq",
       "password": "T0p$3cr31",
       "topic": "test",
       "type": "pub",
       "user_id": "test_admin"
     }
    Response:
    {
      "bind_uri": "tcp://127.0.0.1:5003",
      "cmd": "join",
      "status": "ok",
      "topic": "test"
    }
### Get the statistics about the topic

    Request:
    {
       "cmd": "stats",
       "password": "T0p$3cr31",
       "topic": "test",
       "user_id": "test_admin"
    }
    Response:
    {
      "cmd": "stats",
      "messages_received": 9499570,
      "messages_sent": 9491554,
      "publishers_count": 1,
      "queue_size": 8016,
      "status": "ok",
      "subscribers_count": 1,
      "total_bytes_read": 0,
      "total_bytes_written": 0
   }
   
#Performance:

Laptop hardware:

    MacBook Pro (Retina, 15-inch, Late 2013)
    Processor 2.3 GHz Intel Core i7
    Memory 16 GB 1600 MHz DDR3
    
Broker Type: Transient
##100 bytes, 10M messages

Producer:

    ./dist/Release/GNU-MacOSX/lightq producer 10000000 100 event
    Total Messages:10000000, Time Taken:8.46577 seconds.
    Start Time: 1427658489112, End Time:1427658497577
    1181227 messages per seconds.
    1000000000 bytes sent
    112.6507 MB per second
    
Consumer:

    ./dist/Release/GNU-MacOSX/lightq consumer queue zmq  pull event
    Total Messages:10000001, Time Taken:8.47781 seconds.
    Start Time: 1427658489122, End Time:1427658497600
    1179550 messages per seconds.
    1000000004 bytes received
    112.4907 MB per second.
    

    
##256 bytes 10M Messages

Producer:

    ./dist/Release/GNU-MacOSX/lightq producer 10000000 256 event
    Total Messages:10000000, Time Taken:9.2752 seconds.
    Start Time: 1427658738559, End Time:1427658747834
    1078143 messages per seconds.
    2560000000 bytes sent
    263.2186 MB per second.
    
Consumer:

    ./dist/Release/GNU-MacOSX/lightq consumer queue zmq  pull event
    Total Messages:10000001, Time Taken:9.30292 seconds.
    Start Time: 1427658738562, End Time:1427658747865
    1074931 messages per seconds.
    2560000004 bytes received
    262.4345 MB per second.
    

## 512 bytes 10M Messages

Producer:

    ./dist/Release/GNU-MacOSX/lightq producer 10000000 512 event
    Total Messages:10000000, Time Taken:10.5182 seconds.
    Start Time: 1427658940094, End Time:1427658950612
    950734 messages per seconds.
    5120000000 bytes sent
    464.2258 MB per second.
    
Consumer:

    ./dist/Release/GNU-MacOSX/lightq consumer queue zmq  pull event
    Total Messages:10000001, Time Taken:10.5296 seconds.
    Start Time: 1427658940097, End Time:1427658950627
    949706 messages per seconds.
    5120000004 bytes received
    463.7239 MB per second.
    

    
## 1024 bytes 10M Messages

Producer:

    ./dist/Release/GNU-MacOSX/lightq producer 10000000 1024 event
    Total Messages:10000000, Time Taken:19.8285 seconds.
    Start Time: 1427659063592, End Time:1427659083420
    504324 messages per seconds.
    10240000000 bytes sent
    492.5049 MB per second.
    
Consumer:

    ./dist/Release/GNU-MacOSX/lightq consumer queue zmq  pull event
    Total Messages:10000001, Time Taken:19.8222 seconds.
    Start Time: 1427659063603, End Time:1427659083425
    504485 messages per seconds.
    10240000004 bytes received
    492.6617 MB per second.
    

    
#Performance: (Durable broker: file)
##100 bytes, 10M messages

Producer:

    ./dist/Release/GNU-MacOSX/lightq producer 10000000 100 event
    Total Messages:10000000, Time Taken:25.0786 seconds.
    Start Time: 1427659575158, End Time:1427659600236
    398746 messages per seconds.
    1000000000 bytes sent
    38.0275 MB per second.
    
Consumer:

    ./dist/Release/GNU-MacOSX/lightq consumer file socket  pull event
    Total Messages:10000001, Time Taken:25.0945 seconds.
    Start Time: 1427659575170, End Time:1427659600264
    398493 messages per seconds.
    1000000004 bytes received
    38.0033 MB per second.
    
##256 bytes 10M Messages

Producer:

    ./dist/Release/GNU-MacOSX/lightq producer 10000000 256 event
    Total Messages:10000000, Time Taken:28.3802 seconds.
    Start Time: 1427659339399, End Time:1427659367779
    352358 messages per seconds.
    2560000000 bytes sent
    86.0250 MB per second.
    
Consumer:

    ./dist/Release/GNU-MacOSX/lightq consumer file socket  pull event
    Total Messages:10000001, Time Taken:28.3939 seconds.
    Start Time: 1427659339410, End Time:1427659367804
    352188 messages per seconds.
    2560000004 bytes received
    85.9834 MB per second.

## 512 bytes 10M Messages

Producer:

    ./dist/Release/GNU-MacOSX/lightq producer 10000000 512 event
    Total Messages:10000000, Time Taken:31.0832 seconds.
    Start Time: 1427890737326, End Time:1427890768409
    321716 messages per seconds.
    5120000000 bytes sent
    157.0884 MB per second.
    
Consumer:

    ./dist/Release/GNU-MacOSX/lightq consumer file socket  pull event
    Total Messages:10000001, Time Taken:31.0935 seconds.
    Start Time: 1427890737329, End Time:1427890768423
    321610 messages per seconds.
    5120000004 bytes received
    157.0363 MB per second.
    
## 1024 bytes 10M Messages

Producer:

    ./dist/Release/GNU-MacOSX/lightq producer 10000000 1024 event
    Total Messages:10000000, Time Taken:37.4027 seconds.
    Start Time: 1427890878446, End Time:1427890915848
    267360 messages per seconds.
    10240000000 bytes sent
    261.0942 MB per second.
    
Consumer:

    ./dist/Release/GNU-MacOSX/lightq consumer file socket  pull event
    Total Messages:10000001, Time Taken:37.4166 seconds.
    Start Time: 1427890878455, End Time:1427890915871
    267260 messages per seconds.
    10240000004 bytes received
    260.9970 MB per second.

Performance Test with 100M messages: https://github.com/rohitjoshi/LightQ/blob/master/PerfTest100M.md
###Example: (transient broker)

Start Broker: (broker type:  queue, logging level: event)

    ./dist/Release/GNU-MacOSX/lightq queue event
  
Start Consumer: (client: consumer,  broker type: queue, client socket: zmq, logging level: event)

    ./dist/Release/GNU-MacOSX/lightq consumer queue zmq  pull event
    
Start Producer:  (client: producer, number of messages 10M, payload size: 100 bytes, logging level: event)

    ./dist/Release/GNU-MacOSX/lightq producer 10000000 100 event
    
        
##License :
[![Apache License](http://img.shields.io/badge/license-apache-blue.svg?style=flat)](LICENSE)


Dependecies:
    [ZeroMQ LGPL](http://zeromq.org/area:licensing) 


