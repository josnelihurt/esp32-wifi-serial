#pragma once

#include "infrastructure/types.hpp"

class PubSubClient;
namespace jrb::wifi_serial {

class MqttFlushPolicy {
private:
  PubSubClient &mqttClient;
  const types::string &topic;

public:
  MqttFlushPolicy(PubSubClient &mqttClient, const types::string &topic);
  void flush(const types::span<const uint8_t> &buffer, const char *name);
};

} // namespace jrb::wifi_serial