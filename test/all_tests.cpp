#include "gtest/gtest.h"
#include <string>
#include "domain/serial/circular_buffer_test.cpp"

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}