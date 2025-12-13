## AGENTS.md - Agent Guide for ESP32 WiFi Serial Bridge

### Essential Commands
- **Build**: `make build`
- **Upload via USB**: `make upload`
- **Upload Frontend Files via USB**: `make upload-fs`
- **Full Upload via USB**: `make upload-all`
- **Monitor Serial Output**: `make monitor`
- **OTA Upload Firmware**: `make upload-ota IP=<ip>`
- **OTA Upload Frontend Files**: `make upload-fs-ota IP=<ip>`

### Code Organization
- **src/**: Contains all C++ source files and headers.
  - **domain/**: Core functionality including network, MQTT, WiFi management, and serial communication.
    - **network/**: Handles MQTT client and WiFi manager implementations.
    - **serial/**: Manages serial bridge and logging functionalities.
    - **config/**: Stores configuration preferences and storage handling.
  - **infrastructure/**: Supporting modules like hardware button handlers and web server configurations.
    - **hardware/**: Manages hardware interactions such as button presses and serial command handling.
    - **web/**: Contains web server implementations for the configuration interface.

### Naming Conventions
- **Header Files**: Use `.h` extension, e.g., `mqtt_client.h`, `wifi_manager.h`.
- **Source Files**: Use `.cpp` extension, e.g., `mqtt_client.cpp`, `wifi_manager.cpp`.
- **Configuration Files**: Use snake_case in filenames and constants, e.g., `app_config.h`, `hardware_config.h`.

### Testing Approach
The project doesn't explicitly define test targets in the Makefile. However, it relies on PlatformIO for development and debugging, with serial monitoring (`make monitor`) being a primary method for verification. Functional testing is likely done through the web interface and MQTT communication.

### Important Gotchas
- **OTA Updates**: OTA updates require the device to be connected to WiFi in Station mode, not AP mode.
- **LittleFS Filesystem**: Frontend files are stored using LittleFS; ensure it's mounted successfully for file updates.
- **Serial Port Configuration**: Ensure correct TX/RX pin connections (GPIO 0 and GPIO 1) when interfacing with the ARM device.

### Project-Specific Context
The project is designed for embedded systems, providing a WiFi-to-Serial bridge. It includes features like MQTT integration, web-based configuration, and OTA updates. The deployment involves hardware soldering on specific GPIO pins (GPIO 0, 1, GND) for UART communication with the ARM device.

### Additional Notes
- **Hardware Requirements**: ESP32-C3 board with UART connections to an ARM device (Raspberry Pi/Orange Pi).
- **Web Interface**: Accessible via `http://192.168.4.1` in AP mode or the device's IP in Station mode.
- **Pin Configuration**: Ensure proper soldering of headers on GPIO 0, 1, and GND for UART communication.

### Resources
- **DEPLOYMENT.md**: Detailed deployment instructions and use cases.
- **README.md**: Overview of the project, features, and quick start guide.
- **platformio.ini**: Configuration details for PlatformIO build and upload settings.