#ifndef COMMAND_SCHEDULER_H
#define COMMAND_SCHEDULER_H
#include "protocol.h"

#define COMMAND_RESEND_TIMEOUT_MS 100
// set to 100ms, 15 resends because it works better (:
#define COMMAND_SEND_BUFFER_SIZE NUM_COMMAND_TYPES*7 + 10
#define MAX_COMMAND_RESENDS 15

void initialize_command_stack();

void send_command(int command_id);

int update_command_stack(uint8_t* buffer);

void acknowledge_command(int command_id, int command_uuid);

#endif
