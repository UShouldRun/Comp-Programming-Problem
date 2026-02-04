CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Wpedantic -Wshadow -Wformat=2 \
           -Wfloat-equal -Wconversion -Wlogical-op -Wshift-overflow=2 \
           -Wduplicated-cond -Wcast-qual -Wcast-align -Wno-unused-result

BUILD_DIR = build
SRC_DIR   = src
TEST_DIR  = test

RESET = \033[0m
RED   = \033[031m
GREEN = \033[032m
BLUE  = \033[036m

# Source files
MAIN_SRC = $(SRC_DIR)/main.cpp
TEST_SRC = $(SRC_DIR)/test.cpp

# Object files
MAIN_OBJ = $(BUILD_DIR)/main.o
TEST_OBJ = $(BUILD_DIR)/test.o

COMMIT_MSG =?

# Targets
all: $(BUILD_DIR) main test_runner

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# ------------------------
# Main binary
main: $(MAIN_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(MAIN_OBJ): $(MAIN_SRC)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ------------------------
# Test binary
test_runner: $(TEST_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(TEST_OBJ): $(TEST_SRC)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ------------------------
# Debug build
debug: CXXFLAGS += -g -DDEBUG -fsanitize=address,undefined
debug: clean all

# ------------------------
# Run main
run: main
	./main

# ------------------------
test: all 
	@total=0; passed=0; failed=0; \
	echo "$(BLUE)================= Running tests =================$(RESET)"; \
	for test_file in $(TEST_DIR)/*.in; do \
		total=$$((total + 1)); \
		name=$$(basename "$$test_file"); \
		expected="$${test_file%.in}.out"; \
		actual=$$(./main < $$test_file); \
		expected_content=$$(cat $$expected); \
		echo "$(BLUE)Running $$name...$(RESET)"; \
		if [ "$$actual" = "$$expected_content" ]; then \
			echo " $(GREEN)PASSED$(RESET)"; \
			passed=$$((passed + 1)); \
		else \
			echo " $(RED)FAILED$(RESET)"; \
			echo " $(RED)Expected:$(RESET) $$expected_content"; \
			echo " $(RED)Got:$(RESET) $$actual"; \
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

commit:
	@git add .
	@git commit -m "$(COMMIT_MSG)"

push: commit
	@git push origin main

# ------------------------
# Clean
clean:
	rm -rf $(BUILD_DIR) main test_runner

.PHONY: all debug run test clean commit push
