#include <stdint.h>
#include <stdbool.h>

#include "protocol.h"


struct PendingCommand {
    bool isActive;
    time_t last_send_time;
    uint8_t numberSendAttempts;
    uint8_t command_uuid;
} currentCommandStack[NUM_COMMAND_TYPES + 2];

uint8_t command_uuid_counter = 1;