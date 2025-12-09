#include <Arduino.h>
#include "dependency_container.h"
#include "core/application.h"

namespace jrb::wifi_serial {

static DependencyContainer container;
static Application application(container);

}  // namespace jrb::wifi_serial

void setup() {
  jrb::wifi_serial::application.initialize();
  jrb::wifi_serial::application.setup();
}

void loop() {
  jrb::wifi_serial::application.loop();
}
