
cmake_minimum_required(VERSION 2.8)

project(lightq_project)
#SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -m64 -DDEBUG -std=c++11 -Wall -Wextra -Wfloat-equal -Wundef -Wcast-align -Wwrite-strings -Wlogical-op -Wmissing-declarations -Wredundant-decls -Wshadow -Woverloaded-virtual -Wformat=2  -DPICOJSON_USE_INT64 -O0  -g " )
SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -m64 -DNDEBUG -std=c++11 -Wall -Wextra -Wfloat-equal -Wundef -Wcast-align -Wwrite-strings -Wlogical-op -Wmissing-declarations -Wredundant-decls -Wshadow -Woverloaded-virtual -Wformat=2  -DPICOJSON_USE_INT64 -O3  -g " )

include_directories(/usr/local/include include include/thirdparty /opt/zeromq/include)
link_directories( /opt/zeromq/lib /usr/local/opt/zlib/lib)


add_library(lightq SHARED src/lightq_api.cpp)
TARGET_LINK_LIBRARIES(lightq zmq)
install(TARGETS lightq DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/examples/lib)
install(TARGETS lightq DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/lightq_api.h DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/examples/include)

add_executable(lightq-test examples/lightq-test.cpp)
target_link_libraries(lightq-test zmq z)
install(TARGETS lightq-test DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)

#SET( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -m64 -DDEBUG -std=c11 -Wall -Wextra -Wfloat-equal -Wundef -Wcast-align -Wwrite-strings -Wlogical-op -Wmissing-declarations -Wredundant-decls -Wshadow  -Wformat=2  -O0  -g " )
SET( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -m64 -DNDEBUG -std=c11 -Wall -Wextra -Wfloat-equal -Wundef -Wcast-align -Wwrite-strings -Wlogical-op -Wmissing-declarations -Wredundant-decls -Wshadow  -Wformat=2  -O3  -g " )
find_package (Threads)
link_directories( ${PROJECT_BINARY_DIR}/lib )

add_executable(lightq-broker examples/lightq-broker.c)
target_link_libraries(lightq-broker lightq zmq pthread)
install(TARGETS lightq-broker DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)

add_executable(lightq-consumer examples/lightq-consumer.c)
target_link_libraries(lightq-consumer lightq zmq pthread)
install(TARGETS lightq-consumer DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)

add_executable(lightq-producer examples/lightq-producer.c)
target_link_libraries(lightq-producer lightq zmq pthread)
install(TARGETS lightq-producer DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)

add_executable(lightq-topic examples/lightq-topic.c)
target_link_libraries(lightq-topic lightq zmq pthread)
install(TARGETS lightq-topic DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)




