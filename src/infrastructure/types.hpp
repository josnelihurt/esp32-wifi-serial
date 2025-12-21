#pragma once

#include <nonstd/span.hpp>
#include <string>
#include <string_view>

namespace jrb::wifi_serial::types {
using string = std::string;
using string_view = std::string_view;
template <typename T> using span = nonstd::span<T>;
} // namespace jrb::wifi_serial::types