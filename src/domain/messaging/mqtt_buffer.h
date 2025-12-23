#pragma once

#include "config.h"
#include "infrastructure/memory/buffered_stream.hpp"
#include "mqtt_flush_policy.h"

namespace jrb::wifi_serial {
using MqttLog = BufferedStream<MqttFlushPolicy, MQTT_BUFFER_SIZE>;
} // namespace jrb::wifi_serial