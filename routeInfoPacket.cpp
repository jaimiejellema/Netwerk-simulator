//
// Created by jaimie on 2018-09-19.
//

#include <cstring>
#include <cstdlib>

#include "routeInfoPacket.h"
#include "communicator.h"
#include "connectivity.h"


RouteInfoPacket::RouteInfoPacket(Communicator* comm, packet_header header, std::vector<ip_t> routes) : Packet(comm, header) {
    reachable = routes;

    data = (char*) realloc((void*) data, Packet::getSize() + header.payload_size);
    memcpy(data + Packet::getSize(), &routes[0], header.payload_size);
}

RouteInfoPacket::RouteInfoPacket(Communicator* comm, packet_header header, char *payload) : Packet(comm, header) {
    reachable.reserve(header.payload_size / sizeof(ip_t));
    for(int j = 0; j < header.payload_size; j += 2) {
        reachable.push_back((payload)[j] | (ip_t)payload[j+1] << 8);
    }

    memcpy(data + Packet::getSize(), payload, header.payload_size);
}

void RouteInfoPacket::process() {
    if ((IP_header.destination_address == BROADCAST_ADDRESS) || (IP_header.destination_address == comm->getAddress())) {
        comm->getConnectivity()->routeReceived(IP_header.source_address, reachable);

        //Flood the network with the route data
        comm->sendPacket(this);
    } else {
        return;
    }


}

int RouteInfoPacket::getSize() {
    return Packet::getSize() + IP_header.payload_size;
}

char* RouteInfoPacket::getData() {
    return data;
}