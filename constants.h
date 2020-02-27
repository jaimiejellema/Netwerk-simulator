//
// Created by Jaimie on 2018-07-22.
//

#ifndef NETWORKSYSTEMS_CONSTANTS_H
#define NETWORKSYSTEMS_CONSTANTS_H

#include <cstdint>
#include <climits>

//Constant for client-server communication
const uint16_t SIMULATOR_PORT_NUMBER = 25565;
const uint16_t SIMULATOR_MAX_CLIENTS = 256;
const char* const SERVER_IP_ADDRESS = "127.0.0.1";
const uint8_t SERVER_DROP_CHANGE_PERCENT = 20;

//IP protocol
typedef uint16_t ip_t;
const ip_t BROADCAST_ADDRESS = 0xFFFF;
const uint16_t DUPLICATE_PACKET_TIMEOUT = 1000;


//Neighborhood discovery
const uint16_t PING_TRIES = 3;
const uint16_t PING_TIMEOUT_MS = 500;

//Routing
const uint16_t FORGET_NODE_TIMEOUT_MS = 5000;

//Transport protcol
const uint16_t TCP_MAX_PACKET_SIZE = 4096;
const uint16_t TCP_PACKET_BUFFER_AMOUNT = 8;
const uint16_t TCP_RETRANSMIT_TIMEOUT_MS = 1000;

const uint16_t TCP_INITIAL_WINDOW_SIZE = 2;
const uint16_t TCP_MAXIMUM_WINDOW_SIZE = TCP_PACKET_BUFFER_AMOUNT;

const uint16_t READ_SIZE = 1024;
//Bellman Ford
const int MAX_COST_PATH = 10000;

const bool DEBUG_MODE = false;


#endif //NETWORKSYSTEMS_CONSTANTS_H
