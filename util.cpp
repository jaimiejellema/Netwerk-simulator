//
// Created by jaimie on 2018-07-22.
//

#include "util.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <numeric>

// Generates a lookup table for the checksums of all 8-bit values.
std::array<std::uint_fast32_t, 256> generate_crc_lookup_table() noexcept {
    auto const reversed_polynomial = std::uint_fast32_t{0xEDB88320uL};


    // This is a function object that calculates the checksum for a value,
    // then increments the value, starting from zero.
    struct byte_checksum {
        std::uint_fast32_t operator()() noexcept {
            auto checksum = static_cast<std::uint_fast32_t>(n++);

            for (auto i = 0; i < 8; i++) {
                checksum = (checksum >> 1) ^ ((checksum & 0x1u) ? reversed_polynomial : 0);
            }
            return checksum;
        }

        unsigned n = 0;
    };

    auto table = std::array<std::uint_fast32_t, 256>{};
    std::generate(table.begin(), table.end(), byte_checksum{});

    return table;
}

// Calculates the CRC for any sequence of values.
std::uint_fast32_t util::crc32checksum(std::vector<uint8_t> data) {
    // Generate lookup table only on first use then cache it - this is thread-safe.
    static auto const table = generate_crc_lookup_table();

    // Calculate the checksum - make sure to clip to 32 bits, for systems that don't
    // have a true (fast) 32-bit type.
    return std::uint_fast32_t{0xFFFFFFFFuL} &
           ~std::accumulate(data.begin(), data.end(),
                            ~std::uint_fast32_t{0} & std::uint_fast32_t{0xFFFFFFFFuL},
                            [](std::uint_fast32_t checksum, std::uint_fast8_t value)
                            { return table[(checksum ^ value) & 0xFFu] ^ (checksum >> 8); });
}

bool util::crc32verify(std::vector<uint8_t> data, uint32_t checksum) {
    return crc32checksum(data) == checksum;
}

//int main() {
//    std::vector<uint8_t> data = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};
//    uint_fast32_t checksum = util::crc32checksum(data);
//    comm->logFile << std::hex << std::setw(8) << std::setfill('0') << util::crc32checksum(data) << '\n';
//
//    if (util::crc32verify(data, 0xCBF43926) ) {
//        printf("all good \n");
//    }
//
//    comm->logFile << std::hex << std::setw(8) << std::setfill('0') << util::crc32checksum(data) << '\n';
//}