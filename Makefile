# ESP32 WiFi Serial Bridge - Makefile
# Usage:
#   make upload-fs IP=192.168.1.100    # Upload frontend files only via OTA
#   make upload-ota IP=192.168.1.100   # Upload full firmware via OTA
#   make upload                         # Upload via USB (default)
#   make build                          # Build firmware only
#   make monitor                        # Monitor serial output

# Default IP address (can be overridden: make upload-fs IP=192.168.1.50)
IP ?= 192.168.4.1

# PlatformIO commands
PIO = pio

# Default target
.DEFAULT_GOAL := help

CLANG_FORMAT := clang-format
SRC_DIR := src
FORMAT_FILES := $(shell find $(SRC_DIR) \
  \( -name '*.c' -o -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \))

.PHONY: help
help:
	@echo "ESP32 WiFi Serial Bridge - Available Commands:"
	@echo ""
	@echo "  Build & Upload via USB:"
	@echo "    make build           - Build firmware"
	@echo "    make upload          - Upload firmware via USB cable"
	@echo "    make upload-fs       - Upload filesystem (HTML/CSS/JS) via USB"
	@echo "    make upload-all      - Upload firmware + filesystem via USB"
	@echo ""
	@echo "  Over-The-Air (OTA) Updates:"
	@echo "    make upload-ota IP=<ip>     - Upload firmware via OTA"
	@echo "    make upload-fs-ota IP=<ip>  - Upload frontend files via OTA (faster)"
	@echo ""
	@echo "  Monitoring & Debugging:"
	@echo "    make monitor         - Monitor serial output"
	@echo "    make clean           - Clean build files"
	@echo "    make test            - Run googletest tests (native)"
	@echo ""
	@echo "  Examples:"
	@echo "    make upload-fs-ota IP=192.168.1.100   # Update frontend only"
	@echo "    make upload-ota IP=192.168.1.100      # Update full firmware"
	@echo ""
	@echo "  Current IP: $(IP)"

.PHONY: build
build:
	@echo "Building firmware..."
	$(PIO) run

.PHONY: upload
upload:
	@echo "Uploading firmware via USB..."
	$(PIO) run --target upload

.PHONY: upload-fs
upload-fs:
	@echo "Uploading filesystem (HTML/CSS/JS) via USB..."
	$(PIO) run --target uploadfs

.PHONY: upload-all
upload-all:
	@echo "Uploading firmware and filesystem via USB..."
	$(PIO) run --target uploadfs
	$(PIO) run --target upload

.PHONY: upload-ota
upload-ota:
	@echo "Uploading firmware via OTA to $(IP)..."
	@echo "Note: Device must be connected to WiFi (not in AP mode)"
	$(PIO) run --target upload --upload-port $(IP)

.PHONY: upload-fs-ota
upload-fs-ota:
	@echo "Uploading filesystem (frontend files) via OTA to $(IP)..."
	@echo "Note: This only updates HTML/CSS/JS files, not firmware"
	@echo "Note: Device must be connected to WiFi (not in AP mode)"
	$(PIO) run --target uploadfs --upload-port $(IP)

.PHONY: monitor
monitor:
	@echo "Starting serial monitor (Ctrl+C to exit)..."
	$(PIO) device monitor

.PHONY: clean
clean:
	@echo "Cleaning build files..."
	$(PIO) run --target clean

.PHONY: format
format:
	@echo "Running clang-format on $(words $(FORMAT_FILES)) files..."
	@$(CLANG_FORMAT) -i $(FORMAT_FILES)

.PHONY: size
size:
	@echo "Checking firmware size..."
	$(PIO) run --target size

.PHONY: test
test:
	@echo "Running googletest tests (native environment)..."
	$(PIO) test -e native
