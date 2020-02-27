//
// Created by jaimie on 2018-09-18.
//

#include <cstring>
#include "transportPacket.h"
#include "connectivity.h"
#include "TcpConnector.h"

TransportPacket::TransportPacket(Communicator* comm, packet_header header, char *payload) : Packet(comm, header) {
    data = (char*) realloc((void*) data, Packet::getSize() + header.payload_size);
    memcpy(data + Packet::getSize(), payload, header.payload_size);
    //TODO: verify this struct casting is actually valid
    tcp_data = (transport_header*) ((char*)data + Packet::getSize());
}

TransportPacket::TransportPacket(Communicator *comm, packet_header header, transport_header *transportHeader, char *payload) : Packet(comm, header) {
    data = (char*) realloc((void*) data, Packet::getSize() + header.payload_size);

    tcp_data = (transport_header*) ((char*)data + Packet::getSize());
    memcpy(tcp_data, (void*) transportHeader, sizeof(transport_header));


    //TODO: The following 2 memcopy's are identical
    if ((header.payload_size - sizeof(transport_header)) > 0) {
        memcpy((char*)tcp_data + sizeof(transport_header), payload, header.payload_size - sizeof(transport_header));
    }

    //TODO: verify this memcpy
    if (payload != nullptr) {
        memcpy(data + Packet::getSize() + sizeof(transport_header), payload,
               header.payload_size - sizeof(transport_header));
    }

}

TransportPacket::~TransportPacket() {

}

void TransportPacket::process() {
    comm->logFile << "Received transport packet with destination: " << IP_header.destination_address << " via " << IP_header.proxy_address << std::endl;
    if (IP_header.destination_address == BROADCAST_ADDRESS) {
        //TODO: I should probably process it aswell, instead of just flooding network ;)
        comm->sendPacket(this);

        comm->logFile << "Received broadcast transport packet with port" << tcp_data->destination_port << std::endl;
        TcpConnector* tcpstream = comm->getTcpSteam(IP_header.source_address, tcp_data->destination_port);
        tcpstream->receivePacket(std::unique_ptr<TransportPacket>(this));
        return;
    }

    //Forward packet based on our routing information
    if ((IP_header.destination_address != comm->getAddress()) && (IP_header.proxy_address == comm->getAddress())) {
        char *data = nullptr;
        int length;
        comm->logFile << "About to get contents of" << std::endl;
        getContent(&data, &length);
        if (length != 0) {
            comm->logFile << "Received packet for application with payload" << std::endl;
        } else {
            comm->logFile <<"No data was attached" << std::endl;
        }
        packet_header h = IP_header;

        ip_t nextHop = comm->getConnectivity()->getNextHop(IP_header.destination_address);
        if (nextHop == IP_header.destination_address) {
            h.proxy_address = 0x0000;
        } else {
            h.proxy_address = nextHop;
        }

        std::unique_ptr<Packet> ptr = std::unique_ptr<Packet>(new TransportPacket(comm, h, (char*) tcp_data));
        comm->sendPacket(ptr.get());

        return;
    }

    if (IP_header.destination_address == comm->getAddress()) {
        comm->logFile << "Packet is meant for us, processing it, port: " << tcp_data->destination_port << std::endl;
        TcpConnector* tcpstream = comm->getTcpSteam(IP_header.source_address, tcp_data->destination_port);
        tcpstream->receivePacket(std::unique_ptr<TransportPacket>(this));
    }
}

int TransportPacket::getSize() {
    return Packet::getSize() + IP_header.payload_size;
}

char* TransportPacket::getData() {
    return data;
}

void TransportPacket::getContent(char** payload, int* size) {
    *size = getPacketHeader().payload_size - sizeof(transport_header);
    if (*size > 0){
        *payload = (char*) malloc(*size);
        memcpy(*payload, (void*)((char*)tcp_data + sizeof(transport_header)), *size);
    }
}


transport_header TransportPacket::getTransportHeader() {
    transport_header copy;
    transport_header* original = (transport_header*) tcp_data;
    copy.source_port = original->source_port;
    copy.destination_port = original->destination_port;
    copy.flags = original->flags;
    copy.windowSize = original->windowSize;
    copy.sequence_number = original->sequence_number;
    copy.acknowledgement_number = original->acknowledgement_number;
    return copy;
}