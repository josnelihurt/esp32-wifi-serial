#include "infrastructure/memory/circular_buffer.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <vector>

namespace jrb::wifi_serial {
namespace {

// Matcher to verify buffer state with detailed error reporting
MATCHER_P4(BufferHasExpectedState, expectedSize, expectedHasData, expectedFull,
           expectedEmpty, "Circular buffer should have specific state") {
  const auto &buffer = arg;

  bool success = true;

  if (buffer.size() != expectedSize) {
    *result_listener << " where size expected " << expectedSize << " but is "
                     << buffer.size();
    success = false;
  }

  if (buffer.hasData() != expectedHasData) {
    *result_listener << " where hasData expected " << expectedHasData
                     << " but is " << buffer.hasData();
    success = false;
  }

  if (buffer.full() != expectedFull) {
    *result_listener << " where full expected " << expectedFull << " but is "
                     << buffer.full();
    success = false;
  }

  if (buffer.empty() != expectedEmpty) {
    *result_listener << " where empty expected " << expectedEmpty << " but is "
                     << buffer.empty();
    success = false;
  }

  return success;
}

// ============================================================================
// Test Parameter Structures (using designated initializers)
// ============================================================================

struct BufferStateTest {
  std::vector<int> values;
  std::size_t expected_size;
  bool expected_has_data;
  bool expected_full;
  bool expected_empty;
};

struct SpanAppendTest {
  std::vector<int> span_data;
  std::size_t expected_size;
  bool expected_has_data;
  bool expected_full;
  bool expected_empty;
};

struct WraparoundTest {
  std::size_t initial_fill;
  std::size_t pop_count;
  std::size_t append_count;
  std::vector<int> append_values;
};

// ============================================================================
// Basic Operations Tests
// ============================================================================

class BasicOperationsTest : public ::testing::TestWithParam<BufferStateTest> {};

INSTANTIATE_TEST_SUITE_P(
    CircularBuffer, BasicOperationsTest,
    ::testing::Values(
        // Initial state
        BufferStateTest{.values = {},
                        .expected_size = 0,
                        .expected_has_data = false,
                        .expected_full = false,
                        .expected_empty = true},
        // Single element
        BufferStateTest{.values = {42},
                        .expected_size = 1,
                        .expected_has_data = true,
                        .expected_full = false,
                        .expected_empty = false},
        // Multiple elements
        BufferStateTest{.values = {1, 2, 3, 4, 5},
                        .expected_size = 5,
                        .expected_has_data = true,
                        .expected_full = false,
                        .expected_empty = false},
        // Fill buffer completely
        BufferStateTest{.values = {0, 1, 2, 3, 4, 5, 6, 7},
                        .expected_size = 8,
                        .expected_has_data = true,
                        .expected_full = true,
                        .expected_empty = false}));

TEST_P(BasicOperationsTest, AppendAndVerifyState) {
  CircularBuffer<int, 8> buffer;
  const auto &param = GetParam();

  for (auto value : param.values) {
    buffer.append(value);
  }

  EXPECT_THAT(buffer, BufferHasExpectedState(param.expected_size,
                                             param.expected_has_data,
                                             param.expected_full,
                                             param.expected_empty));

  for (auto value : param.values) {
    EXPECT_EQ(buffer.popFront(), value);
  }

  EXPECT_THAT(buffer, BufferHasExpectedState(0, false, false, true));
}


// ============================================================================
// Span Append Tests
// ============================================================================

class SpanAppendTestSuite : public ::testing::TestWithParam<SpanAppendTest> {};

INSTANTIATE_TEST_SUITE_P(
    CircularBuffer, SpanAppendTestSuite,
    ::testing::Values(
        // Empty span
        SpanAppendTest{.span_data = {},
                       .expected_size = 0,
                       .expected_has_data = false,
                       .expected_full = false,
                       .expected_empty = true},
        // Single element
        SpanAppendTest{.span_data = {100},
                       .expected_size = 1,
                       .expected_has_data = true,
                       .expected_full = false,
                       .expected_empty = false},
        // Multiple elements
        SpanAppendTest{.span_data = {1, 2, 3, 4, 5},
                       .expected_size = 5,
                       .expected_has_data = true,
                       .expected_full = false,
                       .expected_empty = false},
        // Exactly fills buffer
        SpanAppendTest{.span_data = {0, 1, 2, 3, 4, 5, 6, 7},
                       .expected_size = 8,
                       .expected_has_data = true,
                       .expected_full = true,
                       .expected_empty = false},
        // Larger than buffer (overwrites oldest)
        SpanAppendTest{.span_data = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                       .expected_size = 8,
                       .expected_has_data = true,
                       .expected_full = true,
                       .expected_empty = false}));

TEST_P(SpanAppendTestSuite, AppendSpanAndVerifyState) {
  CircularBuffer<int, 8> buffer;
  const auto &param = GetParam();

  types::span<const int> span(param.span_data.data(), param.span_data.size());
  buffer.append(span);

  EXPECT_THAT(buffer, BufferHasExpectedState(param.expected_size,
                                             param.expected_has_data,
                                             param.expected_full,
                                             param.expected_empty));

  // Verify FIFO order - for larger spans, only check last SIZE elements
  std::size_t start_idx =
      param.span_data.size() > 8 ? param.span_data.size() - 8 : 0;
  for (std::size_t i = start_idx; i < param.span_data.size(); ++i) {
    EXPECT_EQ(buffer.popFront(), param.span_data[i]);
  }

  EXPECT_THAT(buffer, BufferHasExpectedState(0, false, false, true));
}

// ============================================================================
// State Operations Tests (popFront, clear)
// ============================================================================

TEST(StateOperationsTest, PopFrontFromEmpty) {
  CircularBuffer<int, 8> buffer;
  EXPECT_EQ(buffer.popFront(), int{});
  EXPECT_THAT(buffer, BufferHasExpectedState(0, false, false, true));
}

TEST(StateOperationsTest, PopFrontWithData) {
  CircularBuffer<int, 8> buffer;
  buffer.append(42);
  EXPECT_EQ(buffer.popFront(), 42);
  EXPECT_THAT(buffer, BufferHasExpectedState(0, false, false, true));
}

TEST(StateOperationsTest, ClearEmpty) {
  CircularBuffer<int, 8> buffer;
  buffer.clear();
  EXPECT_THAT(buffer, BufferHasExpectedState(0, false, false, true));
}

TEST(StateOperationsTest, ClearPartiallyFilled) {
  CircularBuffer<int, 8> buffer;
  buffer.append(1);
  buffer.append(2);
  buffer.append(3);
  buffer.clear();
  EXPECT_THAT(buffer, BufferHasExpectedState(0, false, false, true));
}

TEST(StateOperationsTest, ClearWhenFull) {
  CircularBuffer<int, 8> buffer;
  for (int i = 0; i < 8; ++i) {
    buffer.append(i);
  }
  buffer.clear();
  EXPECT_THAT(buffer, BufferHasExpectedState(0, false, false, true));
}

// ============================================================================
// State Query Tests
// ============================================================================

TEST(StateQueryTest, SizeTracksCorrectly) {
  CircularBuffer<int, 8> buffer;
  EXPECT_EQ(buffer.size(), 0);

  buffer.append(1);
  EXPECT_EQ(buffer.size(), 1);

  buffer.append(2);
  EXPECT_EQ(buffer.size(), 2);

  buffer.popFront();
  EXPECT_EQ(buffer.size(), 1);

  buffer.popFront();
  EXPECT_EQ(buffer.size(), 0);
}

// ============================================================================
// Edge Cases and Boundary Conditions
// ============================================================================

TEST(EdgeCasesTest, OverfillBySmallAmount) {
  CircularBuffer<int, 8> buffer;

  // Append 12 elements (4 more than capacity)
  for (int i = 0; i < 12; ++i) {
    buffer.append(i);
  }

  EXPECT_THAT(buffer, BufferHasExpectedState(8, true, true, false));

  // Should contain last 8 elements: 4-11
  for (int i = 4; i < 12; ++i) {
    EXPECT_EQ(buffer.popFront(), i);
  }
}

TEST(EdgeCasesTest, OverfillByLargeAmount) {
  CircularBuffer<int, 8> buffer;

  // Append 100 elements (92 more than capacity)
  for (int i = 0; i < 100; ++i) {
    buffer.append(i);
  }

  EXPECT_THAT(buffer, BufferHasExpectedState(8, true, true, false));

  // Should contain last 8 elements: 92-99
  for (int i = 92; i < 100; ++i) {
    EXPECT_EQ(buffer.popFront(), i);
  }
}

TEST(EdgeCasesTest, WraparoundAfterPopAndAppend) {
  CircularBuffer<int, 8> buffer;

  // Fill buffer with 0-7
  for (int i = 0; i < 8; ++i) {
    buffer.append(i);
  }

  // Pop 4 elements (0,1,2,3), leaving 4,5,6,7
  for (int i = 0; i < 4; ++i) {
    EXPECT_EQ(buffer.popFront(), i);
  }

  // Append 6 more elements (100-105)
  // Buffer has 4 elements, adding 6 gives 10 total, overflows by 2
  for (int i = 100; i < 106; ++i) {
    buffer.append(i);
  }

  EXPECT_THAT(buffer, BufferHasExpectedState(8, true, true, false));

  // Should contain: 6,7,100,101,102,103,104,105 (last 8 of the 10)
  std::vector<int> expected = {6, 7, 100, 101, 102, 103, 104, 105};
  for (int val : expected) {
    EXPECT_EQ(buffer.popFront(), val);
  }
}

TEST(EdgeCasesTest, CompleteWraparoundCycle) {
  CircularBuffer<int, 8> buffer;

  // Fill, empty, refill to test wraparound
  for (int i = 0; i < 8; ++i) {
    buffer.append(i);
  }

  for (int i = 0; i < 8; ++i) {
    buffer.popFront();
  }

  for (int i = 100; i < 108; ++i) {
    buffer.append(i);
  }

  for (int i = 100; i < 108; ++i) {
    EXPECT_EQ(buffer.popFront(), i);
  }

  EXPECT_THAT(buffer, BufferHasExpectedState(0, false, false, true));
}

// ============================================================================
// Different Buffer Sizes Tests
// ============================================================================

TEST(DifferentSizesTest, Size16Buffer) {
  CircularBuffer<int, 16> buffer;

  for (int i = 0; i < 16; ++i) {
    buffer.append(i);
  }

  EXPECT_THAT(buffer, BufferHasExpectedState(16, true, true, false));

  for (int i = 0; i < 16; ++i) {
    EXPECT_EQ(buffer.popFront(), i);
  }

  EXPECT_THAT(buffer, BufferHasExpectedState(0, false, false, true));
}

TEST(DifferentSizesTest, Size32Buffer) {
  CircularBuffer<int, 32> buffer;

  for (int i = 0; i < 32; ++i) {
    buffer.append(i);
  }

  EXPECT_THAT(buffer, BufferHasExpectedState(32, true, true, false));

  for (int i = 0; i < 32; ++i) {
    EXPECT_EQ(buffer.popFront(), i);
  }

  EXPECT_THAT(buffer, BufferHasExpectedState(0, false, false, true));
}

// ============================================================================
// Different Data Types Tests
// ============================================================================

TEST(DifferentTypesTest, CharBuffer) {
  CircularBuffer<char, 8> buffer;

  buffer.append('H');
  buffer.append('e');
  buffer.append('l');
  buffer.append('l');
  buffer.append('o');

  EXPECT_THAT(buffer, BufferHasExpectedState(5, true, false, false));

  EXPECT_EQ(buffer.popFront(), 'H');
  EXPECT_EQ(buffer.popFront(), 'e');
  EXPECT_EQ(buffer.popFront(), 'l');
  EXPECT_EQ(buffer.popFront(), 'l');
  EXPECT_EQ(buffer.popFront(), 'o');

  EXPECT_THAT(buffer, BufferHasExpectedState(0, false, false, true));
}

TEST(DifferentTypesTest, UInt8Buffer) {
  CircularBuffer<uint8_t, 8> buffer;

  for (uint8_t i = 0; i < 8; ++i) {
    buffer.append(i);
  }

  EXPECT_THAT(buffer, BufferHasExpectedState(8, true, true, false));

  for (uint8_t i = 0; i < 8; ++i) {
    EXPECT_EQ(buffer.popFront(), i);
  }

  EXPECT_THAT(buffer, BufferHasExpectedState(0, false, false, true));
}

struct CustomStruct {
  int id;
  char name;

  bool operator==(const CustomStruct &other) const {
    return id == other.id && name == other.name;
  }
};

TEST(DifferentTypesTest, CustomStructBuffer) {
  CircularBuffer<CustomStruct, 4> buffer;

  buffer.append({1, 'A'});
  buffer.append({2, 'B'});
  buffer.append({3, 'C'});

  EXPECT_THAT(buffer, BufferHasExpectedState(3, true, false, false));

  EXPECT_EQ(buffer.popFront(), (CustomStruct{1, 'A'}));
  EXPECT_EQ(buffer.popFront(), (CustomStruct{2, 'B'}));
  EXPECT_EQ(buffer.popFront(), (CustomStruct{3, 'C'}));

  EXPECT_THAT(buffer, BufferHasExpectedState(0, false, false, true));
}

} // namespace
} // namespace jrb::wifi_serial
