BUILD_DIR ?= ./build

SRCS := ./src/command_scheduler.c \
				./src/database.c \
				./src/logger.c \
				./src/web_server.c \
				./lib/mongoose/mongoose.c \
				./lib/sqlite3/sqlite3.c \
				./lib/protocol/crc_hash.c \
				./lib/protocol/packet_encode.c \
				./lib/protocol/protocol.c

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := ./includes/ \
						./lib/mongoose/ \
						./lib/protocol/ \
						./lib/sqlite3/
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP

CFLAGS := -g

$(BUILD_DIR)/a.out: ./src/main.c $(OBJS)
	$(CC) $? $(CPPFLAGS) $(CFLAGS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/fake_rocket.out: ./src/fake_rocket_main.c $(OBJS)
	$(CC) $? $(CPPFLAGS) $(CFLAGS) -o $@ $(LDFLAGS)

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

install: $(BUILD_DIR)/a.out
	mkdir $(out)/bin
	cp $< $(out)/bin/ground-station-core

.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p
