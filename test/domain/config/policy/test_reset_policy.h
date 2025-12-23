/**
 * @file test_reset_policy.h
 * @brief Test policy for device reset operations in SpecialCharacterHandler tests.
 *
 * This policy class provides a testable implementation of reset-related
 * operations for use in native unit tests. It replaces hardware-dependent
 * reset functionality with trackable state for verification.
 *
 * Design Note:
 * This is a duplicate of HardwareResetPolicy's non-ESP implementation.
 * The duplication exists to:
 * 1. Explicitly document the test policy interface
 * 2. Allow future divergence if test-specific behavior is needed
 * 3. Make test dependencies explicit
 *
 * Testing Limitation:
 * The resetPolicy member in SpecialCharacterHandler is private and created
 * by value (not reference), making it impossible to access test helper
 * methods (getTotalDelay, wasResetCalled, etc.) from tests. This is a
 * known design limitation - tests can only verify observable behavior,
 * not internal policy state.
 *
 * @see HardwareResetPolicy in src/domain/config/policy/hardware_reset_policy.h
 */

#pragma once

#include <cstdint>

namespace jrb::wifi_serial {

/**
 * @class TestResetPolicy
 * @brief Reset policy implementation for testing.
 *
 * This policy provides a mock reset implementation for unit testing.
 * Tracks calls and allows test control over serial data availability.
 */
class TestResetPolicy {
private:
  uint32_t totalDelayMs;
  bool serialDataAvailable;
  bool resetCalled;

public:
  TestResetPolicy()
      : totalDelayMs(0), serialDataAvailable(false), resetCalled(false) {}

  void delay(uint32_t ms) { totalDelayMs += ms; }

  bool isSerialDataAvailable() const { return serialDataAvailable; }

  void readSerialData() { serialDataAvailable = false; }

  void resetDevice() { resetCalled = true; }

  // Test helpers (note: currently inaccessible from tests due to private member)
  void setSerialDataAvailable(bool available) {
    serialDataAvailable = available;
  }

  uint32_t getTotalDelay() const { return totalDelayMs; }

  bool wasResetCalled() const { return resetCalled; }

  void reset() {
    totalDelayMs = 0;
    serialDataAvailable = false;
    resetCalled = false;
  }
};

} // namespace jrb::wifi_serial
