#include "wifi_config_ap_mode_wait_task.h"

namespace jrb::wifi_serial {

void WiFiConfigAPModeWaitTask::setup() {
    if (wifiConfig.isAPMode()) {
        Serial.println("In AP mode - connect to ESP32-C3-Config and configure");
        while (wifiConfig.isAPMode()) {
            wifiConfig.loop();
            delay(10);
        }
    }
}

}  // namespace jrb::wifi_serial

