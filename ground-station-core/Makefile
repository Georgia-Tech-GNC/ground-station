ifeq (${CROSS},no)
	BUILD_DIR ?= build
else
	CC=${CROSS}-gcc
	CXX=${CROSS}-g++
	AS=${CROSS}-as
	BUILD_DIR ?= build-${CROSS}
endif

TARGET_EXEC ?= a.out

SRC_DIRS ?= src includes lib

SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP

CFLAGS := -g -Wall -Wextra -Wpedantic -Wno-newline-eof -Wno-unused-parameter -Wno-strict-prototypes

ifeq (${USE_SD_NOTIFY},yes)
	LDFLAGS += "-lsystemd"
	CFLAGS += "-DUSE_SD_NOTIFY"
endif

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# assembly
$(BUILD_DIR)/%.s.o: %.s
	$(MKDIR_P) $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

# c source
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

install: $(BUILD_DIR)/$(TARGET_EXEC)
	mkdir $(out)/bin
	cp $< $(out)/bin/ground-station-core

.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p
