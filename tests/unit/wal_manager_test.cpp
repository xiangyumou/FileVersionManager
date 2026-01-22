#ifndef WAL_MANAGER_TEST_CPP
#define WAL_MANAGER_TEST_CPP

#include "fvm/wal_manager.h"
#include "../mocks/mock_file_operations.h"
#include "../mocks/mock_logger.h"
#include <gtest/gtest.h>
#include <cstdio>

class WalManagerTest : public ::testing::Test {
protected:
    MockFileOperations mock_file_ops;
    fvm::mocks::MockLogger mock_logger;
    std::unique_ptr<fvm::WalManager> wal_manager;
    std::string test_wal_file = "test_wal_manager.wal";

    void SetUp() override {
        wal_manager = std::make_unique<fvm::WalManager>(
            test_wal_file, mock_logger, &mock_file_ops);
    }

    void TearDown() override {
        wal_manager.reset();
        // Clean up test file
        std::remove(test_wal_file.c_str());
    }
};

TEST_F(WalManagerTest, AppendEntryIncreasesCount) {
    fvm::interfaces::WalEntry entry;
    entry.op = fvm::interfaces::WalOperation::INSERT;
    entry.name_hash = 123;
    entry.data_hash = 456;
    entry.len = 1;
    entry.data = {{1.0, 2.0}};

    ASSERT_TRUE(wal_manager->append_entry(entry));
    EXPECT_EQ(wal_manager->get_entry_count(), 1);
}

TEST_F(WalManagerTest, AppendMultipleEntries) {
    for (int i = 0; i < 5; i++) {
        fvm::interfaces::WalEntry entry;
        entry.op = fvm::interfaces::WalOperation::INSERT;
        entry.name_hash = i;
        entry.data_hash = i * 100;
        entry.len = 1;
        entry.data = {{static_cast<double>(i), 0.0}};

        ASSERT_TRUE(wal_manager->append_entry(entry));
    }

    EXPECT_EQ(wal_manager->get_entry_count(), 5);
}

TEST_F(WalManagerTest, LoadAndReplayInvokesCallback) {
    // Write test data directly to a temp file
    std::string wal_content = "1 789 101112 1 3 4\n";
    std::ofstream out(test_wal_file);
    out << wal_content;
    out.close();

    // Create new wal_manager to simulate restart
    wal_manager = std::make_unique<fvm::WalManager>(
        test_wal_file, mock_logger, &mock_file_ops);

    // Replay
    bool callback_invoked = false;
    unsigned long long captured_hash = 0;

    auto callback = [&](const fvm::interfaces::WalEntry& e) {
        callback_invoked = true;
        captured_hash = e.name_hash;
    };

    ASSERT_TRUE(wal_manager->load_and_replay(callback));
    EXPECT_TRUE(callback_invoked);
    EXPECT_EQ(captured_hash, 789);
}

TEST_F(WalManagerTest, ClearWAL) {
    fvm::interfaces::WalEntry entry;
    entry.op = fvm::interfaces::WalOperation::INSERT;
    entry.name_hash = 999;
    entry.data_hash = 888;
    entry.len = 1;
    entry.data = {{5.0, 6.0}};

    wal_manager->append_entry(entry);
    EXPECT_GT(wal_manager->get_entry_count(), 0);

    wal_manager->clear();

    // Create new wal_manager to verify file is empty
    wal_manager = std::make_unique<fvm::WalManager>(
        test_wal_file, mock_logger, &mock_file_ops);

    bool callback_invoked = false;
    auto callback = [&](const fvm::interfaces::WalEntry& e) {
        callback_invoked = true;
    };

    wal_manager->load_and_replay(callback);
    EXPECT_FALSE(callback_invoked);
}

TEST_F(WalManagerTest, DisabledWALDoesNotWrite) {
    wal_manager->set_enabled(false);

    fvm::interfaces::WalEntry entry;
    entry.op = fvm::interfaces::WalOperation::INSERT;
    entry.name_hash = 111;
    entry.data_hash = 222;
    entry.len = 1;
    entry.data = {{7.0, 8.0}};

    ASSERT_TRUE(wal_manager->append_entry(entry));
    // Entry count should NOT increase when WAL is disabled
    EXPECT_EQ(wal_manager->get_entry_count(), 0);
}

TEST_F(WalManagerTest, SetAutoCompactThreshold) {
    wal_manager->set_auto_compact_threshold(50);
    EXPECT_EQ(wal_manager->get_auto_compact_threshold(), 50);

    wal_manager->set_auto_compact_threshold(200);
    EXPECT_EQ(wal_manager->get_auto_compact_threshold(), 200);
}

TEST_F(WalManagerTest, WALOperations) {
    fvm::interfaces::WalEntry insert_entry;
    insert_entry.op = fvm::interfaces::WalOperation::INSERT;
    insert_entry.name_hash = 100;
    insert_entry.data_hash = 200;
    insert_entry.len = 1;
    insert_entry.data = {{9.0, 10.0}};

    fvm::interfaces::WalEntry update_entry;
    update_entry.op = fvm::interfaces::WalOperation::UPDATE;
    update_entry.name_hash = 100;
    update_entry.data_hash = 300;
    update_entry.len = 1;
    update_entry.data = {{11.0, 12.0}};

    fvm::interfaces::WalEntry delete_entry;
    delete_entry.op = fvm::interfaces::WalOperation::DELETE;
    delete_entry.name_hash = 100;
    delete_entry.data_hash = 0;
    delete_entry.len = 0;

    ASSERT_TRUE(wal_manager->append_entry(insert_entry));
    ASSERT_TRUE(wal_manager->append_entry(update_entry));
    ASSERT_TRUE(wal_manager->append_entry(delete_entry));

    EXPECT_EQ(wal_manager->get_entry_count(), 3);
}

#endif // WAL_MANAGER_TEST_CPP
