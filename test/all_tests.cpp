#include "gtest/gtest.h"
#include "infrastructure/types.hpp"

#include "app/application_test.cpp"
#include "app/broadcaster_test.cpp"
#include "domain/config/preferences_storage_policy_test.cpp" // Must come first - defines static storage
#include "domain/config/preferences_storage_test.cpp"
#include "domain/config/special_character_handler_test.cpp"
#include "domain/messaging/buffered_stream_test.cpp"
#include "domain/messaging/mqtt_flush_policy_test.cpp"
#include "domain/network/ssh_server_test.cpp"
#include "domain/network/ssh_subscriber_test.cpp"
#include "domain/serial/serial_log_test.cpp"
#include "infrastructure/hardware/button_handler_test.cpp"
#include "infrastructure/memory/circular_buffer_test.cpp"
#include "infrastructure/mqttt/mqtt_client_test.cpp"
#include "infrastructure/web/web_config_server_test.cpp"
#include "infrastructure/wifi/wifi_manager_test.cpp"

// Root level tests
// TODO: Fix duplicate test registration issue
// #include "ota_manager_test.cpp"
// #include "system_info_test.cpp"

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}