//
// Created by jaimie on 2018-09-14.
//

#ifndef NETWORKSYSTEMS_COMMUNICATOR_H
#define NETWORKSYSTEMS_COMMUNICATOR_H

class Packet;
class Connectivity;
class TcpConnector;

#include <chrono>
#include <unordered_map>
#include <fstream>


#include "packetIdentification.h"
#include "safequeue.h"

/** This class connects with the server, and handles sending and receiving of packets. */

class Communicator {
public:
    Communicator(ip_t address);
    ~Communicator();

    int connectWithServer();
    int sendPacket(Packet* packet);
    int receivePacket(Communicator *comm);

    std::ofstream logFile;

    SafeQueue<std::string> * getTransmitBuffer();
    SafeQueue<std::string> * getReceiveBuffer();

    /** Gives a sender unique nonce for every packet, only unique in combination with source address.
     * This unique nonce is used to prevend endless bouncing of packets
     * All nodes should ignore a packet which has a source address and nonce it has seen before
     *
     * @return Sender unique nonce for every packet
     */
    uint16_t getNonce();
    ip_t getAddress();
    Connectivity* getConnectivity();
    TcpConnector* getTcpSteam(ip_t destination, uint16_t port);
    void updateTcp();


private:
    int client_socket;
    int receiveData(void *data, int data_size);
    int sendData(const void *data, int data_size);

    uint16_t sender_nonce = 0;

    //TODO: client_address should not be a fixed ID?
    ip_t address;
    Connectivity* connectivity;

    SafeQueue<std::string> transmitBuffer;
    SafeQueue<std::string> receiveBuffer;

    std::unordered_map<PacketIdentification, std::chrono::time_point<std::chrono::system_clock>> received_packets;
    std::unordered_map<ip_t, std::unordered_map<uint16_t, TcpConnector*>> application_list;
};


#endif //NETWORKSYSTEMS_COMMUNICATOR_H
