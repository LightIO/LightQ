#!/bin/sh
mkdir -p bin logs build
echo "$1"
if [ "$1" == "clean" ]; then
	rm -rf bin/* build/*
	cd ..
    make -f LightQ-Makefile.mk clean
	exit
fi
current_dir=`pwd`
cd ..
make -f LightQ-Makefile.mk
cd "$current_dir"

echo "Building lightq-test"
echo "Compiling lightq-test.cpp"
echo "/usr/local/bin/g++-4.9 -m64 -DNDEBUG -std=c++11 -DPICOJSON_USE_INT64  -flto  -O3  -g -I../include -I../include/thirdparty -I/opt/zeromq/include -fPIC   -o build/lightq-test.o -c lightq-test.cpp 
"
/usr/local/bin/g++-4.9 -m64 -DNDEBUG -std=c++11 -DPICOJSON_USE_INT64  -flto  -O3  -g -I../include -I../include/thirdparty -I/opt/zeromq/include -fPIC   -o build/lightq-test.o -c lightq-test.cpp 
echo "Linking lightq-test"
echo "/usr/local/bin/g++-4.9 -m64 -DNDEBUG -std=c++11 -flto  -O3  -fPIC   build/lightq-test.o -o bin/lightq-test -L../dist/Release/GNU-MacOSX -llightq -L/opt/zeromq/lib -lzmq -L/usr/local/opt/zlib/lib -lz -pthread
"
/usr/local/bin/g++-4.9 -m64 -DNDEBUG -std=c++11 -flto  -O3  -fPIC   build/lightq-test.o -o bin/lightq-test -L../dist/Release/GNU-MacOSX -llightq -L/opt/zeromq/lib -lzmq -L/usr/local/opt/zlib/lib -lz -pthread

echo "Building lightq-topic"
echo "Compiling lightq-topic.c"
echo "/usr/local/bin/gcc-4.9 -m64 -DNDEBUG -std=c11 -flto  -O3  -g -I../include  -fPIC -o build/lightq-topic.o -c lightq-topic.c 
"
/usr/local/bin/gcc-4.9 -m64 -DNDEBUG -std=c11 -flto  -O3  -g -I../include  -fPIC -o build/lightq-topic.o -c lightq-topic.c 
echo "Linking lightq-topic"
echo "/usr/local/bin/gcc-4.9 -m64 -DNDEBUG -std=c11 -flto  -O3   -I../include  -fPIC  -o bin/lightq-topic build/lightq-topic.o -L../dist/Release/GNU-MacOSX -llightq
"
/usr/local/bin/gcc-4.9 -m64 -DNDEBUG -std=c11 -flto  -O3   -I../include  -fPIC  -o bin/lightq-topic build/lightq-topic.o -L../dist/Release/GNU-MacOSX -llightq


echo "Building lightq-broker"
echo "Compiling lightq-broker.c"
echo "/usr/local/bin/gcc-4.9 -m64 -DNDEBUG -std=c11 -flto  -O3  -g -I../include  -fPIC -o build/lightq-broker.o -c lightq-broker.c 
"
/usr/local/bin/gcc-4.9 -m64 -DNDEBUG -std=c11 -flto  -O3  -g -I../include  -fPIC -o build/lightq-broker.o -c lightq-broker.c 
echo "Linking lightq-topic"
echo "/usr/local/bin/gcc-4.9 -m64 -DNDEBUG -std=c11 -flto  -O3   -I../include  -fPIC  -o bin/lightq-broker build/lightq-broker.o -L../dist/Release/GNU-MacOSX -llightq
"
/usr/local/bin/gcc-4.9 -m64 -DNDEBUG -std=c11 -flto  -O3   -I../include  -fPIC  -o bin/lightq-broker build/lightq-broker.o -L../dist/Release/GNU-MacOSX -llightq


echo "Building lightq-producer"
echo "Compiling lightq-producer.c"
echo "/usr/local/bin/gcc-4.9 -m64 -DNDEBUG -std=c11 -flto  -O3  -g -I../include  -fPIC -o build/lightq-producer.o -c lightq-producer.c 
"
/usr/local/bin/gcc-4.9 -m64 -DNDEBUG -std=c11 -flto  -O3  -g -I../include  -fPIC -o build/lightq-producer.o -c lightq-producer.c 
echo "Linking lightq-topic"
echo "/usr/local/bin/gcc-4.9 -m64 -DNDEBUG -std=c11 -flto  -O3   -I../include  -fPIC  -o bin/lightq-producer build/lightq-producer.o -L../dist/Release/GNU-MacOSX -llightq
"
/usr/local/bin/gcc-4.9 -m64 -DNDEBUG -std=c11 -flto  -O3   -I../include  -fPIC  -o bin/lightq-producer build/lightq-producer.o -L../dist/Release/GNU-MacOSX -llightq


echo "Building lightq-consumer"
echo "Compiling lightq-consumer.c"
echo "/usr/local/bin/gcc-4.9 -m64 -DNDEBUG -std=c11 -flto  -O3  -g -I../include  -fPIC -o build/lightq-consumer.o -c lightq-consumer.c 
"
/usr/local/bin/gcc-4.9 -m64 -DNDEBUG -std=c11 -flto  -O3  -g -I../include  -fPIC -o build/lightq-consumer.o -c lightq-consumer.c 
echo "Linking lightq-topic"
echo "/usr/local/bin/gcc-4.9 -m64 -DNDEBUG -std=c11 -flto  -O3   -I../include  -fPIC  -o bin/lightq-consumer build/lightq-consumer.o -L../dist/Release/GNU-MacOSX -llightq
"
/usr/local/bin/gcc-4.9 -m64 -DNDEBUG -std=c11 -flto  -O3   -I../include  -fPIC  -o bin/lightq-consumer build/lightq-consumer.o -L../dist/Release/GNU-MacOSX -llightq


