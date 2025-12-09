#include <Arduino.h>
#include "dependency_container.h"

namespace jrb::wifi_serial {

static DependencyContainer container;

}  // namespace jrb::wifi_serial

void setup() {
  jrb::wifi_serial::container.initialize();
  jrb::wifi_serial::container.setup();
}

void loop() {
  jrb::wifi_serial::container.loop();
}
