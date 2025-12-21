#pragma once

#include "infrastructure/memory/buffered_stream.hpp"
#include "ssh_flush_policy.h"

namespace jrb::wifi_serial {

using SshLog = BufferedStream<SshFlushPolicy, 64>; // 64 bytes is enough for SSH

} // namespace jrb::wifi_serial