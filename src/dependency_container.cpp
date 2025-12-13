#include "dependency_container.h"
#include "system_info.h"
#include "infrastructure/hardware/serial_command_handler.h"
#include "infrastructure/hardware/button_handler.h"
#include "ota_manager.h"
#include "infrastructure/web/web_config_server.h"
#include <Arduino.h>
#include <ArduinoLog.h>

namespace jrb::wifi_serial {

DependencyContainer::DependencyContainer() {
    Log.traceln(__PRETTY_FUNCTION__);
}

DependencyContainer::~DependencyContainer() {
    delete serialCmdHandler;
    delete buttonHandler;
    delete otaManager;
    delete webServer;
}

void DependencyContainer::createHandlers(std::function<void(int, const String&)> webToSerialCallback) {
    Log.traceln(__PRETTY_FUNCTION__);
    preferencesStorage.load();
    systemInfo.logSystemInformation();
    serialBridge.setup(preferencesStorage.baudRateTty1);
    serialBridge.setLogs(serial0Log, serial1Log);
    serialCmdHandler = new SerialCommandHandler(
        preferencesStorage, &mqttClient, debugEnabled,
        [this]() { systemInfo.logSystemInformation(); }
    );
    buttonHandler = new ButtonHandler(
        [this]() { systemInfo.logSystemInformation(); }
    );
    otaManager = new OTAManager(preferencesStorage, otaEnabled);
    webServer = new WebConfigServer(
        preferencesStorage, serial0Log, serial1Log,
        webToSerialCallback
    );
}

}  // namespace jrb::wifi_serial

