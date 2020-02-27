cmake_minimum_required(VERSION 3.10)
project(networksystems)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake_modules")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -g")

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)
find_package(SFML REQUIRED system window graphics network audio)

add_executable(server server.cpp constants.h crypto.cpp crypto.h graph.cpp graph.h graphRenderer.cpp)
add_executable(node client.cpp constants.h communicator.cpp communicator.h packet.cpp packet.h crypto.cpp crypto.h transportPacket.cpp transportPacket.h pingPacket.cpp pingPacket.h connectivity.cpp connectivity.h ringBuffer.cpp ringBuffer.h routeInfoPacket.cpp routeInfoPacket.h graph.cpp graph.h packetIdentification.h ui.cpp ui.h blockingQueue.h TcpConnector.cpp TcpConnector.h graphRenderer.h graphRenderer.cpp safequeue.h)
add_executable(SFML SFML.cpp)

if (SFML_FOUND)
    include_directories(${SFML_INCLUDE_DIR})
    target_link_libraries(SFML ${SFML_LIBRARIES})
    target_link_libraries(server Threads::Threads ${SFML_LIBRARIES})
    target_link_libraries(node Threads::Threads ${SFML_LIBRARIES})
endif()
