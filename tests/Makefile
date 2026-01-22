# GTest configuration
GTEST_DIR = /opt/homebrew/include
GTEST_LIBS = -lgtest -lgtest_main -pthread
GTEST_LDFLAGS = -L/opt/homebrew/lib

# Test sources (exclude saver_test.cpp for now - Saver class is defined in saver.cpp)
TEST_SOURCES = $(filter-out unit/saver_test.cpp, $(wildcard unit/*.cpp))
TEST_OBJECTS = $(TEST_SOURCES:.cpp=.o)

# Only compile standalone sources that don't include other .cpp files
# NOTE: saver.cpp includes logger.cpp, so we don't include logger.o separately
MAIN_OBJECTS = \
	../build/random.o \
	../build/system_clock.o \
	../build/encryptor.o \
	../build/data_serializer.o \
	../build/wal_manager.o \
	../build/storage_manager.o \
	../build/saver.o

# Compiler flags
CXXFLAGS = -std=c++17 -I../include -I. -I$(GTEST_DIR) -g -Wall -Wextra
LDFLAGS = $(GTEST_LDFLAGS) $(GTEST_LIBS)

# Main target
test_runner: test_main.o $(TEST_OBJECTS) $(MAIN_OBJECTS)
	g++ $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# Compile test files
%.o: %.cpp
	g++ $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(TEST_OBJECTS) test_main.o test_runner

# Run all tests
.PHONY: test clean
test: test_runner
	./test_runner

# Debug: show what would be compiled
.PHONY: debug
debug:
	@echo "TEST_SOURCES: $(TEST_SOURCES)"
	@echo "TEST_OBJECTS: $(TEST_OBJECTS)"
	@echo "MAIN_OBJECTS: $(MAIN_OBJECTS)"
