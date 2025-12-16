#pragma once

#include <Arduino.h>
#include <ArduinoLog.h>
#include "circular_buffer.hpp"
namespace jrb::wifi_serial {


template <size_t SIZE>
class ByteCircularBuffer : public CircularBuffer<uint8_t, SIZE> {
public:
  String toString() {
    if (!this->hasData())
      return "";

    size_t n = this->size();
    String data;
    data.reserve(n);

    for (size_t i = 0; i < n; ++i) {
      // .concat is used insted of += to avoid misbehavior with the compiler
      // when using += with a String and a char.
      data.concat(
          static_cast<char>(this->buffer[(this->tail + i) & (SIZE - 1)]));
    }

    this->clear();
    return data;
  }
};

using SerialLog = ByteCircularBuffer<SERIAL_LOG_SIZE>;

} // namespace jrb::wifi_serial
