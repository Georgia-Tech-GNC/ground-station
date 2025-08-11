# Define the name of the final executable
TARGET_EXECUTABLE_NAME := ground_station

# Define the source and build folders
BUILD_DIR := ./build
SRC_DIRS := ./source
INC_DIRS := ./include/database ./include/globals ./include

# Find all source files
ALL_SRC_FILES := $(shell find $(SRC_DIRS) -name '*.cpp' -or -name '*.c')

# Generate all object file names by inserting the source name between BUILD_DIR and .o
ALL_OBJ_FILES := $(ALL_SRC_FILES:%=$(BUILD_DIR)/%.o)
DEPS := $(ALL_OBJ_FILES:.o=.d) # Create dependency files by substituting .o for .d

# Compiler flags
CC := gcc
CXX := g++

CFLAGS := -Wall -Wmissing-declarations
CXXFLAGS := $(CFLAGS) -std=c++17
CPPFLAGS := $(addprefix -I,$(INC_DIRS)) -MMD -MP


# Final multi-purpose build step from object files
$(BUILD_DIR)/$(TARGET_EXECUTABLE_NAME): $(ALL_OBJ_FILES)
	$(CXX) $(ALL_OBJ_FILES) -o $@ $(LDFLAGS)

# C build step
$(BUILD_DIR)/%.c.o: %.c	
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# C++ build step
$(BUILD_DIR)/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -r $(BUILD_DIR)

# Suppress errors regarding missing .d makefiles -- we probably don't need this
-include $(DEPS)