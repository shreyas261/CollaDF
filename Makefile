# ==========================================
# Pure C++ CollaDF Makefile
# ==========================================

CXX = g++
# -O3 and -march=native ensure maximum SIMD vectorization speed
CXXFLAGS = -std=c++17 -O3 -march=native -Wall -Wextra -pthread
INCLUDES = -Iinclude

# Directories
SRC_DIR = src
TEST_DIR = tests
BENCH_DIR = benchmarks
BUILD_DIR = build

# Core Library Sources & Objects
CORE_SRCS = $(SRC_DIR)/DataFrame.cpp $(SRC_DIR)/GroupBy.cpp $(SRC_DIR)/Reader.cpp
CORE_OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(CORE_SRCS))

# Main Execution Source 
# (Currently using benchmark.cpp since it has your main() function)
MAIN_SRC = $(BENCH_DIR)/benchmark.cpp
MAIN_OBJ = $(BUILD_DIR)/benchmark.o

# Output Executable Name
TARGET = colladf

# Test Sources & Binaries
TEST_SRCS = $(TEST_DIR)/test_dataframe.cpp $(TEST_DIR)/test_math.cpp
TEST_BINS = $(patsubst $(TEST_DIR)/%.cpp, $(BUILD_DIR)/%, $(TEST_SRCS))

# ==========================================
# Rules
# ==========================================

.PHONY: all clean test run

# Default target builds the 'colladf' executable
all: $(TARGET)

# Compile Core C++ Object Files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling $<..."
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Compile the Main file
$(MAIN_OBJ): $(MAIN_SRC)
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling $<..."
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Link the final 'colladf' executable
$(TARGET): $(CORE_OBJS) $(MAIN_OBJ)
	@echo "Linking executable $@"
	@$(CXX) $(CXXFLAGS) $^ -o $@
	@echo "Build successful! Run with: ./$(TARGET)"


# Compile C++ Tests
$(BUILD_DIR)/%: $(TEST_DIR)/%.cpp $(CORE_OBJS)
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling test $<..."
	@$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@ -lgtest -lgtest_main -pthread

# Run Tests
test: $(TEST_BINS)
	@echo "Running tests..."
	@for t in $(TEST_BINS); do \
		echo "Executing $$t..."; \
		./$$t || exit 1; \
	done


run: $(TARGET)
	./$(TARGET)

clean:
	@echo "Cleaning up..."
	@rm -rf $(BUILD_DIR) $(TARGET)
	@rm -f gtest_temp.csv test_dummy.csv