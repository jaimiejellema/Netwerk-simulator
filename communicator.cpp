//
// Created by jaimie on 2018-09-14.
//

#include "communicator.h"

#include "constants.h"
#include "pingPacket.h"
#include "connectivity.h"
#include "transportPacket.h"
#include "TcpConnector.h"
#include "routeInfoPacket.h"
#include "safequeue.h"



#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <netinet/tcp.h>
#include <mutex>
#include <iostream>
#include <thread>

std::mutex mutex_sendData;

Communicator::Communicator(ip_t address) : address(address) {
    application_list = {};
    connectivity = new Connectivity(address);
    logFile.open("client_" + std::to_string(address) + ".txt", std::ofstream::trunc);
    connectivity->renderThread();
}

Communicator::~Communicator() {
    delete connectivity;
}

uint16_t Communicator::getNonce() {
    sender_nonce++;
    return sender_nonce;
}

Connectivity* Communicator::getConnectivity() {
    return connectivity;
}

int Communicator::connectWithServer() {
    client_socket = 0;
    struct sockaddr_in serv_addr;
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr <<"Socket creation error" << std::endl;
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SIMULATOR_PORT_NUMBER);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, SERVER_IP_ADDRESS, &serv_addr.sin_addr)<=0) {
        std::cerr << "Invalid server address / Address not supported" << std::endl;
        return -1;
    }


    int flag = 1;
    if( setsockopt(client_socket, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int)) ) {
        perror("setsockopt client");
        exit(EXIT_FAILURE);
    }

    if (connect(client_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection to server Failed" << std::endl;
        return -1;
    }


    //transmit ID so server knows which client is this connection
    char id[2];
    id[0] = (address >> 8) & 0xFF;
    id[1] = (address >> 0) & 0xFF;
    send(client_socket, id, 2, 0);

    logFile << "Hello message sent " << address << std::endl;
    std::cout << "Hello message sent " << address << std::endl;

    return 0;
}

int Communicator::receivePacket(Communicator* comm) {
    int remaining_length = 0;
    int result;
    char *payload = nullptr;

    packet_header packet_header;
    Packet* ptr;


    //Use header to determine length of packet and keep receiving until you have got full packet.
    result = receiveData(&packet_header, sizeof(packet_header));

    if (result == 1) {

        remaining_length = packet_header.payload_size;
        if (DEBUG_MODE) {
            comm->logFile << "Remaining packet length: " << remaining_length << std::endl;
        }

        if (remaining_length > 100) {
            logFile << "Large packet detected" << std::endl;
        }

        if (remaining_length > 0) {
            payload = new char[remaining_length];
            result = receiveData(payload, remaining_length);

            if (result != 1) {
                free(payload);
            }
        }
    }

    //Something went wrong while receiving data abort;
    if (result != 1) {
        return 0;
    }


    if (DEBUG_MODE) {
        comm->logFile << "Checking for duplicates" << std::endl;
    }

    //Check if this packet has been received before, which would indicate a fault in network graph, drop packet in that case
    PacketIdentification meta = PacketIdentification(packet_header.source_address, packet_header.sender_nonce);


    //Assume after timeout it's not a duplicate, but reuse of identification
    std::chrono::time_point<std::chrono::system_clock> currentTime = std::chrono::system_clock::now();

    for (auto p = received_packets.cbegin(); p != received_packets.cend();) {
        std::chrono::milliseconds duration = std::chrono::duration_cast<std::chrono::milliseconds >(
                currentTime - p->second
        );

        if (duration.count() > DUPLICATE_PACKET_TIMEOUT) {
            p = received_packets.erase(p);
        } else {
            p++;
        }
    }

    if (received_packets.find(meta) == received_packets.end()) {
        received_packets[meta] = std::chrono::system_clock::now();
    } else {
        return result;
    }

    if (DEBUG_MODE) {
        comm->logFile << "Not a duplicate, yay" << std::endl;
    }

    bool supported_packet = false;

    switch(packet_header.payload_type) {
        case PING:
            comm->logFile << "Detected Ping packet" << std::endl;
            ptr = new PingPacket(comm, packet_header);
            supported_packet = true;
            break;
        case ROUTING_DATA:
            comm->logFile << "Detected Routing data packet" << std::endl;
            ptr = new RouteInfoPacket(comm, packet_header, payload);
            supported_packet = true;
            break;
        case TRANSPORT_DATA:
            comm->logFile << "Detected Transport packet" << std::endl;
            ptr = new TransportPacket(comm, packet_header, payload);
            supported_packet = true;
            break;
        default:
            comm->logFile << "Detected unknown packet: " << packet_header.payload_type << std::endl;
            break;
    }
    if (supported_packet) {
        ptr->process();
    }

    //Transport packets do not make a proper copy, and since I don't have time nor energy to fix that.
    //I will just use this.
    if (packet_header.payload_type != TRANSPORT_DATA) {
        free(ptr);
        if (payload != nullptr) {
            free(payload);
        }
    }


    return result;
}

int Communicator::receiveData(void *data, int data_size) {
    char *data_ptr = (char*) data;
    int bytes_received;


    while (data_size > 0) {
        bytes_received = recv(client_socket, data_ptr, data_size, 0);

        if (bytes_received <= 0) {
            logFile << "Bytes received <= 0: " << bytes_received << " expected: " << data_size << std::endl;
            return bytes_received;
        }

        data_ptr += bytes_received;
        data_size -= bytes_received;
    }

    return 1;
}

ip_t Communicator::getAddress() {
    return address;
}

int Communicator::sendData(const void *data, int data_size) {
    const char *data_ptr = (const char*) data;
    int bytes_sent;

    while (data_size > 0) {
        bytes_sent = send(client_socket, data_ptr, data_size, 0);
        if (bytes_sent == -1) {
            return -1;
        }

        data_ptr += bytes_sent;
        data_size -= bytes_sent;
    }

    return 1;
}

int Communicator::sendPacket(Packet* packet) {
    mutex_sendData.lock();
    int status = sendData(packet->getData(), packet->getSize());
    mutex_sendData.unlock();

    return status;
}

SafeQueue<std::string> * Communicator::getTransmitBuffer() {
    return &transmitBuffer;
}

SafeQueue<std::string> * Communicator::getReceiveBuffer() {
    return &receiveBuffer;
}

TcpConnector* Communicator::getTcpSteam(ip_t destination, uint16_t port) {
    if (application_list.find(destination) != application_list.end()) {
        if (application_list[destination].find(port) != application_list[destination].end()) {
            return application_list[destination][port];
        } else {
            TcpConnector* connector = new TcpConnector(this, address, destination);
            application_list[destination].insert({port, connector});
            return connector;
        }
    } else {
        std::unordered_map<uint16_t, TcpConnector*> map = {};
        application_list.insert({destination, map});

        TcpConnector* connector = new TcpConnector(this, address, destination);
        application_list[destination].insert({port, connector});
        return connector;
    }

    /*
    if (application_list.find(port) != application_list.end()) {
        return application_list[port];
    } else {
        TcpConnector* connector = nullptr;
        if (address == 1) {
            connector = new TcpConnector(this, 1, 3);
        } else if (address == 3){
            connector = new TcpConnector(this, 3, 1);
        } else {
            connector = new TcpConnector(this, address, 5 - address);
        }
        application_list.insert({port, connector});
        return connector;
    }
    */
}

void Communicator::updateTcp() {
    for (auto &receiverAppList : application_list) {
        for (auto &app : receiverAppList.second) {
            app.second->update();
        }
    }
}