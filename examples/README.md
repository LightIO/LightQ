##Compile Examples. 
It will create broker, consumer and producer under ./bin directory. 
Run each program with -h option to see optional commandline parameters.

    mkdir build
    cd build
    cmake ..
    make install
    cd ../bin

## Run examples
   
### Start Broker :  Run in a separate terminal
    ./bin/lightq-broker
    process_name[./lightq-broker], [-u admin_userid[lightq_admin]] [-p admin_password[T0p$3cr31]] [-i bind_ip[*]] [-b bind_port[5500]] [-t transport[tcp]] [-l loglevel[event]]
    
### Create a topic
    ./bin/lightq-topic
    topic[test] admin_userid[lightq_admin] admin_password[T0p$3cr31] bind_uri[tcp://127.0.0.1:5500] userid[test_admin] password[T0p$3cr31] storage[queue] num_partitions[1] loglevel[event]
    initiaze logging
    creating topic test_1
    topic test created successfully
    
### Start Consumer (Receive 10M messages)
    ./bin/lightq-consumer -m 10000000
     [./lightq-consumer] [-t topic[test]] [-u userid[test_admin]]  [-p password[T0p$3cr31]]  [-b broker_uri[tcp://127.0.0.1:5500]] [-c consumer_type[zmq]] [-m messages_to_receive[10000000]] [-n num_partitions[1]] [-l loglevel[event]]
     Log initialized successfully for process[./lightq-consumer]
     Consumer last message received consumer endtime[44741594]
     topic[test_1], Total message received [10000000] in [7.00]sec
     topic[test_1],Average messages received per second [1428571.38]
     topic[test_1],Average bandwidth received per second [136.2392]MB
     First message producer start time[44733954]
     Consumer first message  start time[44733957]
     Consumer first message latency[3]ms
     topic[test_1], consumer end to producer start time[7640]
     topic[test_1], Average latency[764.00]nano sec
    
### Start Producer (Send 10M messages, size 100)
    ./bin/lightq-producer -m 10000000 -s 100
    [./lightq-producer] [-t topic[test]] [-u userid[test_admin]]  [-p password[T0p$3cr31]]  [-b broker_uri[tcp://127.0.0.1:5500]] [-s message_size[100]] [-m messages_to_send[10000000]] [-n num_partitions[1]] [-l loglevel[event]]
    Sending message [N2qmWqBlQ9wQj99nsQzldVI5ZuGXbEWRK5RhRXdCdG5nG5azdNMK66MuCV6GXi5xr84P2R391UXaLHbavJvFZGfO47XWS2qVOw5l]
    First message start time[44733954l]
    Topic[test_1], Total message sent [10000000] in [7.00]sec
    Topic[test_1], Average messages sent per second [1428571.38]
    Topic[test_1], Average bandwidth sent per second [136.2392]MB
    
