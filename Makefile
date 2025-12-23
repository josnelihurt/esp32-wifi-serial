# ESP32 WiFi Serial Bridge - Makefile
# Usage:
#   make upload-fs IP=192.168.1.100    # Upload frontend files only via OTA
#   make upload-ota IP=192.168.1.100   # Upload full firmware via OTA
#   make upload                         # Upload via USB (default)
#   make build                          # Build firmware only
#   make monitor                        # Monitor serial output

# Default IP address (can be overridden: make upload-fs IP=192.168.1.50)
IP ?= 192.168.4.1
PASSWORD ?= password

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
	@echo "  Local Web Development:"
	@echo "    make dev-web         - Start mock server for frontend development"
	@echo "    make dev-setup       - Install mock server dependencies"
	@echo ""
	@echo "  Build & Upload via USB:"
	@echo "    make build           - Build firmware"
	@echo "    make build-fs        - Build filesystem binary (.bin file)"
	@echo "    make upload          - Upload firmware via USB cable"
	@echo "    make upload-fs       - Upload filesystem (HTML/CSS/JS) via USB"
	@echo "    make upload-all      - Upload firmware + filesystem via USB"
	@echo ""
	@echo "  Over-The-Air (OTA) Updates:"
	@echo "    make upload-ota IP=<ip>     - Upload firmware via OTA"
	@echo "    make upload-fs-ota IP=<ip>  - Upload frontend files via OTA (faster)"
	@echo ""
	@echo "  Testing & Coverage:"
	@echo "    make test            - Run googletest tests (native)"
	@echo "    make coverage        - Run tests + generate HTML coverage report"
	@echo "    make view-coverage   - Generate coverage + open in browser"
	@echo "    make clean-coverage  - Clean coverage files"
	@echo ""
	@echo "  Monitoring & Debugging:"
	@echo "    make monitor         - Monitor serial output"
	@echo "    make clean           - Clean build files"
	@echo ""
	@echo "  Examples:"
	@echo "    make dev-web                          # Start local dev server"
	@echo "    make upload-fs-ota IP=192.168.1.100   # Update frontend only"
	@echo "    make upload-ota IP=192.168.1.100      # Update full firmware"
	@echo ""
	@echo "  Current IP: $(IP)"

.PHONY: build
build:
	@echo "Building firmware..."
	$(PIO) run

.PHONY: build-fs
build-fs:
	@echo "Building filesystem binary..."
	$(PIO) run --target buildfs
	@echo ""
	@echo "Filesystem binary created at:"
	@find .pio/build -name "littlefs.bin" -o -name "spiffs.bin" 2>/dev/null || echo "  Check .pio/build/esp32-c3/ directory"

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

.PHONY: coverage
coverage: test
	@echo "=========================================="
	@echo "Generating comprehensive coverage report"
	@echo "=========================================="
	@echo ""
	@echo "[1/6] Capturing test execution coverage..."
	@lcov --capture --directory .pio/build/native --output-file coverage.info \
		--ignore-errors gcov,gcov,mismatch >/dev/null 2>&1
	@echo "[2/6] Extracting production code coverage..."
	@lcov --extract coverage.info '*/src/*' --output-file coverage.production.info >/dev/null 2>&1
	@echo "[3/6] Filtering out test and library code..."
	@lcov --remove coverage.production.info '*/googletest/*' '*/span-lite/*' '*/test/*' \
		--output-file coverage.tested.info >/dev/null 2>&1 || cp coverage.production.info coverage.tested.info
	@echo "[4/6] Adding zero-coverage baseline for untested files..."
	@./generate_zero_coverage.sh coverage.tested.info coverage.complete.info
	@echo "[5/6] Final filtering..."
	@lcov --remove coverage.complete.info '*/test/*' \
		--output-file coverage.filtered.info >/dev/null 2>&1 || cp coverage.complete.info coverage.filtered.info
	@echo "[6/6] Generating HTML report..."
	@genhtml coverage.filtered.info --output-directory coverage_report \
		--demangle-cpp --title "ESP32 WiFi Serial - Code Coverage" \
		--legend >/dev/null 2>&1
	@echo ""
	@echo "=========================================="
	@echo "Coverage Report Summary"
	@echo "=========================================="
	@lcov --summary coverage.filtered.info 2>/dev/null | grep -E "lines|functions" || echo "  No coverage data available"
	@echo ""
	@echo "Report location: coverage_report/index.html"
	@echo ""
	@echo "Note: This report includes ALL source files."
	@echo "      Files at 0% are ESP-coupled and need"
	@echo "      integration testing or policy abstraction."
	@echo "=========================================="
	@echo ""
	@echo "Generating git-trackable coverage snapshot..."
	@./scripts/generate_coverage_summary.sh

.PHONY: clean-coverage
clean-coverage:
	@echo "Cleaning coverage files..."
	@rm -f coverage.info coverage.production.info coverage.tested.info
	@rm -f coverage.complete.info coverage.filtered.info
	@rm -rf coverage_report
	@echo "Coverage files cleaned!"

.PHONY: view-coverage
view-coverage: coverage
	@echo "Opening coverage report in browser..."
	@./open_coverage.sh

# Local web development with mock server
.PHONY: dev-setup
dev-setup:
	@echo "Installing mock server dependencies..."
	@cd mock-server && npm install
	@echo "Mock server dependencies installed!"
	@echo "Run 'make dev-web' to start the development server"

.PHONY: dev-web
dev-web:
	@echo "Starting mock development server..."
	@echo "Visit http://localhost:3000 in your browser"
	@echo "Default credentials: admin / admin123"
	@echo ""
	@cd mock-server && npm run dev

.PHONY: dev-web-open
dev-web-open:
	@echo "Starting mock development server and opening browser..."
	@cd mock-server && npm run dev:open
