CXX = g++
CXXFLAGS = -std=c++20 -O2 -Wall -Wextra -Wpedantic -Wshadow -Wformat=2 \
           -Wfloat-equal -Wconversion -Wlogical-op -Wshift-overflow=2 \
           -Wduplicated-cond -Wcast-qual -Wcast-align -Wno-unused-result

TARGET = main
SRC_DIR = src
BUILD_DIR = build
TEST_DIR = test

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

RESET = \033[0m
RED   = \033[031m
GREEN = \033[032m
BLUE  = \033[036m

# Default target
all: $(BUILD_DIR) $(TARGET)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Link
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

# Compile
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Debug build
debug: CXXFLAGS += -g -DDEBUG -fsanitize=address,undefined
debug: clean all

# Run
run: all
	./$(TARGET)

test: all
	@total=0; passed=0; failed=0; \
	echo "$(BLUE)================= Running tests =================$(RESET)"; \
	for test_file in $(TEST_DIR)/*.in; do \
		total=$$((total + 1)); \
		name=$$(basename "$$test_file"); \
		expected="$${test_file%.in}.out"; \
		actual=$$(./$(TARGET) < $$test_file); \
		expected_content=$$(cat $$expected); \
		echo "$(BLUE)Running $$name...$(RESET)"; \
		if [ "$$actual" = "$$expected_content" ]; then \
			echo "  $(GREEN)PASSED$(RESET)"; \
			passed=$$((passed + 1)); \
		else \
			echo "  $(RED)FAILED$(RESET)"; \
			echo "  $(RED)Expected:$(RESET) $$expected_content"; \
			echo "  $(RED)Got:$(RESET)      $$actual"; \
			failed=$$((failed + 1)); \
		fi; \
	done; \
	echo "$(BLUE)=================================================$(RESET)"; \
	echo "$(GREEN)PASSED $$passed/$$total$(RESET)"; \
	if [ $$failed -ne 0 ]; then \
		echo "$(RED)$$failed tests failed$(RESET)"; \
		exit 1; \
	else \
		echo "$(GREEN)All tests passed ✔$(RESET)"; \
	fi

# Clean
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: all debug run test clean
