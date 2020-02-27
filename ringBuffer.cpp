//
// Created by jaimie on 2018-09-19.
//

#include "ringBuffer.h"

std::chrono::time_point<std::chrono::system_clock> RingBuffer::read(int index) {
    return buffer[index];
}

void RingBuffer::write(std::chrono::time_point<std::chrono::system_clock> time_stamp) {
    buffer[index++] = time_stamp;
    index = index % PING_TRIES;
}