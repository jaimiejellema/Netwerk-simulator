//
// Created by jaimie on 2018-07-22.
//

struct packet_header;

#include "communicator.h"
#include "packet.h"
#include <cstdlib>
#include <cstring>

Packet::Packet(Communicator* comm, packet_header header) : comm(comm), IP_header(header) {
    data = (char *) malloc(sizeof(struct packet_header));
    memcpy(data, (void*) &header, Packet::getSize());
}

Packet::~Packet() {
    free(data);
}

void Packet::process() {

}

int Packet::getSize() {
    return sizeof(struct packet_header);
}

char* Packet::getData() {
    return data;
}

packet_header Packet::getPacketHeader() {
    packet_header copy;
    packet_header* original = reinterpret_cast<packet_header*>(data);
    copy.source_address = original->source_address;
    copy.proxy_address = original->proxy_address;
    copy.destination_address = original->destination_address;
    copy.sender_nonce = original->sender_nonce;
    copy.payload_size = original->payload_size;
    copy.payload_type = original->payload_type;
    return copy;
}

void Packet::updateIPheader(packet_header h) {
    memcpy(data, (void*) &h, Packet::getSize());
}