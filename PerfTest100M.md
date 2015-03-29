#Performance:

Laptop hardware:

    MacBook Pro (Retina, 15-inch, Late 2013)
    Processor 2.3 GHz Intel Core i7
    Memory 16 GB 1600 MHz DDR3
    
Broker Type: Transient

Broker, Consumer and Producer is running on a same machine (Sharing memory/CPU)

##100 bytes, 100M messages

Producer:

    ./dist/Release/GNU-MacOSX/lightq producer 100000000 100 event
    Total Messages:100000000, Time Taken:91.847 seconds.
    Start Time: 1427672217256, End Time:1427672309103
    1088766 messages per seconds.
    10000000000 bytes sent
    103.8329 MB per second.
    
Consumer:

    ./dist/Release/GNU-MacOSX/lightq consumer queue zmq  pull event
    Total Messages:100000001, Time Taken:91.8728 seconds.
    Start Time: 1427672217260, End Time:1427672309133
    1088462 messages per seconds.
    10000000004 bytes received
    103.8038 MB per second.
    
##256 bytes 100M Messages

Producer:

    ./dist/Release/GNU-MacOSX/lightq producer 100000000 256 event
    Total Messages:100000000, Time Taken:99.4206 seconds.
    Start Time: 1427672381712, End Time:1427672481133
    1005827 messages per seconds.
    25600000000 bytes sent
    245.5634 MB per second.
    
Consumer:

    ./dist/Release/GNU-MacOSX/lightq consumer queue zmq  pull event
    Total Messages:100000001, Time Taken:99.4544 seconds.
    Start Time: 1427672381723, End Time:1427672481178
    1005486 messages per seconds.
    25600000004 bytes received
    245.4800 MB per second.
    
## 512 bytes 100M Messages

Producer:

    ./dist/Release/GNU-MacOSX/lightq producer 100000000 512 event
    Total Messages:100000000, Time Taken:103.17 seconds.
    Start Time: 1427672549030, End Time:1427672652200
    969269 messages per seconds.
    51200000000 bytes sent
    473.2762 MB per second.
    
Consumer:

    ./dist/Release/GNU-MacOSX/lightq consumer queue zmq  pull event
    Total Messages:100000001, Time Taken:103.184 seconds.
    Start Time: 1427672549040, End Time:1427672652224
    969144 messages per seconds.
    51200000004 bytes received
    473.2149 MB per second.
    
    
 ## 1024 bytes 100M Messages

Producer:

    ./dist/Release/GNU-MacOSX/lightq producer 100000000 1024 event
    Total Messages:100000000, Time Taken:219.452 seconds.
    Start Time: 1427672739376, End Time:1427672958828
    455680 messages per seconds.
    102400000000 bytes sent
    445.0001 MB per second.
    
Consumer:

    ./dist/Release/GNU-MacOSX/lightq consumer queue zmq  pull event
    Total Messages:100000001, Time Taken:219.451 seconds.
    Start Time: 1427672739386, End Time:1427672958837
    455682 messages per seconds.
    102400000004 bytes received
    445.0020 MB per second.
