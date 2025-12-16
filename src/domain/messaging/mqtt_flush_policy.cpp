#include "mqtt_flush_policy.h"
#include <ArduinoLog.h>

namespace jrb::wifi_serial {

MqttFlushPolicy::MqttFlushPolicy(PubSubClient &mqttClient, const String &topic)
    : mqttClient{mqttClient}, topic{topic} {}

void MqttFlushPolicy::flush(const nonstd::span<const uint8_t> &buffer,
                            const char *name) {
  if (buffer.empty())
    return;

  if (!mqttClient.connected())
    return;

  if (topic.length() == 0) {
    Log.errorln("MqttFlushPolicy::flush: topic is empty for buffer '%s'", name);
    return;
  }

  Log.verboseln("MQTT publishing %d bytes to topic: %s", buffer.size(),
                topic.c_str());
  bool result = mqttClient.publish(topic.c_str(), buffer.data(), buffer.size());
  if (!result) {
    Log.errorln("MQTT publish failed for topic: %s", topic.c_str());
  }
}
} // namespace jrb::wifi_serial