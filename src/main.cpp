#include "core/application.h"
#include <Arduino.h>
#include <ArduinoLog.h>
#include <driver/uart.h>
#include <hal/uart_ll.h>

static const char *TAG = "wifi_serial";
namespace jrb::wifi_serial {
namespace {
Application *app{nullptr};
}
Application &application() { return *app; }

} // namespace jrb::wifi_serial

void setupHardware() {
  // Configure UART0 with larger RX buffer BEFORE Serial.begin()
  // This prevents hardware buffer overflow at 115200 baud
  const uart_port_t uart_num = UART_NUM_0;
  uart_driver_install(uart_num,
                      2048, // RX buffer: handles 145ms of data at 115200 baud
                      0,    // TX buffer: use HW FIFO only
                      0,    // No event queue
                      NULL, // No event queue handle
                      0);   // No interrupt flags

  Serial.begin(SERIAL0_BAUD);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);
  digitalWrite(LED_PIN, HIGH);
  delay(100);
  Log.traceln(__PRETTY_FUNCTION__);
}
//LOG_LEVEL_VERBOSE
//LOG_LEVEL_INFO
void setup() {
  setupHardware();
  Log.begin(LOG_LEVEL_INFO, &Serial);
  Log.infoln(__PRETTY_FUNCTION__);
  jrb::wifi_serial::app = new jrb::wifi_serial::Application();
  Log.infoln(
      R"(
================================================
Starting application...
================================================)");
  jrb::wifi_serial::application().setup();
}

void loop() { jrb::wifi_serial::application().loop(); }
