# GTest configuration
GTEST_DIR = /opt/homebrew/include
GTEST_LIBS = -lgtest -lgtest_main -pthread
GTEST_LDFLAGS = -L/opt/homebrew/lib

# Test sources
TEST_SOURCES = $(wildcard unit/*.cpp)
TEST_OBJECTS = $(TEST_SOURCES:.cpp=.o)

# Only compile standalone sources that don't include other .cpp files
MAIN_OBJECTS = \
	../lib/random.o \
	../lib/system_clock.o

# Compiler flags
CXXFLAGS = -std=c++17 -I../include -I. -I$(GTEST_DIR) -g -Wall
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
