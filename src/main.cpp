#include <Arduino.h>
#include <ArduinoLog.h>
#include "dependency_container.h"
#include "core/application.h"

static const char* TAG = "wifi_serial";
namespace jrb::wifi_serial {
namespace {
Application* app{nullptr};
DependencyContainer* container{nullptr};
}
Application& application() {
  return *app;
}

}  // namespace jrb::wifi_serial

void setupHardware() {
  Serial.begin(SERIAL0_BAUD);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);
  digitalWrite(LED_PIN, HIGH);
  delay(100);
  Log.traceln(__PRETTY_FUNCTION__);
}

void setup() {
  setupHardware();
  Log.begin(LOG_LEVEL_INFO, &Serial);
  Log.infoln(__PRETTY_FUNCTION__);
  jrb::wifi_serial::container = new jrb::wifi_serial::DependencyContainer();
  jrb::wifi_serial::app = new jrb::wifi_serial::Application(*jrb::wifi_serial::container);
  Log.infoln(
R"(
================================================
Starting application...
================================================)"
  );
  jrb::wifi_serial::application().setup();
}

void loop() {
  jrb::wifi_serial::application().loop();
}
