#include "triple_press_check_setup_task.h"

namespace jrb::wifi_serial {

void TriplePressCheckSetupTask::setup() {
    if (buttonHandler.checkTriplePress()) {
        Serial.println("Triple press detected! Resetting configuration...");
        if (resetFunc) resetFunc();
        delay(1000);
        ESP.restart();
    }
}

}  // namespace jrb::wifi_serial

