# Deployment Guide

## Quick Reference

### Frontend-Only Updates (Fast)
When you only modify HTML, CSS, or JavaScript files:

```bash
# Via USB
make upload-fs

# Via OTA (device must be connected to WiFi)
make upload-fs-ota IP=192.168.1.100
```

### Full Firmware Updates
When you modify C++ code:

```bash
# Via USB
make upload-all

# Via OTA (device must be connected to WiFi)
make upload-ota IP=192.168.1.100
```

## Makefile Commands

### Build Commands
- `make build` - Build firmware without uploading
- `make size` - Check firmware size usage

### USB Upload Commands
- `make upload` - Upload firmware only via USB
- `make upload-fs` - Upload frontend files (HTML/CSS/JS) via USB
- `make upload-all` - Upload both firmware and filesystem via USB

### OTA Upload Commands
**Important:** OTA only works when device is connected to WiFi (Station mode), not in AP mode.

- `make upload-ota IP=<ip>` - Upload firmware via OTA
- `make upload-fs-ota IP=<ip>` - Upload frontend files via OTA (faster, recommended for frontend changes)

### Monitoring Commands
- `make monitor` - Open serial monitor (115200 baud)
- `make clean` - Clean build files

### Examples

#### Update frontend after editing HTML/CSS/JS
```bash
# If connected via USB
make upload-fs

# If device is on WiFi at 192.168.1.50
make upload-fs-ota IP=192.168.1.50
```

#### Update firmware after modifying C++ code
```bash
# Via USB (recommended for development)
make upload-all

# Via OTA (for deployed devices)
make upload-ota IP=192.168.1.50
```

#### Using default IP (192.168.4.1 - AP mode)
```bash
make upload-fs-ota  # Uses default IP
```

## File Structure

### Frontend Files (data/)
All web interface files are separated for easy maintenance:

- `data/index.html` - Main page with Configuration, Serial tabs
- `data/style.css` - All CSS styling (3.4KB)
- `data/script.js` - All JavaScript functionality (4KB+)
- `data/ota.html` - OTA update tab content (loaded dynamically)
- `data/about.html` - About tab content (loaded dynamically)

### Template Variables
The following variables are processed server-side in HTML files:

- `%SSID%` - WiFi SSID
- `%PASSWORD_DISPLAY%` - Password display (masked)
- `%DEVICE_NAME%` - Device hostname
- `%MQTT_BROKER%` - MQTT broker address
- `%MQTT_PORT%` - MQTT port number
- `%MQTT_USER%` - MQTT username
- `%IP_ADDRESS%` - Device IP address
- `%TOPIC_TTY0_RX%` - MQTT topic for ttyS0 RX
- `%TOPIC_TTY0_TX%` - MQTT topic for ttyS0 TX
- `%TOPIC_TTY1_RX%` - MQTT topic for ttyS1 RX
- `%TOPIC_TTY1_TX%` - MQTT topic for ttyS1 TX
- `%BAUD_RATE_TTY1%` - Serial1 baud rate
- `%OTA_PASSWORD_STATUS%` - OTA password status

## Typical Development Workflow

### 1. Frontend Development
When working on web interface (HTML/CSS/JS):

```bash
# Edit files in data/
vim data/index.html
vim data/style.css
vim data/script.js

# Upload only frontend files (much faster!)
make upload-fs

# Or via OTA if device is on WiFi
make upload-fs-ota IP=192.168.1.100
```

### 2. Firmware Development
When working on C++ code:

```bash
# Edit C++ files
vim src/infrastructure/web/web_config_server.cpp

# Build and upload via USB
make upload-all

# Monitor serial output
make monitor
```

### 3. Production Deployment
For deployed devices on WiFi:

```bash
# Update everything via OTA
make upload-fs-ota IP=<device-ip>
make upload-ota IP=<device-ip>
```

## Benefits

### Separated Architecture
- **Maintainable**: HTML/CSS/JS separated from C++ code
- **Fast Iteration**: Frontend changes don't require firmware recompilation
- **Version Control**: Easy to track changes in each layer
- **Modular**: OTA and About content in separate files

### Efficient Updates
- **Frontend-only updates**: ~2-3 seconds (filesystem only)
- **Full firmware updates**: ~10 seconds (firmware + filesystem)
- **OTA updates**: Possible remotely without physical access

### Template System
- **Dynamic Content**: Server-side template processing for configuration values
- **Static Assets**: CSS/JS served without template processing for efficiency
- **Partial Loading**: OTA and About tabs loaded on-demand via JavaScript

## Troubleshooting

### OTA Upload Fails
- Verify device is connected to WiFi (not in AP mode)
- Check IP address is correct: `ping <device-ip>`
- Ensure firewall allows port 3232
- Try via hostname: `make upload-ota IP=<device-name>.local`

### Frontend Not Updating
- Clear browser cache (Ctrl+Shift+R)
- Verify filesystem upload succeeded
- Check LittleFS mounted: look for "LittleFS mounted successfully" in serial monitor

### Build Errors
- Clean and rebuild: `make clean && make build`
- Check PlatformIO installation: `pio --version`
- Update dependencies: `pio lib update`
