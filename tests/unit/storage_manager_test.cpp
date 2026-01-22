#ifndef STORAGE_MANAGER_TEST_CPP
#define STORAGE_MANAGER_TEST_CPP

#include "fvm/storage_manager.h"
#include "../mocks/mock_logger.h"
#include <gtest/gtest.h>
#include <cstdio>
#include <fstream>

class StorageManagerTest : public ::testing::Test {
protected:
    fvm::mocks::MockLogger mock_logger;
    std::unique_ptr<fvm::StorageManager> storage_manager;
    std::string test_data_file = "test_storage_manager.chm";

    void SetUp() override {
        storage_manager = std::make_unique<fvm::StorageManager>(mock_logger);
    }

    void TearDown() override {
        storage_manager.reset();
        // Clean up test file
        std::remove(test_data_file.c_str());
    }
};

TEST_F(StorageManagerTest, StoreAndRetrieve) {
    std::vector<std::pair<double, double>> data = {{1.0, 2.0}, {3.0, 4.0}};

    storage_manager->store(123, 456, data, 1);

    EXPECT_TRUE(storage_manager->exists(123));

    fvm::interfaces::DataNode node;
    ASSERT_TRUE(storage_manager->retrieve(123, node));
    EXPECT_EQ(node.name_hash, 123);
    EXPECT_EQ(node.data_hash, 456);
    EXPECT_EQ(node.len, 1);
}

TEST_F(StorageManagerTest, RetrieveNonExistentReturnsFalse) {
    fvm::interfaces::DataNode node;
    EXPECT_FALSE(storage_manager->retrieve(999, node));
}

TEST_F(StorageManagerTest, ExistsReturnsFalseForNonExistent) {
    EXPECT_FALSE(storage_manager->exists(999));
}

TEST_F(StorageManagerTest, RemoveExistingReturnsTrue) {
    std::vector<std::pair<double, double>> data = {{1.0, 2.0}};
    storage_manager->store(123, 456, data, 1);

    EXPECT_TRUE(storage_manager->remove(123));
    EXPECT_FALSE(storage_manager->exists(123));
}

TEST_F(StorageManagerTest, RemoveNonExistentReturnsFalse) {
    EXPECT_FALSE(storage_manager->remove(999));
}

TEST_F(StorageManagerTest, StoreOverwritesExisting) {
    std::vector<std::pair<double, double>> data1 = {{1.0, 2.0}};
    std::vector<std::pair<double, double>> data2 = {{3.0, 4.0}};

    storage_manager->store(123, 456, data1, 1);
    storage_manager->store(123, 789, data2, 1);

    fvm::interfaces::DataNode node;
    ASSERT_TRUE(storage_manager->retrieve(123, node));
    EXPECT_EQ(node.data_hash, 789);  // Should have new hash
}

TEST_F(StorageManagerTest, ClearRemovesAllData) {
    std::vector<std::pair<double, double>> data = {{1.0, 2.0}};

    storage_manager->store(123, 456, data, 1);
    storage_manager->store(456, 789, data, 1);

    EXPECT_TRUE(storage_manager->exists(123));
    EXPECT_TRUE(storage_manager->exists(456));

    storage_manager->clear();

    EXPECT_FALSE(storage_manager->exists(123));
    EXPECT_FALSE(storage_manager->exists(456));
}

TEST_F(StorageManagerTest, GetAllDataReturnsCopy) {
    std::vector<std::pair<double, double>> data = {{1.0, 2.0}};

    storage_manager->store(123, 456, data, 1);
    storage_manager->store(456, 789, data, 1);

    auto all_data = storage_manager->get_all_data();
    EXPECT_EQ(all_data.size(), 2);
    EXPECT_TRUE(all_data.count(123));
    EXPECT_TRUE(all_data.count(456));
}

TEST_F(StorageManagerTest, SaveAndLoadFromFile) {
    // Store some data
    // Note: len is data.size() / block_size, so for block_size=16, we need appropriate data
    std::vector<std::pair<double, double>> data;
    for (int i = 0; i < 16; i++) {
        data.push_back({static_cast<double>(i), static_cast<double>(i * 2)});
    }
    storage_manager->store(123, 456, data, 1);  // len = 16 / 16 = 1

    // Save to file
    ASSERT_TRUE(storage_manager->save_to_file(test_data_file));

    // Create new storage manager and load
    storage_manager = std::make_unique<fvm::StorageManager>(mock_logger);
    ASSERT_TRUE(storage_manager->load_from_file(test_data_file, 16));

    // Verify data was loaded
    EXPECT_TRUE(storage_manager->exists(123));

    fvm::interfaces::DataNode node;
    ASSERT_TRUE(storage_manager->retrieve(123, node));
    EXPECT_EQ(node.name_hash, 123);
    EXPECT_EQ(node.data_hash, 456);
    EXPECT_EQ(node.data.size(), 16);
}

TEST_F(StorageManagerTest, LoadFromNonExistentFileReturnsFalse) {
    // Try to load from a file that doesn't exist
    EXPECT_FALSE(storage_manager->load_from_file("nonexistent_file.chm", 16));
}

TEST_F(StorageManagerTest, StoreMultipleAndRetrieveAll) {
    std::vector<std::pair<double, double>> data1 = {{1.0, 2.0}};
    std::vector<std::pair<double, double>> data2 = {{3.0, 4.0}};
    std::vector<std::pair<double, double>> data3 = {{5.0, 6.0}};

    storage_manager->store(111, 1111, data1, 1);
    storage_manager->store(222, 2222, data2, 1);
    storage_manager->store(333, 3333, data3, 1);

    EXPECT_EQ(storage_manager->get_all_data().size(), 3);

    fvm::interfaces::DataNode node;
    ASSERT_TRUE(storage_manager->retrieve(111, node));
    EXPECT_EQ(node.data_hash, 1111);

    ASSERT_TRUE(storage_manager->retrieve(222, node));
    EXPECT_EQ(node.data_hash, 2222);

    ASSERT_TRUE(storage_manager->retrieve(333, node));
    EXPECT_EQ(node.data_hash, 3333);
}

TEST_F(StorageManagerTest, SaveEmptyDataToFile) {
    // Save empty storage
    ASSERT_TRUE(storage_manager->save_to_file(test_data_file));

    // Create new storage manager and load
    storage_manager = std::make_unique<fvm::StorageManager>(mock_logger);
    ASSERT_TRUE(storage_manager->load_from_file(test_data_file, 16));

    // Should have no data
    EXPECT_EQ(storage_manager->get_all_data().size(), 0);
}

#endif // STORAGE_MANAGER_TEST_CPP
