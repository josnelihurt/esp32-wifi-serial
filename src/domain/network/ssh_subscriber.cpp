#include "ssh_subscriber.h"

namespace jrb::wifi_serial {

SSHSubscriber::SSHSubscriber(SSHServer &server) : sshServer(server) {
  buffer.reserve(128);
}

void SSHSubscriber::append(uint8_t byte) {
  buffer.push_back(byte);
  flushIfNeeded();
}

void SSHSubscriber::append(const types::span<const uint8_t> &data) {
  if (data.empty())
    return;

  buffer.insert(buffer.end(), data.data(), data.data() + data.size());
  flushIfNeeded();
}

void SSHSubscriber::flushIfNeeded() {
  // Flush on newline or threshold
  if (!buffer.empty() &&
      (buffer.back() == '\n' || buffer.size() >= FLUSH_THRESHOLD)) {
    types::span<const uint8_t> view(buffer.data(), buffer.size());
    sshServer.sendToSSHClients(view);
    buffer.clear();
  }
}

} // namespace jrb::wifi_serial
