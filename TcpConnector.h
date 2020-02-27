//
// Created by jaimie on 7-10-19.
//

#ifndef NETWORKSYSTEMS_TCPCONNECTOR_H
#define NETWORKSYSTEMS_TCPCONNECTOR_H


#include <cstdint>
#include "constants.h"
#include "communicator.h"
#include "transportPacket.h"
#include <fstream>
#include <list>
#include <random>

class TcpConnector {
public:
    enum CONNECTION_STATUS {
        LISTEN,
        SYN_SENT,
        SYN_RECEIVED,
        ESTABLISHED,
        FIN_WAIT_1,
        FIN_WAIT_2,
        CLOSING,
        TIME_WAIT,
        CLOSE_WAIT,
        LAST_ACK
    } connection_status;

    TcpConnector(Communicator* comm, ip_t sender, ip_t receiver);
    ~TcpConnector();

    bool sendPacket(std::unique_ptr<TransportPacket> ptr);
    bool sendData(char* data, int length);
    void receivePacket(std::unique_ptr<TransportPacket> ptr);

    void sendQueuedPackets();

    void update();
    void closeConnection();

private:
    std::ofstream fileData;
    Communicator* comm;

    std::default_random_engine generator;
    std::uniform_int_distribution<uint32_t > distribution;

    ip_t sender;
    ip_t receiver;

    uint16_t source_port;
    uint16_t destination_port;

    uint32_t seq_number_last_received;  //Last number that was delivered to application
    uint32_t seq_number_last_send;      //Used to determine what numbers to expect as acknowledgements
    uint32_t ack_number_last_received;  //Used to determine what packets to retransmit

    struct data_packet_node{
        std::unique_ptr<TransportPacket> data_packet;
        std::chrono::time_point<std::chrono::system_clock> timeSent;
    };

    std::list<std::unique_ptr<TransportPacket>> packets_to_be_send = std::list<std::unique_ptr<TransportPacket>>();
    std::list<data_packet_node> in_flight_list = std::list<data_packet_node>();
    std::list<std::unique_ptr<TransportPacket>> reorder_buffer = std::list<std::unique_ptr<TransportPacket>>();


    packet_header getIPheader();
    transport_header getTransportHeader(uint32_t sequence, uint32_t acknowledgement);
    uint32_t randomSequenceNumber();
    uint32_t getHighestNumberOverlow(uint32_t a, uint32_t b);
    void sendSynPacket();
    void sendSynAckPacket(uint32_t acknowledgement);
    void sendAckPacket(uint32_t sequence, uint32_t acknowledgement);

    void sendPacketImmediately(std::unique_ptr<TransportPacket> pkt);

    void reorderPackets();
    void checkRetransmission();
};


#endif //NETWORKSYSTEMS_TCPCONNECTOR_H
