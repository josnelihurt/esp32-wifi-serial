#pragma once

#include "infrastructure/types.hpp"

namespace jrb::wifi_serial {

class SSHServer;

class SshFlushPolicy final {
private:
  SSHServer &sshServer;
  const char *name;
public:
  explicit SshFlushPolicy(SSHServer &sshServer, const char *name);
  ~SshFlushPolicy() = default;
  void flush(const types::span<const uint8_t> &buffer, const char *name = "ssh");
};

} // namespace jrb::wifi_serial