#ifndef DATA_SERIALIZER_TEST_CPP
#define DATA_SERIALIZER_TEST_CPP

#include "fvm/data_serializer.h"
#include <gtest/gtest.h>

class DataSerializerTest : public ::testing::Test {
protected:
    fvm::DataSerializer serializer;
};

TEST_F(DataSerializerTest, SerializeEmptyContent) {
    fvm::interfaces::vvs empty;
    std::vector<int> sequence;
    ASSERT_TRUE(serializer.serialize(empty, sequence));
    // Empty content (0 blocks) still serializes to "0"
    EXPECT_FALSE(sequence.empty());
    EXPECT_EQ(sequence.size(), 1);  // Just the character '0'
}

TEST_F(DataSerializerTest, SerializeSimpleContent) {
    fvm::interfaces::vvs content = {{"hello", "world"}};
    std::vector<int> sequence;
    ASSERT_TRUE(serializer.serialize(content, sequence));
    EXPECT_FALSE(sequence.empty());
}

TEST_F(DataSerializerTest, SerializeRoundTrip) {
    fvm::interfaces::vvs original = {{"hello", "world"}, {"test", "data"}};
    std::vector<int> sequence;
    fvm::interfaces::vvs restored;

    ASSERT_TRUE(serializer.serialize(original, sequence));
    ASSERT_TRUE(serializer.deserialize(sequence, restored));

    EXPECT_EQ(original.size(), restored.size());
    for (size_t i = 0; i < original.size(); i++) {
        EXPECT_EQ(original[i], restored[i]);
    }
}

TEST_F(DataSerializerTest, SerializeRoundTripComplex) {
    fvm::interfaces::vvs original = {
        {"key1", "value1", "extra1"},
        {"key2", "value2"},
        {"", "empty_value"},
        {"single"}
    };
    std::vector<int> sequence;
    fvm::interfaces::vvs restored;

    ASSERT_TRUE(serializer.serialize(original, sequence));
    ASSERT_TRUE(serializer.deserialize(sequence, restored));

    EXPECT_EQ(original.size(), restored.size());
    for (size_t i = 0; i < original.size(); i++) {
        EXPECT_EQ(original[i], restored[i]);
    }
}

TEST_F(DataSerializerTest, HashCalculationConsistent) {
    std::vector<int> data = {1, 2, 3, 4, 5};
    auto hash1 = serializer.calculate_hash(data);
    auto hash2 = serializer.calculate_hash(data);
    EXPECT_EQ(hash1, hash2);
}

TEST_F(DataSerializerTest, HashDifferentForDifferentData) {
    std::vector<int> data1 = {1, 2, 3};
    std::vector<int> data2 = {1, 2, 4};
    EXPECT_NE(serializer.calculate_hash(data1), serializer.calculate_hash(data2));
}

TEST_F(DataSerializerTest, HashStringConsistent) {
    std::string data = "test_string";
    auto hash1 = serializer.calculate_hash(data);
    auto hash2 = serializer.calculate_hash(data);
    EXPECT_EQ(hash1, hash2);
}

TEST_F(DataSerializerTest, DeserializeInvalidDataReturnsFalse) {
    std::vector<int> invalid_sequence = {1, 2, 3};  // Too short to be valid
    fvm::interfaces::vvs content;
    EXPECT_FALSE(serializer.deserialize(invalid_sequence, content));
}

TEST_F(DataSerializerTest, SerializeEmptyStrings) {
    fvm::interfaces::vvs original = {{"", "", ""}, {"", ""}};
    std::vector<int> sequence;
    fvm::interfaces::vvs restored;

    ASSERT_TRUE(serializer.serialize(original, sequence));
    ASSERT_TRUE(serializer.deserialize(sequence, restored));

    EXPECT_EQ(original.size(), restored.size());
    for (size_t i = 0; i < original.size(); i++) {
        EXPECT_EQ(original[i], restored[i]);
    }
}

TEST_F(DataSerializerTest, SerializeWithSpecialCharacters) {
    fvm::interfaces::vvs original = {{"hello\nworld", "\ttest"}, {"a\nb\nc"}};
    std::vector<int> sequence;
    fvm::interfaces::vvs restored;

    ASSERT_TRUE(serializer.serialize(original, sequence));
    ASSERT_TRUE(serializer.deserialize(sequence, restored));

    EXPECT_EQ(original.size(), restored.size());
    for (size_t i = 0; i < original.size(); i++) {
        EXPECT_EQ(original[i], restored[i]);
    }
}

#endif // DATA_SERIALIZER_TEST_CPP
