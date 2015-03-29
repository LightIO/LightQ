# LightQ 

It is a high performance,  brokered messaging queue which supports transient (1M msg/sec)  and durable (~300K msg/sec) queues.  Durable queues are similar to <B>Kafka</B>  where data are written to the file and consumers consume from the file.

It is mostly header only project with main.cpp as an example for broker, producer and consumer.

NOTE: This is an initial version and may not be ready for production use.

###Example: (transient broker)

Start Broker: (broker type:  queue, logging level: event)

    ./dist/Release/GNU-MacOSX/lightq queue event
  
Start Consumer: (client: consumer,  broker type: queue, client socket: zmq, logging level: event)

    ./dist/Release/GNU-MacOSX/lightq consumer queue zmq  pull event
    
Start Producer:  (client: producer, number of messages 10M, payload size: 100 bytes, logging level: event)

    ./dist/Release/GNU-MacOSX/lightq producer 10000000 100 event
    
 ##Protocol:

 ###Create a Topic:  
 

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

## 512 bytes 1M Messages

Producer:

    ./dist/Release/GNU-MacOSX/lightq producer 1000000 512 event
    Total Messages:1000000, Time Taken:3.11399 seconds.
    Start Time: 1427659753024, End Time:1427659756138
    321131 messages per seconds.
    512000000 bytes sent
    156.8026 MB per second.
    
Consumer:

    ./dist/Release/GNU-MacOSX/lightq consumer file socket  pull event
    Total Messages:1000001, Time Taken:3.12513 seconds.
    Start Time: 1427659753031, End Time:1427659756156
    319987 messages per seconds.
    512000004 bytes received
    156.2437 MB per second.
    
## 1024 bytes 1M Messages

Producer:

    ./dist/Release/GNU-MacOSX/lightq producer 1000000 1024 event
    Total Messages:1000000, Time Taken:3.63717 seconds.
    Start Time: 1427659848032, End Time:1427659851669
    274938 messages per seconds.
    1024000000 bytes sent
    268.4949 MB per second.
    
Consumer:

    ./dist/Release/GNU-MacOSX/lightq consumer file socket  pull event
    Total Messages:1000001, Time Taken:3.64869 seconds.
    Start Time: 1427659848036, End Time:1427659851685
    274071 messages per seconds.
    1024000004 bytes received
    267.6477 MB per second.
    
##License

(The MIT License)

Copyright (c) 2012-2014 Apcera Inc.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
