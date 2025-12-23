#pragma once

#include "config.h"
#include "infrastructure/logging/logger.h"
#include "infrastructure/types.hpp"
#include <array>
#include <cstdint>

namespace jrb::wifi_serial {

template <typename FlushPolicy, size_t SIZE> class BufferedStream final {
private:
  std::array<uint8_t, SIZE> buffer;
  size_t head{0};
  size_t tail{0};
  size_t size{0};
  FlushPolicy flusher;
  const char *name;

public:
  explicit BufferedStream(FlushPolicy &&flusher_, const char *name_)
      : flusher(flusher_), name(name_) {
    static_assert((SIZE & (SIZE - 1)) == 0,
                  "MQTT_BUFFER_SIZE must be power of two");
  }

  void append(uint8_t byte) {
    // Overflow check
    if (needsFlushForOverflow(1)) {
      flush();
    }

    buffer[head] = byte;
    head = (head + 1) & (SIZE - 1);

    if (full()) {
      LOG_WARN("MQTT buffer overflow");
      tail = (tail + 1) & (SIZE - 1);
    } else {
      size++;
    }

    // Delimiter check
    if (byte == '\n') {
      flush();
    }
  }

  void append(const types::span<const uint8_t> &data) {
    if (data.empty())
      return;

    // Overflow check
    if (needsFlushForOverflow(data.size())) {
      flush();
    }

    for (size_t i = 0; i < data.size(); ++i) {
      append(data[i]);
    }
  }

  void flush() {
    if (empty())
      return;

    if (head > tail || full()) {
      // Contiguous segment
      types::span<const uint8_t> span(&buffer[tail], size);
      flusher.flush(span, name);
    } else {
      // Wrapped segment: send two chunks
      types::span<const uint8_t> span1(&buffer[tail], SIZE - tail);
      flusher.flush(span1, name);
      if (head > 0) {
        types::span<const uint8_t> span2(&buffer[0], head);
        flusher.flush(span2, name);
      }
    }

    tail = head;
    size = 0;
  }

  bool full() const { return size == SIZE; }
  bool empty() const { return size == 0; }

private:
  bool needsFlushForOverflow(size_t dataSize) const {
    return (size + dataSize) > SIZE;
  }
};

} // namespace jrb::wifi_serial