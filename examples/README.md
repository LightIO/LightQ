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
    
### Create a topic
    ./bin/lightq-topic
    
### Start Consumer (Receive 10M messages)
    ./bin/lightq-consumer -m 10000000
    
### Start Producer (Send 10M messages, size 100)
    ./bin/lightq-producer -m 10000000 -s 100
    
