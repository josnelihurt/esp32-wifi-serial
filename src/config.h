#pragma once

namespace jrb::wifi_serial {

#define TRIPLE_PRESS_TIMEOUT 2000

#define SERIAL_BUFFER_SIZE 4096
#define SERIAL_LOG_SIZE 4096

#define CMD_PREFIX 0x19 // Ctrl+Y
#define CMD_INFO 'i'
#define CMD_DEBUG 'd'
#define CMD_TTY0_TO_TTY1_BRIDGE 'b'
#define CMD_RESET 0x0E
#define CMD_DISCONNECT_SSH 'x'
#define LED_PIN 8
#define BOOT_BUTTON_PIN 9

#define SERIAL0_BAUD 115200

#define SERIAL1_RX_PIN 0
#define SERIAL1_TX_PIN 1

#define BUTTON_DEBOUNCE_MS 50
#define MQTT_RECONNECT_INTERVAL 5000

#define MQTT_BUFFER_SIZE 1024
#define MQTT_PUBLISH_BUFFER_SIZE 256
#define MQTT_PUBLISH_INTERVAL_MS 5000
#define MQTT_PUBLISH_MIN_CHARS 64
#define DEFAULT_DEVICE_NAME "esp32c3"
#define DEFAULT_BAUD_RATE_TTY1 115200
#define DEFAULT_MQTT_PORT 1883
#define DEFAULT_MQTT_BROKER ""

#define DEFAULT_TOPIC_TTY0 "wifi_serial/%s/ttyS0"
#define DEFAULT_TOPIC_TTY1 "wifi_serial/%s/ttyS1"

#define HTTP_PORT 80

} // namespace jrb::wifi_serial
