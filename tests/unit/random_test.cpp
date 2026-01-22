#include "fvm/random.h"
#include <gtest/gtest.h>
#include <vector>

using namespace fvm::core;

class RandomTest : public ::testing::Test {
protected:
    Random rng;
};

TEST_F(RandomTest, NextIntReturnsValidValue) {
    int value = rng.next_int();
    EXPECT_GE(value, 0);
}

TEST_F(RandomTest, NextIntInRangeReturnsWithinBounds) {
    int min = 10;
    int max = 20;
    int value = rng.next_int_range(min, max);
    EXPECT_GE(value, min);
    EXPECT_LE(value, max);
}

TEST_F(RandomTest, NextIntRangeSingleValue) {
    int value = rng.next_int_range(5, 5);
    EXPECT_EQ(value, 5);
}

TEST_F(RandomTest, MultipleCallsProduceValues) {
    std::vector<int> values;
    for (int i = 0; i < 100; i++) {
        values.push_back(rng.next_int_range(1, 100));
    }
    EXPECT_EQ(values.size(), 100);
}
