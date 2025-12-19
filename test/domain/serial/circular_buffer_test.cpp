#include <gtest/gtest.h>
#include "domain/serial/circular_buffer.hpp"
#include <nonstd/span.hpp>

namespace jrb::wifi_serial {
namespace {

// Test fixture for basic CircularBuffer tests
template <typename T, std::size_t SIZE>
class CircularBufferFixture : public ::testing::Test {
protected:
    CircularBuffer<T, SIZE> buffer;

    void SetUp() override {
        // Buffer is initialized empty
    }

    void TearDown() override {
        // Clean up if needed
    }

    // Helper to fill buffer completely
    void fillBuffer() {
        for (std::size_t i = 0; i < SIZE; ++i) {
            buffer.append(static_cast<T>(i));
        }
    }

    // Helper to verify buffer state
    void verifyBufferState(std::size_t expectedSize, bool expectedHasData,
                          bool expectedFull, bool expectedEmpty) {
        EXPECT_EQ(buffer.size(), expectedSize);
        EXPECT_EQ(buffer.hasData(), expectedHasData);
        EXPECT_EQ(buffer.full(), expectedFull);
        EXPECT_EQ(buffer.empty(), expectedEmpty);
    }
};

// Type alias for common test cases
using IntBuffer8 = CircularBufferFixture<int, 8>;
using IntBuffer16 = CircularBufferFixture<int, 16>;
using CharBuffer8 = CircularBufferFixture<char, 8>;
using UInt8Buffer32 = CircularBufferFixture<uint8_t, 32>;

// ============================================================================
// Constructor and Initial State Tests
// ============================================================================

TEST_F(IntBuffer8, InitialStateIsEmpty) {
    verifyBufferState(0, false, false, true);
}

TEST_F(CharBuffer8, InitialStateIsEmpty) {
    verifyBufferState(0, false, false, true);
}

TEST_F(UInt8Buffer32, InitialStateIsEmpty) {
    verifyBufferState(0, false, false, true);
}

// ============================================================================
// Single Element Append Tests
// ============================================================================

TEST_F(IntBuffer8, AppendSingleElement) {
    buffer.append(42);

    verifyBufferState(1, true, false, false);
    EXPECT_EQ(buffer.popFront(), 42);
}

TEST_F(CharBuffer8, AppendSingleCharacter) {
    buffer.append('A');

    verifyBufferState(1, true, false, false);
    EXPECT_EQ(buffer.popFront(), 'A');
}

TEST_F(IntBuffer8, AppendMultipleElementsNotFull) {
    buffer.append(1);
    buffer.append(2);
    buffer.append(3);

    verifyBufferState(3, true, false, false);

    EXPECT_EQ(buffer.popFront(), 1);
    EXPECT_EQ(buffer.popFront(), 2);
    EXPECT_EQ(buffer.popFront(), 3);
}

TEST_F(IntBuffer8, AppendUntilFull) {
    for (int i = 0; i < 8; ++i) {
        buffer.append(i);
        EXPECT_EQ(buffer.size(), static_cast<std::size_t>(i + 1));
    }

    verifyBufferState(8, true, true, false);
}

TEST_F(IntBuffer8, AppendWhenFull_OverwritesOldestData) {
    // Fill buffer with 0-7
    fillBuffer();

    // Append one more element (should overwrite oldest)
    buffer.append(100);

    // Buffer should still be full
    verifyBufferState(8, true, true, false);

    // First element should be 1 (0 was overwritten)
    EXPECT_EQ(buffer.popFront(), 1);
}

TEST_F(IntBuffer8, AppendWhenFull_OverwritesMultipleOldest) {
    // Fill buffer with 0-7
    fillBuffer();

    // Append 3 more elements
    buffer.append(100);
    buffer.append(101);
    buffer.append(102);

    // Buffer should still be full
    verifyBufferState(8, true, true, false);

    // First elements should be 3, 4, 5, 6, 7, 100, 101, 102
    EXPECT_EQ(buffer.popFront(), 3);
    EXPECT_EQ(buffer.popFront(), 4);
    EXPECT_EQ(buffer.popFront(), 5);
    EXPECT_EQ(buffer.popFront(), 6);
    EXPECT_EQ(buffer.popFront(), 7);
    EXPECT_EQ(buffer.popFront(), 100);
    EXPECT_EQ(buffer.popFront(), 101);
    EXPECT_EQ(buffer.popFront(), 102);
}

// ============================================================================
// Span Append Tests
// ============================================================================

TEST_F(IntBuffer8, AppendSpan_EmptySpan) {
    std::array<int, 0> data{};
    nonstd::span<const int> span(data.data(), 0);

    buffer.append(span);

    verifyBufferState(0, false, false, true);
}

TEST_F(IntBuffer8, AppendSpan_SingleElement) {
    std::array<int, 1> data{42};
    nonstd::span<const int> span(data.data(), data.size());

    buffer.append(span);

    verifyBufferState(1, true, false, false);
    EXPECT_EQ(buffer.popFront(), 42);
}

TEST_F(IntBuffer8, AppendSpan_MultipleElements) {
    std::array<int, 4> data{10, 20, 30, 40};
    nonstd::span<const int> span(data.data(), data.size());

    buffer.append(span);

    verifyBufferState(4, true, false, false);

    EXPECT_EQ(buffer.popFront(), 10);
    EXPECT_EQ(buffer.popFront(), 20);
    EXPECT_EQ(buffer.popFront(), 30);
    EXPECT_EQ(buffer.popFront(), 40);
}

TEST_F(IntBuffer8, AppendSpan_ExactlyFillsBuffer) {
    std::array<int, 8> data{0, 1, 2, 3, 4, 5, 6, 7};
    nonstd::span<const int> span(data.data(), data.size());

    buffer.append(span);

    verifyBufferState(8, true, true, false);

    for (int i = 0; i < 8; ++i) {
        EXPECT_EQ(buffer.popFront(), i);
    }
}

TEST_F(IntBuffer8, AppendSpan_LargerThanBuffer_OverwritesOldest) {
    std::array<int, 12> data{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    nonstd::span<const int> span(data.data(), data.size());

    buffer.append(span);

    verifyBufferState(8, true, true, false);

    // Should contain last 8 elements: 4, 5, 6, 7, 8, 9, 10, 11
    for (int i = 4; i < 12; ++i) {
        EXPECT_EQ(buffer.popFront(), i);
    }
}

TEST_F(CharBuffer8, AppendSpan_Characters) {
    const char* str = "ABCD";
    nonstd::span<const char> span(str, 4);

    buffer.append(span);

    verifyBufferState(4, true, false, false);

    EXPECT_EQ(buffer.popFront(), 'A');
    EXPECT_EQ(buffer.popFront(), 'B');
    EXPECT_EQ(buffer.popFront(), 'C');
    EXPECT_EQ(buffer.popFront(), 'D');
}

// ============================================================================
// PopFront Tests
// ============================================================================

TEST_F(IntBuffer8, PopFront_FromEmptyBuffer_ReturnsDefault) {
    int value = buffer.popFront();

    EXPECT_EQ(value, int{});
    verifyBufferState(0, false, false, true);
}

TEST_F(CharBuffer8, PopFront_FromEmptyBuffer_ReturnsDefault) {
    char value = buffer.popFront();

    EXPECT_EQ(value, char{});
    verifyBufferState(0, false, false, true);
}

TEST_F(IntBuffer8, PopFront_SingleElement) {
    buffer.append(99);

    int value = buffer.popFront();

    EXPECT_EQ(value, 99);
    verifyBufferState(0, false, false, true);
}

TEST_F(IntBuffer8, PopFront_MultipleElements) {
    buffer.append(10);
    buffer.append(20);
    buffer.append(30);

    EXPECT_EQ(buffer.popFront(), 10);
    verifyBufferState(2, true, false, false);

    EXPECT_EQ(buffer.popFront(), 20);
    verifyBufferState(1, true, false, false);

    EXPECT_EQ(buffer.popFront(), 30);
    verifyBufferState(0, false, false, true);
}

TEST_F(IntBuffer8, PopFront_AllElements_HasDataBecomesFalse) {
    fillBuffer();

    for (int i = 0; i < 8; ++i) {
        int value = buffer.popFront();
        EXPECT_EQ(value, i);

        if (i < 7) {
            EXPECT_TRUE(buffer.hasData());
        } else {
            EXPECT_FALSE(buffer.hasData());
        }
    }

    verifyBufferState(0, false, false, true);
}

TEST_F(IntBuffer8, PopFront_AfterWraparound) {
    // Fill buffer
    fillBuffer();

    // Pop some elements
    buffer.popFront();
    buffer.popFront();
    buffer.popFront();

    // Add new elements (causing head to wrap)
    buffer.append(100);
    buffer.append(101);
    buffer.append(102);

    // Verify FIFO order
    EXPECT_EQ(buffer.popFront(), 3);
    EXPECT_EQ(buffer.popFront(), 4);
    EXPECT_EQ(buffer.popFront(), 5);
    EXPECT_EQ(buffer.popFront(), 6);
    EXPECT_EQ(buffer.popFront(), 7);
    EXPECT_EQ(buffer.popFront(), 100);
    EXPECT_EQ(buffer.popFront(), 101);
    EXPECT_EQ(buffer.popFront(), 102);
}

// ============================================================================
// Clear Tests
// ============================================================================

TEST_F(IntBuffer8, Clear_EmptyBuffer) {
    buffer.clear();

    verifyBufferState(0, false, false, true);
}

TEST_F(IntBuffer8, Clear_PartiallyFilledBuffer) {
    buffer.append(1);
    buffer.append(2);
    buffer.append(3);

    buffer.clear();

    verifyBufferState(0, false, false, true);
}

TEST_F(IntBuffer8, Clear_FullBuffer) {
    fillBuffer();

    buffer.clear();

    verifyBufferState(0, false, false, true);
}

TEST_F(IntBuffer8, Clear_ThenReuse) {
    buffer.append(1);
    buffer.append(2);
    buffer.clear();

    buffer.append(10);
    buffer.append(20);

    verifyBufferState(2, true, false, false);

    EXPECT_EQ(buffer.popFront(), 10);
    EXPECT_EQ(buffer.popFront(), 20);
}

// ============================================================================
// State Query Tests
// ============================================================================

TEST_F(IntBuffer8, HasData_InitiallyFalse) {
    EXPECT_FALSE(buffer.hasData());
}

TEST_F(IntBuffer8, HasData_TrueAfterAppend) {
    buffer.append(1);
    EXPECT_TRUE(buffer.hasData());
}

TEST_F(IntBuffer8, HasData_FalseAfterPoppingAll) {
    buffer.append(1);
    buffer.popFront();
    EXPECT_FALSE(buffer.hasData());
}

TEST_F(IntBuffer8, HasData_FalseAfterClear) {
    buffer.append(1);
    buffer.clear();
    EXPECT_FALSE(buffer.hasData());
}

TEST_F(IntBuffer8, Full_InitiallyFalse) {
    EXPECT_FALSE(buffer.full());
}

TEST_F(IntBuffer8, Full_TrueWhenFilled) {
    fillBuffer();
    EXPECT_TRUE(buffer.full());
}

TEST_F(IntBuffer8, Full_FalseAfterPop) {
    fillBuffer();
    buffer.popFront();
    EXPECT_FALSE(buffer.full());
}

TEST_F(IntBuffer8, Empty_InitiallyTrue) {
    EXPECT_TRUE(buffer.empty());
}

TEST_F(IntBuffer8, Empty_FalseAfterAppend) {
    buffer.append(1);
    EXPECT_FALSE(buffer.empty());
}

TEST_F(IntBuffer8, Empty_TrueAfterPoppingAll) {
    buffer.append(1);
    buffer.popFront();
    EXPECT_TRUE(buffer.empty());
}

TEST_F(IntBuffer8, Size_TracksCorrectly) {
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

TEST_F(IntBuffer8, AlternatingAppendAndPop) {
    for (int i = 0; i < 20; ++i) {
        buffer.append(i);
        EXPECT_EQ(buffer.popFront(), i);
        verifyBufferState(0, false, false, true);
    }
}

TEST_F(IntBuffer8, FillEmptyFillEmpty) {
    // First fill
    fillBuffer();
    verifyBufferState(8, true, true, false);

    // Empty
    for (int i = 0; i < 8; ++i) {
        buffer.popFront();
    }
    verifyBufferState(0, false, false, true);

    // Second fill
    fillBuffer();
    verifyBufferState(8, true, true, false);

    // Empty again
    for (int i = 0; i < 8; ++i) {
        buffer.popFront();
    }
    verifyBufferState(0, false, false, true);
}

TEST_F(IntBuffer8, PartialFillPopPartialFill) {
    // Add 5 elements
    for (int i = 0; i < 5; ++i) {
        buffer.append(i);
    }

    // Remove 3 elements
    for (int i = 0; i < 3; ++i) {
        buffer.popFront();
    }

    EXPECT_EQ(buffer.size(), 2);

    // Add 6 more elements (total should be 8 = full)
    for (int i = 100; i < 106; ++i) {
        buffer.append(i);
    }

    verifyBufferState(8, true, true, false);

    // Verify order: 3, 4, 100, 101, 102, 103, 104, 105
    EXPECT_EQ(buffer.popFront(), 3);
    EXPECT_EQ(buffer.popFront(), 4);
    EXPECT_EQ(buffer.popFront(), 100);
    EXPECT_EQ(buffer.popFront(), 101);
    EXPECT_EQ(buffer.popFront(), 102);
    EXPECT_EQ(buffer.popFront(), 103);
    EXPECT_EQ(buffer.popFront(), 104);
    EXPECT_EQ(buffer.popFront(), 105);
}

TEST_F(IntBuffer8, MultiplePopFromEmpty) {
    EXPECT_EQ(buffer.popFront(), int{});
    EXPECT_EQ(buffer.popFront(), int{});
    EXPECT_EQ(buffer.popFront(), int{});

    verifyBufferState(0, false, false, true);
}

TEST_F(IntBuffer8, OverfillByMany) {
    // Add way more than capacity
    for (int i = 0; i < 100; ++i) {
        buffer.append(i);
    }

    // Should still be full with last 8 elements
    verifyBufferState(8, true, true, false);

    // Should contain 92-99
    for (int i = 92; i < 100; ++i) {
        EXPECT_EQ(buffer.popFront(), i);
    }
}

// ============================================================================
// Wraparound Behavior Tests
// ============================================================================

TEST_F(IntBuffer8, HeadWraparound) {
    // Fill buffer completely
    fillBuffer();

    // Pop half
    for (int i = 0; i < 4; ++i) {
        buffer.popFront();
    }

    // Add more than remaining space (forces head wrap)
    for (int i = 100; i < 108; ++i) {
        buffer.append(i);
    }

    // Should be full
    verifyBufferState(8, true, true, false);

    // Should contain last 8: 100, 101, 102, 103, 104, 105, 106, 107
    for (int i = 100; i < 108; ++i) {
        EXPECT_EQ(buffer.popFront(), i);
    }
}

TEST_F(IntBuffer8, TailWraparound) {
    // Fill to capacity
    fillBuffer();

    // Pop all
    for (int i = 0; i < 8; ++i) {
        buffer.popFront();
    }

    // Fill again (head has wrapped, now tail will catch up)
    for (int i = 100; i < 108; ++i) {
        buffer.append(i);
    }

    // Pop all (tail should wrap)
    for (int i = 100; i < 108; ++i) {
        EXPECT_EQ(buffer.popFront(), i);
    }

    verifyBufferState(0, false, false, true);
}

// ============================================================================
// Parametric Tests for Different Buffer Sizes
// ============================================================================

class CircularBufferSizeTest : public ::testing::TestWithParam<std::size_t> {
protected:
    // Helper to test a buffer of given size
    template<std::size_t SIZE>
    void testBufferSize() {
        CircularBuffer<int, SIZE> buffer;

        // Fill completely
        for (std::size_t i = 0; i < SIZE; ++i) {
            buffer.append(static_cast<int>(i));
        }

        EXPECT_TRUE(buffer.full());
        EXPECT_EQ(buffer.size(), SIZE);

        // Pop all
        for (std::size_t i = 0; i < SIZE; ++i) {
            EXPECT_EQ(buffer.popFront(), static_cast<int>(i));
        }

        EXPECT_TRUE(buffer.empty());
        EXPECT_EQ(buffer.size(), 0);
    }
};

TEST_F(IntBuffer16, FillAndEmptySize16) {
    for (int i = 0; i < 16; ++i) {
        buffer.append(i);
    }

    verifyBufferState(16, true, true, false);

    for (int i = 0; i < 16; ++i) {
        EXPECT_EQ(buffer.popFront(), i);
    }

    verifyBufferState(0, false, false, true);
}

TEST_F(UInt8Buffer32, FillAndEmptySize32) {
    for (uint8_t i = 0; i < 32; ++i) {
        buffer.append(i);
    }

    verifyBufferState(32, true, true, false);

    for (uint8_t i = 0; i < 32; ++i) {
        EXPECT_EQ(buffer.popFront(), i);
    }

    verifyBufferState(0, false, false, true);
}

// ============================================================================
// Different Data Types Tests
// ============================================================================

struct CustomStruct {
    int id;
    char name;

    bool operator==(const CustomStruct& other) const {
        return id == other.id && name == other.name;
    }
};

using CustomStructBuffer4 = CircularBufferFixture<CustomStruct, 4>;

TEST_F(CustomStructBuffer4, AppendAndPopCustomStruct) {
    CustomStruct s1{1, 'A'};
    CustomStruct s2{2, 'B'};
    CustomStruct s3{3, 'C'};

    buffer.append(s1);
    buffer.append(s2);
    buffer.append(s3);

    verifyBufferState(3, true, false, false);

    EXPECT_EQ(buffer.popFront(), s1);
    EXPECT_EQ(buffer.popFront(), s2);
    EXPECT_EQ(buffer.popFront(), s3);
}

TEST_F(CustomStructBuffer4, CustomStruct_Wraparound) {
    // Fill buffer
    for (int i = 0; i < 4; ++i) {
        buffer.append(CustomStruct{i, static_cast<char>('A' + i)});
    }

    // Overwrite first 2
    buffer.append(CustomStruct{100, 'X'});
    buffer.append(CustomStruct{101, 'Y'});

    verifyBufferState(4, true, true, false);

    EXPECT_EQ(buffer.popFront(), (CustomStruct{2, 'C'}));
    EXPECT_EQ(buffer.popFront(), (CustomStruct{3, 'D'}));
    EXPECT_EQ(buffer.popFront(), (CustomStruct{100, 'X'}));
    EXPECT_EQ(buffer.popFront(), (CustomStruct{101, 'Y'}));
}

// ============================================================================
// Combined Operations Tests
// ============================================================================

TEST_F(IntBuffer8, MixedOperations) {
    // Append some
    buffer.append(1);
    buffer.append(2);

    // Pop one
    EXPECT_EQ(buffer.popFront(), 1);

    // Append more
    buffer.append(3);
    buffer.append(4);

    // Clear
    buffer.clear();

    // Start fresh
    buffer.append(10);
    EXPECT_EQ(buffer.size(), 1);
    EXPECT_EQ(buffer.popFront(), 10);
}

TEST_F(IntBuffer8, AppendSpanThenSingleAppends) {
    std::array<int, 3> data{1, 2, 3};
    nonstd::span<const int> span(data.data(), data.size());

    buffer.append(span);
    buffer.append(4);
    buffer.append(5);

    verifyBufferState(5, true, false, false);

    for (int i = 1; i <= 5; ++i) {
        EXPECT_EQ(buffer.popFront(), i);
    }
}

TEST_F(IntBuffer8, MultipleSpanAppends) {
    std::array<int, 2> data1{1, 2};
    std::array<int, 3> data2{3, 4, 5};
    std::array<int, 2> data3{6, 7};

    nonstd::span<const int> span1(data1.data(), data1.size());
    nonstd::span<const int> span2(data2.data(), data2.size());
    nonstd::span<const int> span3(data3.data(), data3.size());

    buffer.append(span1);
    buffer.append(span2);
    buffer.append(span3);

    verifyBufferState(7, true, false, false);

    for (int i = 1; i <= 7; ++i) {
        EXPECT_EQ(buffer.popFront(), i);
    }
}

// ============================================================================
// FIFO Order Verification Tests
// ============================================================================

TEST_F(IntBuffer8, VerifyFIFOOrder_NoWraparound) {
    for (int i = 0; i < 5; ++i) {
        buffer.append(i * 10);
    }

    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(buffer.popFront(), i * 10);
    }
}

TEST_F(IntBuffer8, VerifyFIFOOrder_WithWraparound) {
    // Fill buffer
    for (int i = 0; i < 8; ++i) {
        buffer.append(i);
    }

    // Remove 5 elements
    for (int i = 0; i < 5; ++i) {
        buffer.popFront();
    }

    // Add 5 more (causes wraparound)
    for (int i = 100; i < 105; ++i) {
        buffer.append(i);
    }

    // Should have: 5, 6, 7, 100, 101, 102, 103, 104
    EXPECT_EQ(buffer.popFront(), 5);
    EXPECT_EQ(buffer.popFront(), 6);
    EXPECT_EQ(buffer.popFront(), 7);
    EXPECT_EQ(buffer.popFront(), 100);
    EXPECT_EQ(buffer.popFront(), 101);
    EXPECT_EQ(buffer.popFront(), 102);
    EXPECT_EQ(buffer.popFront(), 103);
    EXPECT_EQ(buffer.popFront(), 104);
}

TEST_F(IntBuffer8, VerifyFIFOOrder_MultipleWraparounds) {
    // Simulate many wrap-arounds
    for (int cycle = 0; cycle < 5; ++cycle) {
        fillBuffer();
        for (int i = 0; i < 8; ++i) {
            EXPECT_EQ(buffer.popFront(), i);
        }
    }

    verifyBufferState(0, false, false, true);
}

} // namespace
} // namespace jrb::wifi_serial
