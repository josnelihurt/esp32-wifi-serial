# PlatformIO udev rules

```bash
wget -O 99-platformio-udev.rules https://raw.githubusercontent.com/platformio/platformio-core/develop/platformio/assets/system/99-platformio-udev.rules
sudo cp 99-platformio-udev.rules /etc/udev/rules.d/99-platformio-udev.rules
sudo udevadm control --reload-rules
sudo udevadm trigger
```

Disconnect and reconnect your ESP32-C3 after installation.

