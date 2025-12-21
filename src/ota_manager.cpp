#include "ota_manager.h"
#include "domain/config/preferences_storage.h"
#include "infrastructure/logging/logger.h"
#include <ArduinoOTA.h>

namespace jrb::wifi_serial {

void OTAManager::setup() {
  ArduinoOTA.setHostname(preferencesStorage.deviceName.c_str());

  if (preferencesStorage.webPassword.length() > 0) {
    ArduinoOTA.setPassword(preferencesStorage.webPassword.c_str());
  }

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {
      type = "filesystem";
    }
    LOG_INFO("OTA: Start updating %s", type.c_str());
  });

  ArduinoOTA.onEnd([]() { LOG_INFO("OTA: Update complete"); });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Log.traceln("OTA: Progress: %u%%", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
    if (error == OTA_AUTH_ERROR) {
      Log.errorln("OTA Error[%u]: Auth Failed", error);
    } else if (error == OTA_BEGIN_ERROR) {
      Log.errorln("OTA Error[%u]: Begin Failed", error);
    } else if (error == OTA_CONNECT_ERROR) {
      Log.errorln("OTA Error[%u]: Connect Failed", error);
    } else if (error == OTA_RECEIVE_ERROR) {
      Log.errorln("OTA Error[%u]: Receive Failed", error);
    } else if (error == OTA_END_ERROR) {
      Log.errorln("OTA Error[%u]: End Failed", error);
    }
  });

  ArduinoOTA.begin();
  otaEnabled = true;
}

} // namespace jrb::wifi_serial
