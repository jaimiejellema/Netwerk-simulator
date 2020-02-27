//
// Created by jaimie on 2018-09-18.
//

#ifndef NETWORKSYSTEMS_TRANSPORTPACKET_H
#define NETWORKSYSTEMS_TRANSPORTPACKET_H

#include <cstdint>
#include "communicator.h"
#include "packet.h"

//TODO: add methods to convert to network order

const uint8_t FLAGS_FIN = 0x01;
const uint8_t FLAGS_SYN = 0x02;
const uint8_t FLAGS_RST = 0x04;
const uint8_t FLAGS_ACK = 0x08;


struct transport_header {
    uint16_t source_port;
    uint16_t destination_port;
    uint32_t sequence_number;
    uint32_t acknowledgement_number;

    //TODO: build window scaling into this variable? like assume try window size is 2^{thisnumber}
    //Or maybe more like Idle oil tycoon scaling? inverse of 2^(log5 (experience))
    //Drop window scaling altogether, congestion is not really possible, I want to finish this project, and it's just added complexity

    uint16_t windowSize;
    uint8_t		flags;
};

class TransportPacket : public Packet {
public:
    TransportPacket(Communicator* comm, packet_header header, char *payload);
    TransportPacket(Communicator* comm, packet_header header, transport_header* transportPacket, char *payload);
    ~TransportPacket();

    TransportPacket(const TransportPacket&) = delete;
    //TransportPacket& operator=(TransportPacket other) = delete;


    void process();
    int getSize() override;
    char* getData() override;
    transport_header getTransportHeader();
    void getContent(char** payload, int* size);
private:
    transport_header* tcp_data;
};


#endif //NETWORKSYSTEMS_TRANSPORTPACKET_H
