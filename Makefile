# FileVersionManager Makefile
# Community best practices for C++ project building
#
# NOTE: This codebase uses an unusual pattern where main.cpp includes other .cpp files.
# Standard separate compilation is not possible without refactoring.

# ============================================================================
# Compiler & Flags
# ============================================================================
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g
INCLUDE_DIRS = -Iinclude

# GTest configuration (used by test target)
GTEST_DIR = /opt/homebrew/include
GTEST_LIBS = -lgtest -lgtest_main -pthread
GTEST_LDFLAGS = -L/opt/homebrew/lib

# ============================================================================
# Directories & Files
# ============================================================================
BUILD_DIR = build
TARGET = $(BUILD_DIR)/ffvms

# Find all source files (both lib/ and lib/repositories/)
# These are compiled as object files for tests
LIB_SRCS = $(wildcard lib/*.cpp lib/repositories/*.cpp)
LIB_OBJS = $(addprefix $(BUILD_DIR)/,$(notdir $(LIB_SRCS:.cpp=.o)))

# Standalone source files (NOT included in main.cpp, must be compiled separately)
# NOTE: saver.cpp includes logger.cpp, so logger is NOT standalone
STANDALONE_SRCS = \
	lib/random.cpp \
	lib/system_clock.cpp \
	lib/data_serializer.cpp \
	lib/wal_manager.cpp \
	lib/storage_manager.cpp \
	lib/logger.cpp \
	lib/encryptor.cpp \
	lib/saver.cpp
STANDALONE_OBJS = $(addprefix $(BUILD_DIR)/,$(notdir $(STANDALONE_SRCS:.cpp=.o)))

# For main build, exclude logger and saver since they're included in other files
MAIN_BUILD_SRCS = \
	lib/random.cpp \
	lib/system_clock.cpp \
	lib/data_serializer.cpp \
	lib/wal_manager.cpp \
	lib/storage_manager.cpp
MAIN_BUILD_OBJS = $(addprefix $(BUILD_DIR)/,$(notdir $(MAIN_BUILD_SRCS:.cpp=.o)))

# ============================================================================
# Build Rules
# ============================================================================

# Default target: build the main application
all: $(TARGET)

# Build main application
# NOTE: main.cpp includes most .cpp files directly. We compile standalone .cpp files
# separately and link them together (this is necessary due to the current code structure)
$(TARGET): $(MAIN_BUILD_OBJS) main.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) $(MAIN_BUILD_OBJS) main.cpp -o $@
	@echo "Built: $@"

# Compile standalone .cpp files to .o files
$(BUILD_DIR)/%.o: lib/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) -c $< -o $@

# Compile repository .cpp files to .o files (for tests)
$(BUILD_DIR)/%.o: lib/repositories/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) -c $< -o $@

# ============================================================================
# Test Target
# ============================================================================

# Run tests (delegate to tests/Makefile)
.PHONY: test
test: $(STANDALONE_OBJS)
	@echo "Running tests..."
	$(MAKE) -C tests test

# ============================================================================
# Utility Targets
# ============================================================================

# Remove build artifacts
.PHONY: clean
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILD_DIR)
	$(MAKE) -C tests clean
	@echo "Clean complete."

# Show help information
.PHONY: help
help:
	@echo "FileVersionManager Makefile"
	@echo ""
	@echo "Usage: make [target]"
	@echo ""
	@echo "Targets:"
	@echo "  all (default)  - Build the main application ($(TARGET))"
	@echo "  test           - Build and run all tests"
	@echo "  clean          - Remove all build artifacts"
	@echo "  help           - Show this help message"
	@echo ""
	@echo "Examples:"
	@echo "  make           # Build the application"
	@echo "  make test      # Run tests"
	@echo "  make clean     # Clean build directory"
