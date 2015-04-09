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

Producer: (MPS 400000)

    ./lightq-producer -m 10000000  -s 100 -n 1
    Topic[test_1], Producer: first message sent timestamp [56160258l]
    Topic[test_1], Producer: last message sent timestamp [56185810l]
    Topic[test_1], Total message sent [10000000] in [25.00]sec
    Topic[test_1], Average messages sent per second [400000.00]
    Topic[test_1], Average bandwidth sent per second [38.1470] MB
    
Consumer: (MPS: 400000, Latency: 2557 ns, Bandwidth: 38.1470 MB)

    ./lightq-consumer -c socket -m 10000000 -n 1
    Topic[test_1], Total message received [10000000] in [25.00]sec
    Topic[test_1], Average messages received per second [400000.00]
    Topic[test_1], Total bytes received [999999908], average bandwidth received per second [38.1470] MB
    Topic[test_1], First message:  producer start time [56160258], consumer start time [56160269]
    Topic[test_1], Last Message: consumer endtime [56185834]
    Topic[test_1], Consumer first message latency [11] ms
    Topic[test_1], Consumer last message received -   producer first message sent timestamp [25576]
    Topic[test_1], Average latency [2557.00] nano sec
    
##256 bytes, 10M messages

Producer: (MPS 357142.84)

    ./lightq-producer -m 10000000  -s 256 -n 1
    Topic[test_1], Producer: first message sent timestamp [56602604l]
    Topic[test_1], Producer: last message sent timestamp [56630995l]
    Topic[test_1], Total message sent [10000000] in [28.00]sec
    Topic[test_1], Average messages sent per second [357142.84]
    Topic[test_1], Average bandwidth sent per second [87.1931]MB
    
Consumer: (MPS: 357142.84, Latency: 2840 ns, Bandwidth: 87.1931 MB)

    ./lightq-consumer -c socket -m 10000000 -n 1
    Topic[test_1], Total message received [10000000] in [28.00]sec
    Topic[test_1], Average messages received per second [357142.84]
    Topic[test_1], Total bytes received [2559999752], average bandwidth received per second [87.1931]MB
    Topic[test_1], First message:  producer start time [56602604], consumer start time [56602606]
    Topic[test_1], Last Message: consumer endtime [56631011]
    Topic[test_1], Consumer first message latency [2] ms
    Topic[test_1], Consumer last message received -   producer first message sent timestamp [28407]
    Topic[test_1], Average latency [2840.00] nano sec

 ##512 bytes, 10M messages

Producer: (MPS 125000)

    ./lightq-producer -m 10000000  -s 512 -n 1
    Topic[test_1], Producer: first message sent timestamp [56763002l]
    Topic[test_1], Producer: last message sent timestamp [56843148l]
    Topic[test_1], Total message sent [10000000] in [80.00]sec
    Topic[test_1], Average messages sent per second [125000.00]
    Topic[test_1], Average bandwidth sent per second [61.0351]MB
    
Consumer: (MPS: 125000, Latency: 8022 ns, Bandwidth: 61.0351 MB)

    ./lightq-consumer -c socket -m 10000000 -n 1
    Topic[test_1], Total message received [10000000] in [80.00]sec
    Topic[test_1], Average messages received per second [125000.00]
    Topic[test_1], Total bytes received [5119999496], average bandwidth received per second [61.0351]MB
    Topic[test_1], First message:  producer start time [56763002], consumer start time [56763008]
    Topic[test_1], Last Message: consumer endtime [56843231]
    Topic[test_1], Consumer first message latency [6]ms
    Topic[test_1], Consumer last message received -   producer first message sent timestamp[80229]
    Topic[test_1], Average latency [8022.00] nano sec 
