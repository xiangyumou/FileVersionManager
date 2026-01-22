#ifndef SAVER_TEST_CPP
#define SAVER_TEST_CPP

#include "fvm/interfaces/ISaver.h"
#include "fvm/interfaces/IEncryptor.h"
#include "../mocks/mock_logger.h"
#include "../mocks/mock_file_operations.h"
#include "../mocks/mock_encryptor.h"
#include <gtest/gtest.h>

// Factory function to create Saver instances
extern "C" {
    struct Saver;
    Saver* create_test_saver(fvm::interfaces::ILogger&, fvm::interfaces::IEncryptor*, fvm::interfaces::IFileOperations*);
    void destroy_test_saver(Saver* s);
}

// Helper to create ISaver from Saver*
inline fvm::interfaces::ISaver* make_saver(fvm::interfaces::ILogger& logger,
                                           fvm::interfaces::IEncryptor* encryptor,
                                           fvm::interfaces::IFileOperations* file_ops) {
    return create_test_saver(logger, encryptor, file_ops);
}

class SaverTest : public ::testing::Test {
protected:
    MockFileOperations mock_file_ops;
    MockEncryptor mock_encryptor;
    fvm::mocks::MockLogger mock_logger;
    fvm::interfaces::ISaver* saver;

    void SetUp() override {
        saver = make_saver(mock_logger, &mock_encryptor, &mock_file_ops);
        saver->initialize();
    }

    void TearDown() override {
        if (saver) {
            saver->shutdown();
            destroy_test_saver(static_cast<Saver*>(saver));
            saver = nullptr;
        }
    }
};

// ========== Lifecycle Tests ==========

TEST_F(SaverTest, InitializeReturnsTrue) {
    Saver* saver2 = make_saver(mock_logger, &mock_encryptor, &mock_file_ops);
    EXPECT_TRUE(saver2->initialize());
    destroy_test_saver(saver2);
}

TEST_F(SaverTest, ShutdownReturnsTrue) {
    EXPECT_TRUE(saver->shutdown());
}

// ========== Save/Load Tests ==========

TEST_F(SaverTest, SaveAndLoadRoundTrip) {
    fvm::interfaces::vvs original = {{"hello", "world"}, {"test", "data"}};

    ASSERT_TRUE(saver->save("test_key", original));

    fvm::interfaces::vvs loaded;
    ASSERT_TRUE(saver->load("test_key", loaded));

    EXPECT_EQ(original.size(), loaded.size());
    for (size_t i = 0; i < original.size(); i++) {
        EXPECT_EQ(original[i], loaded[i]);
    }
}

TEST_F(SaverTest, SaveOverwritesExistingData) {
    fvm::interfaces::vvs v1 = {{"version1"}};
    fvm::interfaces::vvs v2 = {{"version2"}};

    ASSERT_TRUE(saver->save("key", v1));
    ASSERT_TRUE(saver->save("key", v2));

    fvm::interfaces::vvs loaded;
    ASSERT_TRUE(saver->load("key", loaded));

    EXPECT_EQ(loaded, v2);
}

TEST_F(SaverTest, LoadNonExistentReturnsFalse) {
    fvm::interfaces::vvs loaded;
    EXPECT_FALSE(saver->load("nonexistent_key", loaded));
}

TEST_F(SaverTest, SaveMultipleKeys) {
    fvm::interfaces::vvs data1 = {{"data1"}};
    fvm::interfaces::vvs data2 = {{"data2"}};

    ASSERT_TRUE(saver->save("key1", data1));
    ASSERT_TRUE(saver->save("key2", data2));

    fvm::interfaces::vvs loaded;
    ASSERT_TRUE(saver->load("key1", loaded));
    EXPECT_EQ(loaded, data1);
}

// ========== WAL Tests ==========

TEST_F(SaverTest, WALSizeWorks) {
    size_t initial_size = saver->get_wal_size();

    fvm::interfaces::vvs data = {{"test"}};
    saver->save("key1", data);

    EXPECT_GT(saver->get_wal_size(), initial_size);
}

TEST_F(SaverTest, WALCanBeDisabled) {
    ASSERT_TRUE(saver->set_wal_enabled(false));
    EXPECT_FALSE(saver->get_wal_enabled());

    ASSERT_TRUE(saver->set_wal_enabled(true));
    EXPECT_TRUE(saver->get_wal_enabled());
}

TEST_F(SaverTest, FlushReturnsTrue) {
    EXPECT_TRUE(saver->flush());
}

TEST_F(SaverTest, CompactReturnsTrue) {
    EXPECT_TRUE(saver->compact());
}

// ========== Configuration Tests ==========

TEST_F(SaverTest, GetDataFileReturnsDefault) {
    EXPECT_EQ(saver->get_data_file(), "data.chm");
}

TEST_F(SaverTest, GetWalFileReturnsDefault) {
    EXPECT_EQ(saver->get_wal_file(), "data.wal");
}

TEST_F(SaverTest, SetAutoCompactThreshold) {
    ASSERT_TRUE(saver->set_auto_compact(50));
    EXPECT_EQ(saver->get_auto_compact_threshold(), 50);
}

TEST_F(SaverTest, SetAutoCompactThresholdRejectsZero) {
    EXPECT_FALSE(saver->set_auto_compact(0));
}

// ========== String Utilities Tests ==========

TEST_F(SaverTest, IsAllDigitsWithValidString) {
    std::string digits = "12345";
    EXPECT_TRUE(saver->is_all_digits(digits));
}

TEST_F(SaverTest, IsAllDigitsWithMixedString) {
    std::string mixed = "123abc";
    EXPECT_FALSE(saver->is_all_digits(mixed));
}

TEST_F(SaverTest, StrToUllWithValidNumber) {
    std::string number = "12345";
    EXPECT_EQ(saver->str_to_ull(number), 12345ULL);
}

TEST_F(SaverTest, StrToUllWithNonDigitsReturnsZero) {
    std::string invalid = "abc";
    EXPECT_EQ(saver->str_to_ull(invalid), 0ULL);
}

#endif // SAVER_TEST_CPP
