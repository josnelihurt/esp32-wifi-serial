#pragma once

#include "ssh_server.h"
#include <nonstd/span.hpp>
#include <vector>

namespace jrb::wifi_serial {

/**
 * @brief Adapter to make SSHServer conform to Subscriber interface
 *
 * Buffers incoming data and forwards to SSH server queue.
 * Provides append() methods required by Broadcaster.
 */
class SSHSubscriber final {
public:
  explicit SSHSubscriber(SSHServer &server);

  void append(uint8_t byte);
  void append(const nonstd::span<const uint8_t> &data);

private:
  SSHServer &sshServer;
  std::vector<uint8_t> buffer;
  static constexpr size_t FLUSH_THRESHOLD = 64; // Flush every 64 bytes

  void flushIfNeeded();
};

} // namespace jrb::wifi_serial
