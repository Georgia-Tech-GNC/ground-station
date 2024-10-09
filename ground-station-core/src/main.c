#include <stdio.h>
#include <time.h>
#include <stdint.h>

#include "mongoose.h"  
#include "sqlite3.h"
#include "database.h"
#include "protocol.h"
#include "web_server.h"
#include "packet_encode.h"
#include "logger.h"


#define SERIAL_BUFFER_SIZE 256
#define MESSAGE_BUFFER_SIZE 270
#define LOOP_TIMEOUT_MS 1000

uint8_t buffer[SERIAL_BUFFER_SIZE];
uint8_t message[MESSAGE_BUFFER_SIZE];

struct mg_mgr manager;
struct sqlite3 *db = NULL;

bool initialize() {
  if (generate_new_database(&db) != PROCESS_SUCCESSFUL_CODE) return false;
  if (!initialize_web_server(&manager, "http://0.0.0.0:8080", db)) return false;

  // TODO: Open Serial Port

  return true;
}

void finalize() {
  sqlite3_close(db);
  mg_mgr_free(&manager);
}

void process_xbee_message(int message_size_bytes) {
  if (!verify_packet(buffer, message_size_bytes)) {
    log_warning("Recieved corrupted message (unable to verify)!\n");
    return;
  }

  int message_id = extract_packet(buffer, message_size_bytes, message);
  
  if (message_id == 0) {
    log_warning("Recieved corrupted message! (message id was 0)\n");
    return;
  } 

  if (get_msg_size(message_id) > message_size_bytes - 5) {
    log_warning("Recieved incomplete message! Expected %d bytes from message id %d but only recieved %d\n", get_msg_size(message_id) + 5, message_id, message_size_bytes);
    return;
  }

  if (message_id == COMMAND_MSG_ID) {
    // TODO: should the rocket ever send commands?
  } else if (message_id == COMMAND_ACK_MSG_ID) {
    // TODO: handle command acknowledgement
  } else if (is_data_send_msg(message_id)) {
    // Handle incoming data
    parse_data_packet_into_sql(db, message_id, message);
  } else {
    log_warning("Recieved unexpected message id: %d\n", message_id);
  }

}

int main(void) {

  log_header("GNC Ground Station v1.0\n");
  log_standard("Initializing...\n\n");

  if (!initialize()) {
    log_error("Aborting!\n");
    return 0;
  }

  log_standard("\nInitialization Complete...\n\n");

  
  for (int i = 0; i < 10; i++) {
    // Update web server
    update_server(&manager, LOOP_TIMEOUT_MS);

    // TODO read bytes from serial, parse them using process_incoming_byte, and 
    // call process_xbee_message when the entire message is in the buffer (the positive buffer size should be the parameter)
  }

  finalize();

  return 0;
}