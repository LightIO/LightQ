LightQ project dependens on zeromq so make sure it is in the path.

1.  Create a build directory and cd to build

    mkdir build
    
    cd build
    
2.  Run cmake command

    cmake ..

3.  Run make install command.  It will install programs under bin directory

    make install
    
    cd ../bin
    
    mkdir logs #logs directory is required where you start program
    
4.   Start a broker. Pass -h for optional command line options.
    
    ./lightq-broker
    
5.   Create a topic (test).  Pass -h for optional command line options.

     ./lightq-topic
     
6.   Start a consumer (Read 10M messages). Pass -h for optional command line options.
 
    ./lightq-consumer -m 100000000

7.   Start a producer (Send 10M messages). Pass -h for optional command line options.
    
      ./lightq-producer -m 10000000

    
    
    
