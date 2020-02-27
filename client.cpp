//
// Created by Jaimie on 2018-07-22.
//


#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <future>

#include "blockingQueue.h"
#include "constants.h"
#include "communicator.h"
#include "connectivity.h"
#include "packet.h"

#include "ui.h"
#include "transportPacket.h"
#include "TcpConnector.h"

void ping_thread_test(Communicator* comm, std::future<void> futureObj) {
    while (futureObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout) {
        usleep(PING_TIMEOUT_MS * 1000);
        comm->getConnectivity()->sendPingPackets(comm);
        comm->getConnectivity()->sendRoutePackets(comm);
    }
}

void tcp_update_thread(Communicator* comm, std::future<void> futureObj) {
    while (futureObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout) {
        usleep(TCP_RETRANSMIT_TIMEOUT_MS * 1000);
        comm->updateTcp();
    }
}

void read_terminal_thread(Communicator* comm) {
    while (true) {
        std::string input;
        getline(std::cin, input);


        std::size_t spacePos = input.find(" ");

        if (spacePos == std::string::npos) {
            continue;
        }

        std::string firstPart = input.substr(0, spacePos);
        int destination = atoi(firstPart.c_str());

        if (destination == comm->getAddress()) {
            std::cout << "\033[31;1;4mCannot send a message to yourself\033[0m" << std::endl;
        }

        if (destination < 0) {
            break;
        }

        std::string secondPart = input.substr(spacePos + 1);
        const char* data = secondPart.c_str();

        TcpConnector* tcpStream = comm->getTcpSteam(destination,1234);


        char* stringData;
        stringData = (char*) malloc(strlen("data"));
        memcpy(stringData, data, strlen(data));
        tcpStream->sendData(stringData, strlen(data));

        /*
        char* stringData;
        stringData = (char*) malloc(strlen("This is a test string"));
        memcpy(stringData, "This is a test string", strlen("This is a test string"));
        tcpStream->sendData(stringData, strlen("This is a test string"));

        char* stringData2;
        stringData2 = (char*) malloc(strlen("This too is a test string"));
        memcpy(stringData2, "This too is a test string", strlen("This too is a test string"));
        tcpStream->sendData(stringData2, strlen("This too is a test string"));
        */
    }

}

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        std::cerr << "Please provide a client number" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string str(argv[1]);
    int client_id;
    if (std::all_of(str.begin(), str.end(), ::isdigit)) {
        client_id = std::stoi(argv[1]);
    } else {
        exit(-1);
    }

    Communicator* comm = new Communicator(client_id);
    if (comm->connectWithServer() != 0) {
        exit(EXIT_FAILURE);
    }

    // Create a std::promise object
    std::promise<void> exitSignalPing;
    std::future<void> futureObjPing = exitSignalPing.get_future();

    std::promise<void> exitSignalTcp;
    std::future<void> futureObjTcp = exitSignalTcp.get_future();

    std::thread ping_thread(ping_thread_test, comm, std::move(futureObjPing));
    std::thread tcp_thread(tcp_update_thread, comm, std::move(futureObjTcp));

    std::thread read_thread(read_terminal_thread, comm);
    //TODO: graphrenderer
    //std::thread render_thread(Connectivity::renderThread);

    bool running = true;
    int counter = 0;
    while (running) {
        if (comm->receivePacket(comm) != 1) {
            comm->logFile << "Error while receiving packets, stopping thread" << std::endl;
            running = false;
        }


        counter++;
        if (DEBUG_MODE) {
            comm->logFile << "Counter " << counter << std::endl;
        }
     }
    exitSignalPing.set_value();
    exitSignalTcp.set_value();
    ping_thread.join();
    return 0;
}
