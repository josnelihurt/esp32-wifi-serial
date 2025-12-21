#pragma once

#include "circular_buffer.hpp"
#include "config.h"
#include <algorithm>
namespace jrb::wifi_serial {

template <size_t SIZE>
class ByteCircularBuffer : public CircularBuffer<uint8_t, SIZE> {
public:
  size_t drainTo(uint8_t *buffer, size_t size) {
    size_t n = this->size();
    if (n == 0 || size == 0)
      return 0;

    size_t toCopy = std::min(n, size);
    size_t first = std::min(toCopy, SIZE - this->tail);
    memcpy(buffer, &this->buffer[this->tail], first);

    size_t second = toCopy - first;
    if (second > 0) {
      memcpy(buffer + first, this->buffer.data(), second);
    }

    this->clear();
    return toCopy;
  }
};

using SerialLog = ByteCircularBuffer<SERIAL_LOG_SIZE>;

} // namespace jrb::wifi_serial
