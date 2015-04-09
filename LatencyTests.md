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

Producer:

    ./lightq-producer -m 10000000  -n 1 -s 100
    First message start time[44733954l]
    Topic[test_1], Total message sent [10000000] in [7.00]sec
    Topic[test_1], Average messages sent per second [1428571.38]
    Topic[test_1], Average bandwidth sent per second [136.2392]MB
    
Consumer: (MPS: 1428571.38, Latency: 764 ns)

    ./lightq-consumer -m 10000000  -n 1
    Consumer last message received consumer endtime[44741594]
    topic[test_1], Total message received [10000000] in [7.00]sec
    topic[test_1],Average messages received per second [1428571.38]
    topic[test_1],Average bandwidth received per second [136.2392]MB
    First message producer start time [44733954]
    Consumer first message  start time [44733957]
    Consumer first message latency [3]ms
    topic[test_1], consumer end to producer start time [7640]
    topic[test_1], Average latency [764.00] nano sec
    

##256 bytes, 10M messages

Producer:

    ./lightq-producer -m 10000000  -n 1 -s 256
    First message start time[52937210l]
    Topic[test_1], Total message sent [10000000] in [8.00]sec
    Topic[test_1], Average messages sent per second [1250000.00]
    Topic[test_1], Average bandwidth sent per second [305.1758]MB
    
Consumer: (MPS: 1250000, Latency: 874 ns)

    ./lightq-consumer -m 10000000  -n 1
    Consumer last message received consumer endtime[52945952]
    topic[test_1], Total message received [10000000] in [8.00]sec
    topic[test_1],Average messages received per second [1250000.00]
    topic[test_1],Average bandwidth received per second [305.1758]MB
    First message producer start time [52937210]
    Consumer first message  start time [52937216]
    Consumer first message latency [6] ms
    topic[test_1], consumer end to producer start time [8742]
    topic[test_1], Average latency [874.00] nano sec
    
##512 bytes, 10M messages

Producer:

    ./lightq-producer -m 10000000  -n 1 -s 512
    First message start time[52984331l]
    Topic[test_1], Total message sent [10000000] in [10.00]sec
    Topic[test_1], Average messages sent per second [1000000.00]
    Topic[test_1], Average bandwidth sent per second [488.2812]MB
    
Consumer: (MPS: 1000000, Latency: 1053 ns)

    ./lightq-consumer -m 10000000  -n 1
    Consumer last message received consumer endtime[52994866]
    topic[test_1], Total message received [10000000] in [10.00]sec
    topic[test_1],Average messages received per second [1000000.00]
    topic[test_1],Average bandwidth received per second [488.2812]MB
    First message producer start time [52984331]
    Consumer first message  start time [52984334]
    Consumer first message latency [3] ms
    topic[test_1], consumer end to producer start time [10535]
    topic[test_1], Average latency [1053.00] nano sec
    
    
##1024 bytes, 10M messages

Producer:

    ./lightq-producer -m 10000000  -n 1 -s 1024
    First message start time[53064370l]
    Topic[test_1], Total message sent [10000000] in [19.00]sec
    Topic[test_1], Average messages sent per second [526315.81]
    Topic[test_1], Average bandwidth sent per second [513.9802]MB
    
Consumer: (MPS: 526315.81, Latency: 1936 ns)

    ./lightq-consumer -m 10000000  -n 1
    Consumer last message received consumer endtime[53083731]
    topic[test_1], Total message received [10000000] in [19.00]sec
    topic[test_1],Average messages received per second [526315.81]
    topic[test_1],Average bandwidth received per second [513.9802]MB
    First message producer start time [53064370]
    Consumer first message  start time [53064378]
    Consumer first message latency [8] ms
    topic[test_1], consumer end to producer start time [19361]
    topic[test_1], Average latency [1936.00] nano sec
   

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
