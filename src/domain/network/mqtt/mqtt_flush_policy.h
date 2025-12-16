#pragma once

#include <PubSubClient.h>
#include <nonstd/span.hpp>
#include <Arduino.h>

namespace jrb::wifi_serial {

class MqttFlushPolicy {
private:
  PubSubClient &mqttClient;
  const String &topic;

public:
  MqttFlushPolicy(PubSubClient &mqttClient, const String &topic);
  void flush(const nonstd::span<const uint8_t> &buffer, const char *name);
};

} // namespace jrb::wifi_serial