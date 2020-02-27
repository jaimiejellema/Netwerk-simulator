//
// Created by jaimie on 2018-09-18.
//

#ifndef NETWORKSYSTEMS_PINGPACKET_H
#define NETWORKSYSTEMS_PINGPACKET_H

#include "packet.h"

class PingPacket : public Packet {
public:
    PingPacket(Communicator* comm, packet_header header);
    void process();
};


#endif //NETWORKSYSTEMS_PINGPACKET_H
