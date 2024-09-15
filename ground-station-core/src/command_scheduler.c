// I AM BROKEN!!!!!!!!!!!!!!!!
#include <stdint.h>
#include <time.h>
#include <stdbool.h>

#define RESEND_MESSAGE_TIMEOUT_MS 250
#define ATTEMPT_MESSAGE_SENDS 5 

#define MAX_SIMULTANEOUS_MESSAGE_SENDS 5

struct PendingCommand {
    time_t last_send_time;
    uint8_t commandId;
    uint8_t numberSendAttempts;
} currentCommandStack[MAX_SIMULTANEOUS_MESSAGE_SENDS];

uint8_t num_command_in_stack = 0;

void reset_command_scheduler() {
    num_command_in_stack = 0;
}


bool addToCommandStack(uint8_t commandId) {
    if (num_command_in_stack >= MAX_SIMULTANEOUS_MESSAGE_SENDS) return false;
    for (int i = 0; i < num_command_in_stack; i++) {
        if (currentCommandStack[i].commandId == commandId) return false;
    }

    currentCommandStack[num_command_in_stack].last_send_time = time(NULL);
    currentCommandStack[num_command_in_stack].commandId = commandId;
    currentCommandStack[num_command_in_stack].numberSendAttempts = 1;

    send_command(num_command_in_stack++);

    return true;
}

bool recieve_command_confirmation(uint8_t commandId) {
    for (int i = 0; i < num_command_in_stack; i++) {
        if (currentCommandStack[i].commandId == commandId) {
            confirm_command_send(commandId);

            for (int j = i; j < num_command_in_stack-1; j++) {
                currentCommandStack[j] = currentCommandStack[j+1];
            }

            num_command_in_stack--;
            return true;
        }
    }

    return false;
}

void resend_state_commands() {
    time_t current_time = TIME(NULL);

    for (int i = 0; i < num_command_in_stack; i++) {
        if (current_time - currentCommandStack[i].last_send_time >= RESEND_MESSAGE_TIMEOUT_MS) {
            send_command(i);
        }
    }
}

void send_command(uint8_t command_stack_index) {
    // TODO
}

void confirm_command_send(uint8_t commandId) {
    // TODO
}
