#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#include "command_scheduler.h"
#include "packet_encode.h"
#include "protocol.h"
#include "logger.h"

struct PendingCommand {
    bool is_active;
    clock_t last_send_time;
    uint8_t numberSendAttempts;
    uint8_t command_uuid;
} currentCommandStack[NUM_COMMAND_TYPES + 2];

uint8_t command_uuid_counter = 1;

int64_t get_time_ms() {
    struct timespec start;
    clock_gettime( CLOCK_REALTIME, &start );
    return (start.tv_sec*1000) + (start.tv_nsec/1000000);
}

void initialize_command_stack() {
    command_uuid_counter = 1;

    for (int i = 1; i <= NUM_COMMAND_TYPES; i++) {
        currentCommandStack[i].is_active = false;
    }
}

void send_command(int command_id) {
    if (command_id < 1 || command_id > NUM_COMMAND_TYPES) log_warning("Tried to send a command of invalid type %d\n", command_id); // Invalid command id
    else if (currentCommandStack[command_id].is_active) log_warning("Tried to send command of type %d, which is already being sent\n", command_id); // Command is already being sent 
    else {

        log_debug("Added a command of type %d and uuid %d to the command stack\n", command_id, command_uuid_counter);
        currentCommandStack[command_id].is_active = true;
        currentCommandStack[command_id].last_send_time = get_time_ms() - COMMAND_RESEND_TIMEOUT_MS*3;
        currentCommandStack[command_id].numberSendAttempts = 0;
        currentCommandStack[command_id].command_uuid = command_uuid_counter++;
    }
}

void acknowledge_command(int command_id, int command_uuid) {
    if (command_id < 1 || command_id > NUM_COMMAND_TYPES) log_warning("Tried to acknowledge invalid type %d\n", command_id); // Invalid command id
    else {
        if (!currentCommandStack[command_id].is_active || currentCommandStack[command_id].command_uuid != command_uuid) {
            log_warning("Tried to acknowledge a command with valid type %d, but with an invalid uuid %d\n", command_id, command_uuid);
        } else {
            currentCommandStack[command_id].is_active = false;
            log_debug("Acknowledged command of type %d with uuid %d\n", command_id, command_uuid);
        }
    }
}

int update_command_stack(uint8_t* buffer) {
    int buffer_size = 0;
    int64_t current_time = get_time_ms();
    for (int i = 1; i <= NUM_COMMAND_TYPES; i++) {
        if (!currentCommandStack[i].is_active || current_time - currentCommandStack[i].last_send_time < COMMAND_RESEND_TIMEOUT_MS) continue;

        log_debug("Resending command of type %d\n", i);

        uint8_t cmd_payload[2];
        generate_command_payload(cmd_payload, i, currentCommandStack[i].command_uuid);
        buffer_size += generate_packet(cmd_payload, 2, buffer+buffer_size, COMMAND_MSG_ID);

        currentCommandStack[i].numberSendAttempts++;
        currentCommandStack[i].last_send_time = current_time;
        if (currentCommandStack[i].numberSendAttempts >= MAX_COMMAND_RESENDS) {
            currentCommandStack[i].is_active = false;
            log_warning("Removed cmd of type %d from stack because it was resent %d times without a response\n", i, MAX_COMMAND_RESENDS);
        }
    }

    return buffer_size;
}

