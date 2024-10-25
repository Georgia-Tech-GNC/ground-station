#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <signal.h>
#include <errno.h>

#include "mongoose.h"  
#include "sqlite3.h"
#include "database.h"
#include "protocol.h"
#include "web_server.h"
#include "packet_encode.h"
#include "command_scheduler.h"
#include "logger.h"


#define SERIAL_BUFFER_SIZE 256
#define MESSAGE_BUFFER_SIZE 270
#define LOOP_TIMEOUT_MS 10

uint8_t buffer[SERIAL_BUFFER_SIZE];
uint8_t message[MESSAGE_BUFFER_SIZE];
uint8_t fread_buffer[250];

struct mg_mgr manager;
struct sqlite3 *db = NULL;

struct sigaction old_action;

char *serialPath;

void finalize() {
  sqlite3_close(db);
  mg_mgr_free(&manager);
}

void sigint_handler(int sig_no)
{
    log_header("\n\nCTRL-C pressed, terminating ground station...\n");
    sigaction(SIGINT, &old_action, NULL);
    kill(0, SIGINT);

    finalize();
    log_header("Ground station terminated.");
}

bool initialize() {
  if (generate_new_database(&db) != PROCESS_SUCCESSFUL_CODE) return false;
  set_sql_database(db);
  if (!initialize_web_server(&manager, "http://0.0.0.0:8080", db)) return false;
  initialize_command_stack();

  // TODO: Open Serial Port

  return true;
}

uint8_t command_send_buffer[COMMAND_SEND_BUFFER_SIZE];

void process_xbee_message(int message_size_bytes) {
  if (!verify_packet(buffer, message_size_bytes)) {
    log_warning("Received corrupted message (unable to verify)!\n");
    return;
  }

  int message_id = extract_packet(buffer, message_size_bytes, message);
  int payload_size = message_size_bytes - 5;
  
  if (message_id == 0) {
    log_warning("Received corrupted message! (message id was 0)\n");
    return;
  }

  if (message_id == COMMAND_MSG_ID) {
    // TODO: should the rocket ever send commands?
    log_warning("Rocket tried to send a command");
  } else if (message_id == COMMAND_ACK_MSG_ID) {
    if (message_size_bytes >= 2 + 5) {
      uint8_t command_id = message[0];
      uint8_t command_counter = message[1];
      acknowledge_command(command_id, command_counter);
    } else {
      log_warning("Rocket tried to acknowledge a command, but packet size was only %d instead of expected %d", message_size_bytes, sizeof(struct CommandStruct) + 5);
    }
  } else if (is_data_send_msg(message_id)) {
    if (get_msg_size(message_id) > message_size_bytes - 5) {
      log_warning("Recieved incomplete message! Expected %d bytes from message id %d but only recieved %d\n", get_msg_size(message_id) + 5, message_id, message_size_bytes);
    } else {
      // Handle incoming data
      parse_data_packet_into_sql(db, message_id, message, payload_size);
    }
  } else {
    log_warning("Recieved unexpected message id: %d\n", message_id);
  }
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("must have one argument for serial path\n");
    return 1;
  }

  serialPath = argv[1];

  // Set up exit code
  struct sigaction action;
  memset(&action, 0, sizeof(action));
  action.sa_handler = &sigint_handler;
  sigaction(SIGINT, &action, &old_action);

  log_header("GNC Ground Station v1.0\n");
  log_standard("Press ctrl-c to terminate\n");
  log_standard("Initializing...\n\n");

  if (!initialize()) {
    log_error("Aborting!\n");
    return 0;
  }

  log_standard("\nInitialization Complete...\n\n");

  log_standard("opening %s...\n", serialPath);
  FILE *serial = fopen(serialPath, "rb");
  if (serial == NULL) {
    perror("open serial file");
    return 1;
  }
  int current_buffer_size = 0;
  
  while (true) {
    // Update web server
    update_server(&manager, LOOP_TIMEOUT_MS);

    if (fread(fread_buffer, sizeof(uint8_t), 1, serial) > 0) {
      unsigned char byte_read = (unsigned char) fread_buffer[0];

      log_standard("read byte %02x\n", byte_read);
      current_buffer_size = process_incoming_byte(byte_read, buffer, current_buffer_size);
      log_standard("now, current_buffer_size=%d\n", current_buffer_size);
      if (current_buffer_size > 255) {
        log_standard("resetting buffer, too big (>255)\n");
        current_buffer_size = 0;
      }
      if (current_buffer_size < 0) {
        if (-current_buffer_size == 260) {
          log_error("something went wrong ): 260-5=255 so overfloe QED\n");
        }
        log_standard("Attempting to process message of size %d\n", -current_buffer_size);
        process_xbee_message(-current_buffer_size);
        current_buffer_size = 0;
      }
    }
    if (errno != 0) {
      perror("read byte");
    }

    // Update command stack
    update_command_stack(command_send_buffer);
  }


  return 0;
}
