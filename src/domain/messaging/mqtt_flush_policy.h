#pragma once

#include "infrastructure/logging/logger.h"
#include "infrastructure/mqttt/pub_sub_client_policy.h"
#include "infrastructure/types.hpp"

namespace jrb::wifi_serial {
namespace internal {
/**
 * @class MqttFlushPolicy
 * @brief Template-based flush policy for MQTT publishing.
 * @tparam PubSubClientPolicy The PubSubClient policy type (PubSubClient or
 * PubSubClientTest)
 *
 * Zero-cost abstraction using compile-time polymorphism.
 */
template <typename PubSubClientPolicy> class MqttFlushPolicy {
private:
  PubSubClientPolicy &mqttClient;
  const types::string &topic;

public:
  MqttFlushPolicy(PubSubClientPolicy &mqttClient, const types::string &topic);

  void flush(const types::span<const uint8_t> &buffer, const char *name);
};
} // namespace internal

using MqttFlushPolicy = internal::MqttFlushPolicy<PubSubClientPolicy>;

} // namespace jrb::wifi_serial