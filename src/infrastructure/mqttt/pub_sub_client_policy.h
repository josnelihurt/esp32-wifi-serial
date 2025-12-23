#pragma once


#ifdef ESP_PLATFORM
// ESP32 Platform - Use PubSubClient library
#include <PubSubClient.h>
#else
// Test/Native Platform - Use mock PubSubClient
#include "infrastructure/mqttt/pub_sub_client_test.h"
#endif

namespace jrb::wifi_serial {

// Type aliases for convenience
#ifdef ESP_PLATFORM
using PubSubClientPolicy = PubSubClient;
#else
using PubSubClientPolicy = PubSubClientTest;
#endif
} // namespace jrb::wifi_serial