#pragma once

namespace jrb::wifi_serial {

#define DEFAULT_DEVICE_NAME "esp32c3"
#define DEFAULT_BAUD_RATE_TTY1 115200
#define DEFAULT_MQTT_PORT 1883
#define DEFAULT_MQTT_BROKER ""

#define DEFAULT_TOPIC_TTY0 "wifi_serial/%s/ttyS0"
#define DEFAULT_TOPIC_TTY1 "wifi_serial/%s/ttyS1"

}  // namespace jrb::wifi_serial

