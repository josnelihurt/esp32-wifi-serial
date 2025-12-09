#include "config_manager_sync_task.h"

namespace jrb::wifi_serial {

void ConfigManagerSyncTask::setup() {
    configManager.deviceName(wifiConfig.getDeviceName());
    configManager.mqttBroker(wifiConfig.getMqttBroker());
    configManager.mqttPort(wifiConfig.getMqttPort());
    configManager.mqttUser(wifiConfig.getMqttUser());
    configManager.mqttPassword(wifiConfig.getMqttPassword());
    
    if (configManager.deviceName().length() == 0) {
        configManager.deviceName(DEFAULT_DEVICE_NAME);
    }
    if (configManager.mqttPort() == 0) {
        configManager.mqttPort(DEFAULT_MQTT_PORT);
    }
    
    // Topics are now generated automatically in PreferencesStorage::generateDefaultTopics()
    // No need to sync from WiFiConfig
}

}  // namespace jrb::wifi_serial

