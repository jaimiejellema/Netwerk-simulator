//
// Created by jaimie on 2018-09-18.
//

#include "pingPacket.h"

#include "communicator.h"
#include "connectivity.h"
#include "constants.h"

PingPacket::PingPacket(Communicator* comm, packet_header header) : Packet(comm, header) {

}

void PingPacket::process() {
    if ((IP_header.destination_address == BROADCAST_ADDRESS) || (IP_header.destination_address == comm->getAddress())) {
        comm->getConnectivity()->pingReceived(IP_header.source_address);
    } else {
        return;
    }
}
