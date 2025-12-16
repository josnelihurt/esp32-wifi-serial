#pragma once
#include "config.h"
#include <Arduino.h>
#include <nonstd/span.hpp>
#include <tuple>
#include <utility>

namespace jrb::wifi_serial {

/**
 * @brief Zero-cost broadcaster using variadic templates
 *
 * Compile-time polymorphism with NO runtime overhead:
 * - No std::function (no heap allocation, no function pointers)
 * - No virtual dispatch
 * - All calls inlined by compiler
 * - Subscribers stored as references (zero copy)
 * - Fold expressions expand at compile-time (zero loop overhead)
 *
 * Trade-off: Different types for different subscriber lists
 * (Broadcaster<SerialLog, BufferedStream> != Broadcaster<SerialLog,
 * BufferedStream, SSHSubscriber>)
 *
 * Usage:
 *   SerialLog log;
 *   BufferedStream stream;
 *   Broadcaster<SerialLog, BufferedStream> bc(log, stream);
 *   bc.append(byte);  // Calls log.append(byte), stream.append(byte) - fully
 * inlined!
 */
template <typename... Subscribers> class Broadcaster final {
private:
  std::tuple<std::reference_wrapper<Subscribers>...> subscribers;

public:
  /**
   * @brief Construct broadcaster with references to all subscribers
   *
   * Subscribers must outlive the broadcaster (typically all stack-allocated)
   */
  explicit Broadcaster(Subscribers &...subs) : subscribers(std::ref(subs)...) {}

  /**
   * @brief Broadcast single byte to all subscribers
   *
   * Uses C++17 fold expression - fully inlined, zero overhead
   */
  void append(uint8_t byte) {
    std::apply(
        [byte](auto &...subs) {
          (subs.get().append(byte),
           ...); // Fold expression: expands to sub1.append(byte),
                 // sub2.append(byte), ...
        },
        subscribers);
  }

  /**
   * @brief Broadcast span to all subscribers
   *
   * Uses C++17 fold expression - fully inlined, zero overhead
   */
  void append(const nonstd::span<const uint8_t> &buffer) {
    std::apply(
        [&buffer](auto &...subs) {
          (subs.get().append(buffer),
           ...); // Fold expression: expands to sub1.append(buffer),
                 // sub2.append(buffer), ...
        },
        subscribers);
  }
};

} // namespace jrb::wifi_serial
