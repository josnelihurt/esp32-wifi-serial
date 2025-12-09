#include "config_manager_sync_task.h"

namespace jrb::wifi_serial {

void ConfigManagerSyncTask::setup() {
    preferencesStorage.deviceName(wifiManager.getDeviceName());
    preferencesStorage.mqttBroker(wifiManager.getMqttBroker());
    preferencesStorage.mqttPort(wifiManager.getMqttPort());
    preferencesStorage.mqttUser(wifiManager.getMqttUser());
    preferencesStorage.mqttPassword(wifiManager.getMqttPassword());
    
    if (preferencesStorage.deviceName().length() == 0) {
        preferencesStorage.deviceName(DEFAULT_DEVICE_NAME);
    }
    if (preferencesStorage.mqttPort() == 0) {
        preferencesStorage.mqttPort(DEFAULT_MQTT_PORT);
    }
    
    // Topics are now generated automatically in PreferencesStorage::generateDefaultTopics()
    // No need to sync from WiFiConfig
}

}  // namespace jrb::wifi_serial

