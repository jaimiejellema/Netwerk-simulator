//
// Created by jaimie on 2018-09-19.
//

#ifndef NETWORKSYSTEMS_ROUTINGPACKET_H
#define NETWORKSYSTEMS_ROUTINGPACKET_H

#include "packet.h"
#include <vector>

//TODO: add methods to convert to network order

class RouteInfoPacket : public Packet {
public:
    RouteInfoPacket(Communicator* comm, packet_header header, std::vector<ip_t> routes);
    RouteInfoPacket(Communicator* comm, packet_header header, char * payload);
    void process();
    int getSize() override;
    char* getData() override;
private:
    std::vector<ip_t> reachable;
};


#endif //NETWORKSYSTEMS_ROUTINGPACKET_H
