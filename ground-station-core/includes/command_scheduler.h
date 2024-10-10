#ifndef COMMAND_SCHEDULER_H
#define COMMAND_SCHEDULER_H
#include "protocol.h"

#define COMMAND_RESEND_TIMEOUT_MS 500
#define COMMAND_SEND_BUFFER_SIZE NUM_COMMAND_TYPES*7 + 10
#define MAX_COMMAND_RESENDS 5

void initialize_command_stack();
bool send_command(int command_id);
int update_command_stack(uint8_t* buffer);
void acknowledge_command(int command_id, int command_uuid);

#endif