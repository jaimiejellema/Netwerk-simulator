//
// Created by jaimie on 2018-07-22.
//

#ifndef NETWORKSYSTEMS_UTIL_H
#define NETWORKSYSTEMS_UTIL_H

#include <vector>
#include <cstdint>
#include <numeric>

namespace util {
    std::uint_fast32_t crc32checksum(std::vector<uint8_t> data);
    bool crc32verify(std::vector<uint8_t> data, uint32_t checksum);
};

#endif //NETWORKSYSTEMS_UTIL_H
