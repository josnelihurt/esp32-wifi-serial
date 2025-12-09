#include "dependency_container.h"
#include "system_info.h"
#include "handlers/serial_command_handler.h"
#include "handlers/button_handler.h"
#include "ota_manager.h"
#include "web_config_server.h"
#include <Arduino.h>

namespace jrb::wifi_serial {

DependencyContainer::DependencyContainer() {
    serialBridge.setLogs(serial0Log, serial1Log);
}

DependencyContainer::~DependencyContainer() {
    delete systemInfo;
    delete serialCmdHandler;
    delete buttonHandler;
    delete otaManager;
    delete webServer;
}

void DependencyContainer::createHandlers(std::function<void(int, const String&)> webToSerialCallback) {
    systemInfo = new SystemInfo(preferencesStorage, mqttClient, otaEnabled);
    serialCmdHandler = new SerialCommandHandler(
        preferencesStorage, mqttClient, debugEnabled,
        [this]() { systemInfo->printWelcomeMessage(); }
    );
    buttonHandler = new ButtonHandler(
        [this]() { systemInfo->printWelcomeMessage(); }
    );
    otaManager = new OTAManager(preferencesStorage, otaEnabled);
    webServer = new WebConfigServer(
        preferencesStorage, serial0Log, serial1Log,
        webToSerialCallback
    );
}

}  // namespace jrb::wifi_serial

