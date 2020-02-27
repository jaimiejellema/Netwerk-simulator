//
// Created by Jaimie on 2018-07-22.
//

#ifndef NETWORKSYSTEMS_PACKET_H
#define NETWORKSYSTEMS_PACKET_H


#include <cstdint>

class Communicator;

#include "constants.h"

enum PayloadIdentifier {
    PING,
    ROUTING_DATA,
    TRANSPORT_DATA,
};

struct packet_header {
    ip_t source_address;
    ip_t proxy_address;         //Because all packets are send to all connected nodes, proxy is the node which is intended to forward it.
    ip_t destination_address;
    uint16_t sender_nonce;
    uint16_t payload_size;
    uint8_t payload_type;

    //datagram_header payload_header;
};


class Packet {
public:
    Packet(Communicator* comm, packet_header header);
    virtual ~Packet();

    /*
    struct __attribute__((packed)) ping_packet {
        uint32_t data;
    };

    struct __attribute__((packed)) neighbourhood_packet {
        uint32_t data;
    };
    */

    virtual void process();
    virtual int getSize();
    virtual char* getData();
    packet_header getPacketHeader();
    virtual void updateIPheader(packet_header h);

protected:
    Communicator* comm;
    packet_header IP_header;
    char* data;
};


#endif //NETWORKSYSTEMS_PACKET_H
