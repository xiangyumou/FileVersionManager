#include "mocks/mock_logger.h"
#include <gtest/gtest.h>

using namespace fvm::mocks;
using namespace fvm::interfaces;

class MockLoggerTest : public ::testing::Test {
protected:
    MockLogger logger;
};

TEST_F(MockLoggerTest, InitiallyEmpty) {
    EXPECT_EQ(logger.get_log_count(), 0);
    EXPECT_FALSE(logger.contains("anything"));
}

TEST_F(MockLoggerTest, LogInfoIncrementsCount) {
    logger.info("test message");
    EXPECT_EQ(logger.get_log_count(), 1);
    EXPECT_TRUE(logger.contains("test message"));
}

TEST_F(MockLoggerTest, ClearLogsEmptiesStorage) {
    logger.info("message1");
    logger.info("message2");
    EXPECT_EQ(logger.get_log_count(), 2);

    logger.clear_logs();
    EXPECT_EQ(logger.get_log_count(), 0);
}

TEST_F(MockLoggerTest, CountAtLevel) {
    logger.info("info msg");
    logger.debug("debug msg");
    logger.warning("warning msg");
    logger.fatal("fatal msg");

    EXPECT_EQ(logger.count_at_level(LogLevel::INFO), 1);
    EXPECT_EQ(logger.count_at_level(LogLevel::DEBUG), 1);
    EXPECT_EQ(logger.count_at_level(LogLevel::WARNING), 1);
    EXPECT_EQ(logger.count_at_level(LogLevel::FATAL), 1);
}

TEST_F(MockLoggerTest, SetLogFileWorks) {
    EXPECT_TRUE(logger.set_log_file("/tmp/test.log"));
    EXPECT_EQ(logger.get_log_file(), "/tmp/test.log");
}

TEST_F(MockLoggerTest, SetLogFileCanFail) {
    logger.set_fail_on_set_log_file(true);
    EXPECT_FALSE(logger.set_log_file("/tmp/test.log"));
}

TEST_F(MockLoggerTest, SetMinLogLevel) {
    EXPECT_TRUE(logger.set_min_log_level(LogLevel::WARNING));
    EXPECT_EQ(logger.get_min_log_level(), LogLevel::WARNING);
}

TEST_F(MockLoggerTest, SetConsoleOutput) {
    EXPECT_TRUE(logger.set_console_output(true));
    EXPECT_TRUE(logger.get_console_output());

    EXPECT_TRUE(logger.set_console_output(false));
    EXPECT_FALSE(logger.get_console_output());
}

TEST_F(MockLoggerTest, SilentMode) {
    logger.set_silent(true);
    logger.info("silent message");
    EXPECT_EQ(logger.get_log_count(), 1);
}
