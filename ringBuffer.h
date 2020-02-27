//
// Created by jaimie on 2018-09-19.
//

#ifndef NETWORKSYSTEMS_RINGBUFFER_H
#define NETWORKSYSTEMS_RINGBUFFER_H

#include <vector>
#include <chrono>

#include "constants.h"

class RingBuffer {
public:
    void write(std::chrono::time_point<std::chrono::system_clock> time_stamp);
    std::chrono::time_point<std::chrono::system_clock> read(int index);
private:
    std::chrono::time_point<std::chrono::system_clock> buffer[PING_TRIES];
    int index = 0;
};


#endif //NETWORKSYSTEMS_RINGBUFFER_H
