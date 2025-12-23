#include "mqtt_flush_policy.h"
#include "infrastructure/logging/logger.h"

namespace jrb::wifi_serial {
namespace internal {
template <typename PubSubClientPolicy>
MqttFlushPolicy<PubSubClientPolicy>::MqttFlushPolicy(
    PubSubClientPolicy &mqttClient, const types::string &topic)
    : mqttClient{mqttClient}, topic{topic} {}

template <typename PubSubClientPolicy>
void MqttFlushPolicy<PubSubClientPolicy>::flush(
    const types::span<const uint8_t> &buffer, const char *name) {
  if (buffer.empty())
    return;

  if (!mqttClient.connected())
    return;

  if (topic.length() == 0) {
    LOG_ERROR("MqttFlushPolicy::flush: topic is empty for buffer '%s'", name);
    return;
  }

  LOG_VERBOSE("MQTT publishing %d bytes to topic: %s", buffer.size(),
              topic.c_str());
  bool result = mqttClient.publish(topic.c_str(), buffer.data(), buffer.size());
  if (!result) {
    LOG_ERROR("MQTT publish failed for topic: %s", topic.c_str());
  }
}
} // namespace internal
// Explicit instantiation for production and test builds
#ifdef ESP_PLATFORM
template class internal::MqttFlushPolicy<PubSubClient>;
#else
template class internal::MqttFlushPolicy<PubSubClientTest>;
#endif
} // namespace jrb::wifi_serial