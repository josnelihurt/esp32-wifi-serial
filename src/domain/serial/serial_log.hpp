#pragma once

#include "config.h"
#include <Arduino.h>
#include <ArduinoLog.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <nonstd/span.hpp>
#include <type_traits>
namespace jrb::wifi_serial {

/**
 * @brief Generic circular buffer
 */
template <typename T, size_t SIZE> class CircularBuffer {
protected:
  std::array<T, SIZE> buffer;
  size_t head{0};
  size_t tail{0};
  size_t size_field{0};
  bool hasNewData{false};

public:
  CircularBuffer() { static_assert(SIZE > 0, "SIZE must be > 0"); }
  virtual ~CircularBuffer() = default;

  void append(const T &value) {
    if (full()) {
      tail = (tail + 1) & (SIZE - 1); // wrap around
    } else {
      size_field++;
    }
    buffer[head] = value;
    head = (head + 1) &
           (SIZE - 1); // (0xFFFF + 1) & (0xFFFF - 1) = 0x0000 (wrap around)
    hasNewData = true;
  }

  void append(const nonstd::span<const T> &data) {
    for (size_t i = 0; i < data.size(); ++i) {
      append(data[i]);
    }
  }

  bool hasData() const { return hasNewData; }
  bool full() const { return size_field == SIZE; }
  bool empty() const { return size_field == 0; }
  size_t size() const { return size_field; }

  void clear() {
    head = tail = size_field = 0;
    hasNewData = false;
  }

  /**
   * @brief Pop the front element, returns default T if empty
   */
  T popFront() {
    if (empty()) {
      Log.warningln("%s: is empty", __PRETTY_FUNCTION__);
      return T{}; // default-constructed value instead of exception
    }
    T value = buffer[tail];
    tail = (tail + 1) & (SIZE - 1);
    size_field--;
    if (size_field == 0)
      hasNewData = false;
    return value;
  }
};

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
