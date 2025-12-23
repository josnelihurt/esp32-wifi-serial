#include "ssh_flush_policy.h"
#include "ssh_server.h"

namespace jrb::wifi_serial {

SshFlushPolicy::SshFlushPolicy(SSHServer &sshServer, const char *name)
    : sshServer(sshServer), name(name) {}

void SshFlushPolicy::flush(const types::span<const uint8_t> &buffer,
                           const char *name) {
  sshServer.sendToSSHClients(buffer);
}

} // namespace jrb::wifi_serial