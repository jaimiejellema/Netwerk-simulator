//
// Created by jaimie on 2018-09-21.
//

#ifndef NETWORKSYSTEMS_PACKETIDENTIFICATION_H
#define NETWORKSYSTEMS_PACKETIDENTIFICATION_H

#include <utility>
#include <functional>
#include "constants.h"

typedef std::pair<ip_t, uint16_t> PacketIdentification;

/** Each packet has a unique identifier, this method calculates hash for the unique identifier.
 * This hashfunction is part of std to allow easy use of unordered_maps (hashmaps) with unique identifier as key.
 */

namespace std {
    template <>
    class hash<PacketIdentification>{
    public :
        size_t operator()(const PacketIdentification &packetIdentification ) const {
            size_t res = 17;
            res = res * 31 + packetIdentification.first;
            res = res * 31 + packetIdentification.second;
            return res;
        }
    };
};



#endif //NETWORKSYSTEMS_PACKETIDENTIFICATION_H
