#pragma once

#include <algorithm>
#include <etl/string.h>
#include <nonstd/span.hpp>
#include <string>
#include <string_view>

namespace jrb::wifi_serial::types {
// Standard dynamic string (used for persistent storage)
using string = std::string;
using string_view = std::string_view;

// Span abstraction
template <typename T> using span = nonstd::span<T>;

// Fixed-size stack strings for logging (no heap allocation)
// ETL provides STL-compatible API with compile-time size bounds
template <size_t N> using fixed_string = etl::string<N>;

// Common string sizes for different use cases
using log_string = etl::string<256>;   // Standard logging (debug messages)
using small_string = etl::string<64>;  // Short messages (status codes, labels)
using large_string = etl::string<512>; // Large debug output (stack traces)

/**
 * @brief Factory function to create fixed-size string from byte span
 * @tparam N Maximum string capacity
 * @param data Byte span to convert
 * @return Fixed-size string with safe truncation
 *
 * Zero-cost helper for creating etl::string from byte buffers for logging.
 * Automatically truncates if data exceeds capacity.
 *
 * Usage:
 *   types::span<const uint8_t> data = ...;
 *   auto msg = types::make_log_string(data);
 *   LOG_INFO("Message: %s", msg.c_str());
 */
template <size_t N = 256>
inline etl::string<N>
make_fixed_string(const span<const uint8_t> &data) noexcept {
  size_t len = std::min(data.size(), N);
  return etl::string<N>(reinterpret_cast<const char *>(data.data()), len);
}

// Convenience aliases for common use cases
inline log_string make_log_string(const span<const uint8_t> &data) noexcept {
  return make_fixed_string<256>(data);
}

inline small_string
make_small_string(const span<const uint8_t> &data) noexcept {
  return make_fixed_string<64>(data);
}

inline large_string
make_large_string(const span<const uint8_t> &data) noexcept {
  return make_fixed_string<512>(data);
}

} // namespace jrb::wifi_serial::types