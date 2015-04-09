##Latency Test
End to end latency test is calcuated as below.
- Producer sends first message with current timestamp
- Consumer stores this as a start time
- Consumer stores current time stamp when last message is received.
- Latency = (Consumers's received last message timestamp - Producer's first message timestamp)/total messages received

Laptop hardware:

    MacBook Pro (Retina, 15-inch, Late 2013)
    Processor 2.3 GHz Intel Core i7
    Memory 16 GB 1600 MHz DDR3
    
Broker Type: Transient

## Create a transient topic (test), num partition 1
 
    ./lightq-topic -n 1
    topic test created successfully
    
##100 bytes, 10M messages

Producer: ( 1250000 msg/sec,  Bandwidth: 119.2093 MB)

    ./lightq-producer -m 10000000  -n 1 -s 100
    Topic[test_1], Producer: first message sent timestamp [6378502l]
    Topic[test_1], Producer: last message sent timestamp [6386647l]
    Topic[test_1], Total message sent [10000000] in [8.00] sec
    Topic[test_1], Average messages sent per second [1250000.00]
    Topic[test_1], Average bandwidth sent per second [119.2093] MB
    
Consumer: ( 1250000 msg/sec, Latency: 813 ns, Bandwidth: 119.2093 MB)

    ./lightq-consumer -m 10000000  -n 1
    Topic[test_1], First message:  producer start time [6378502], consumer start time [6378520]
    Topic[test_1], Total message received [10000000] in [8.00]sec
    Topic[test_1], Average messages received per second [1250000.00]
    Topic[test_1], Total bytes received [999999907], average bandwidth received per second [119.2093]MB
    Topic[test_1], Last Message: consumer endtime [6386632]
    Topic[test_1], Consumer first message latency [18] ms
    Topic[test_1], Consumer last message received -   producer first message sent timestamp [8130]
    Topic[test_1], Average latency [813.00] nano sec
    

##256 bytes, 10M messages

Producer: ( 1250000 msg/sec, Bandwidth: 305.1758 MB)

    ./lightq-producer -m 10000000  -n 1 -s 256
    Topic[test_1], Producer: first message sent timestamp [5826133l]
    Topic[test_1], Producer: last message sent timestamp [5834823l]
    Topic[test_1], Total message sent [10000000] in [8.00] sec
    Topic[test_1], Average messages sent per second [1250000.00]
    Topic[test_1], Average bandwidth sent per second [305.1758] MB
    
Consumer: ( 1250000 msg/sec, Latency: 868 ns, Bandwidth: 305.1758 MB)

    ./lightq-consumer -m 10000000  -n 1
    Topic[test_1], First message:  producer start time [5826133], consumer start time [5826155]
    Topic[test_1], Total message received [10000000] in [8.00]sec
    Topic[test_1], Average messages received per second [1250000.00]
    Topic[test_1], Total bytes received [2559999751], average bandwidth received per second [305.1758]MB
    Topic[test_1], Last Message: consumer endtime [5834813]
    Topic[test_1], Consumer first message latency [22] ms
    Topic[test_1], Consumer last message received - producer first message sent timestamp [8680]
    Topic[test_1], Average latency [868.00] nano sec
    
##512 bytes, 10M messages

Producer: ( 1000000 msg/sec,  Bandwidth: [488.2812 MB )

    ./lightq-producer -m 10000000  -n 1 -s 512
    Topic[test_1], Producer: first message sent timestamp [6231211l]
    Topic[test_1], Producer: last message sent timestamp [6241288l]
    Topic[test_1], Total message sent [10000000] in [10.00] sec
    Topic[test_1], Average messages sent per second [1000000.00]
    Topic[test_1], Average bandwidth sent per second [488.2812] MB
    
Consumer: ( 1000000 msg/sec, Latency: 1006 ns, Bandwidth: [488.2812 MB )

    ./lightq-consumer -m 10000000  -n 1
    Topic[test_1], First message:  producer start time [6231211], consumer start time [6231235]
    Topic[test_1], Total message received [10000000] in [10.00]sec
    Topic[test_1], Average messages received per second [1000000.00]
    Topic[test_1], Total bytes received [5119999495], average bandwidth received per second [488.2812]MB
    Topic[test_1], Last Message: consumer endtime [6241279]
    Topic[test_1], Consumer first message latency [24] ms
    Topic[test_1], Consumer last message received -   producer first message sent timestamp [10068]
    Topic[test_1], Average latency [1006.00] nano sec
    
    
##1024 bytes, 10M messages

Producer: ( 526315 msg/sec,  Bandwidth: 513.9802 MB)

    ./lightq-producer -m 10000000  -n 1 -s 1024
    Topic[test_1], Producer: first message sent timestamp [6029718l]
    Topic[test_1], Producer: last message sent timestamp [6049370l]
    Topic[test_1], Total message sent [10000000] in [19.00] sec
    Topic[test_1], Average messages sent per second [526315.81]
    Topic[test_1], Average bandwidth sent per second [513.9802] MB
    
Consumer: ( 526315 msg/sec, Latency: 1964 ns, Bandwidth: 513.9802 MB)

    ./lightq-consumer -m 10000000  -n 1
    Topic[test_1], First message:  producer start time [6029718], consumer start time [6029727]
    Topic[test_1], Total message received [10000000] in [19.00]sec
    Topic[test_1], Average messages received per second [526315.81]
    Topic[test_1], Total bytes received [10239998983], average bandwidth received per second [513.9802]MB
    Topic[test_1], Last Message: consumer endtime [6049366]
    Topic[test_1], Consumer first message latency [9] ms
    Topic[test_1], Consumer last message received -   producer first message sent timestamp [19648]
    Topic[test_1], Average latency [1964.00] nano sec
   

Broker Type: Durable (file) 

## Create a durable topic (test), num partition 1
 
    ./lightq-topic -n 1 -s file
    topic test created successfully
    
##100 bytes, 10M messages

Producer: (  400000 msg/sec, Bandwidth: 38.1470 MB)

    ./lightq-producer -m 10000000  -s 100 -n 1
    Topic[test_1], Producer: first message sent timestamp [4906704l]
    Topic[test_1], Producer: last message sent timestamp [4932029l]
    Topic[test_1], Total message sent [10000000] in [25.00] sec
    Topic[test_1], Average messages sent per second [400000.00]
    Topic[test_1], Average bandwidth sent per second [38.1470] MB
    
Consumer: ( 400000 msg/sec, Latency: 2535 ns, Bandwidth: 38.1470 MB)

    ./lightq-consumer -c socket -m 10000000 -n 1
    Topic[test_1], First message:  producer start time [4906704], consumer start time [4906718]
    Topic[test_1], Total message received [10000000] in [25.00]sec
    Topic[test_1], Average messages received per second [400000.00]
    Topic[test_1], Total bytes received [999999907], average bandwidth received per second [38.1470]MB
    Topic[test_1], Last Message: consumer endtime [4932061]
    Topic[test_1], Consumer first message latency [14] ms
    Topic[test_1], Consumer last message received -   producer first message sent timestamp [25357]
    Topic[test_1], Average latency [2535.00] nano sec
    
##256 bytes, 10M messages

Producer: ( 370370 msg/sec, Bandwidth: 90.4224 MB)

    ./lightq-producer -m 10000000  -s 256 -n 1
    Topic[test_1], Producer: first message sent timestamp [4783156l]
    Topic[test_1], Producer: last message sent timestamp [4810966l]
    Topic[test_1], Total message sent [10000000] in [27.00] sec
    Topic[test_1], Average messages sent per second [370370.38]
    Topic[test_1], Average bandwidth sent per second [90.4224] MB
    
Consumer: ( 370370 msg/sec, Latency: 2783 ns, Bandwidth: 90.4224 MB)

    ./lightq-consumer -c socket -m 10000000 -n 1
    Topic[test_1], First message:  producer start time [4783156], consumer start time [4783163]
    Topic[test_1], Total message received [10000000] in [27.00]sec
    Topic[test_1], Average messages received per second [370370.38]
    Topic[test_1], Total bytes received [2559999751], average bandwidth received per second [90.4224]MB
    Topic[test_1], Last Message: consumer endtime [4810993]
    Topic[test_1], Consumer first message latency [7] ms
    Topic[test_1], Consumer last message received -   producer first message sent timestamp [27837]
    Topic[test_1], Average latency [2783.00] nano sec

##512 bytes, 10M messages

Producer: ( 322580 msg/sec, Bandwidth: 157.5101 MB)

    ./lightq-producer -m 10000000  -s 512 -n 1
    Topic[test_1], Producer: first message sent timestamp [4563339l]
    Topic[test_1], Producer: last message sent timestamp [4595009l]
    Topic[test_1], Total message sent [10000000] in [31.00] sec
    Topic[test_1], Average messages sent per second [322580.66]
    Topic[test_1], Average bandwidth sent per second [157.5101] MB
    
Consumer: ( 322580 msg/sec, Latency: 3168 ns, Bandwidth: 157.5101 MB)

    ./lightq-consumer -c socket -m 10000000 -n 1
    Topic[test_1], First message:  producer start time [4563339], consumer start time [4563361]
    Topic[test_1], Total message received [10000000] in [31.00]sec
    Topic[test_1], Average messages received per second [322580.66]
    Topic[test_1], Total bytes received [5119999495], average bandwidth received per second [157.5101]MB
    Topic[test_1], Last Message: consumer endtime [4595027]
    Topic[test_1], Consumer first message latency [22] ms
    Topic[test_1], Consumer last message received -   producer first message sent timestamp [31688]
    Topic[test_1], Average latency [3168.00] nano sec
    
##1024 bytes, 10M messages

Producer: ( 270270 msg/sec, Bandwidth: 263.9358 MB)

    ./lightq-producer -m 10000000  -s 1024 -n 1
    Topic[test_1], Producer: first message sent timestamp [4319543l]
    Topic[test_1], Producer: last message sent timestamp [4357308l]
    Topic[test_1], Total message sent [10000000] in [37.00] sec
    Topic[test_1], Average messages sent per second [270270.28]
    Topic[test_1], Average bandwidth sent per second [263.9358] MB
    
Consumer: ( 270270 msg/sec, Latency: 3779 ns, Bandwidth: 263.9358 MB)

    ./lightq-consumer -c socket -m 10000000 -n 1
    Topic[test_1], First message:  producer start time [4319543], consumer start time [4319564]
    Topic[test_1], Total message received [10000000] in [37.00]sec
    Topic[test_1], Average messages received per second [270270.28]
    Topic[test_1], Total bytes received [10239998983], average bandwidth received per second [263.9358]MB
    Topic[test_1], Last Message: consumer endtime [4357340]
    Topic[test_1], Consumer first message latency [21] ms
    Topic[test_1], Consumer last message received -   producer first message sent timestamp [37797]
    Topic[test_1], Average latency [3779.00] nano sec
