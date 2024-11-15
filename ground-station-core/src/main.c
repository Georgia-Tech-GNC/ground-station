#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <signal.h>
#include <errno.h>
#include <termios.h>
#include <pthread.h>
#include <fcntl.h> 
#include <string.h>
#include <unistd.h>
#ifdef USE_SD_NOTIFY
#include <systemd/sd-daemon.h>
#endif

#include "mongoose.h"  
#include "sqlite3.h"
#include "database.h"
#include "protocol.h"
#include "web_server.h"
#include "packet_encode.h"
#include "command_scheduler.h"
#include "logger.h"
#include "serial.h"

#define IN_TESTING_MODE

#define SERIAL_BUFFER_SIZE 256
#define MESSAGE_BUFFER_SIZE 270
#define LOOP_TIMEOUT_MS 10

uint8_t buffer[SERIAL_BUFFER_SIZE];
uint8_t message[MESSAGE_BUFFER_SIZE];
uint8_t fread_buffer[250];

struct mg_mgr manager;
struct sqlite3 *db = NULL;
pthread_mutex_t db_access_mutex = PTHREAD_MUTEX_INITIALIZER;

#define DB_AQUIRE_LOCK pthread_mutex_lock(&db_access_mutex)
#define DB_RELEASE_LOCK pthread_mutex_unlock(&db_access_mutex)


struct sigaction old_action;

void finalize() {
  DB_AQUIRE_LOCK;
  sqlite3_close(db);
  DB_RELEASE_LOCK;

  mg_mgr_free(&manager);
}

void sigint_handler(int sig_no) {
    log_header("\n\nCTRL-C pressed, terminating ground station...\n");
    sigaction(SIGINT, &old_action, NULL);
    kill(0, SIGINT);

    finalize();
    log_header("Ground station terminated.\n");
}

bool initialize() {
  if (generate_new_database(&db) != PROCESS_SUCCESSFUL_CODE) return false;
  if (!initialize_web_server(&manager, "http://0.0.0.0:8080", db)) return false;
  initialize_command_stack();

  // TODO: Open Serial Port

  return true;
}

uint8_t command_send_buffer[COMMAND_SEND_BUFFER_SIZE];

void process_xbee_message(int message_size_bytes) {
  if (!verify_packet(buffer, message_size_bytes)) {
    DB_AQUIRE_LOCK;
    log_warning_alt_db(db, "Received corrupted message (unable to verify)!\n");
    DB_RELEASE_LOCK;
    return;
  }

  int message_id = extract_packet(buffer, message_size_bytes, message);
  int payload_size = message_size_bytes - 5;
  
  if (message_id == 0) {
    DB_AQUIRE_LOCK;
    log_warning_alt_db(db, "Received corrupted message! (message id was 0)\n");
    DB_RELEASE_LOCK;
    return;
  }

  if (message_id == COMMAND_MSG_ID) {
    // TODO: should the rocket ever send commands?
    DB_AQUIRE_LOCK;
    log_warning_alt_db(db, "Rocket tried to send a command\n");
    DB_RELEASE_LOCK;
  } else if (message_id == COMMAND_ACK_MSG_ID) {
    if (message_size_bytes >= 2 + 5) {
      uint8_t command_id = message[0];
      uint8_t command_uuid = message[1];
      acknowledge_command(command_id, command_uuid);
    } else {
      DB_AQUIRE_LOCK;
      log_warning_alt_db(db, "Rocket tried to acknowledge a command, but packet size was only %d instead of expected %d", message_size_bytes, sizeof(struct CommandStruct) + 5);
      DB_RELEASE_LOCK;
    }
  } else if (is_data_send_msg(message_id)) {
    if (get_msg_size(message_id) > message_size_bytes - 5) {
      DB_AQUIRE_LOCK;
      log_warning_alt_db(db, "Recieved incomplete message! Expected %d bytes from message id %d but only recieved %d\n", get_msg_size(message_id) + 5, message_id, message_size_bytes);
      DB_RELEASE_LOCK;
    } else {
      // Handle incoming data
      DB_AQUIRE_LOCK;
      parse_data_packet_into_sql(db, message_id, message, payload_size);
      DB_RELEASE_LOCK;
    }
  } else {
    DB_AQUIRE_LOCK;
    log_warning_alt_db(db, "Recieved unexpected message id: %d\n", message_id);
    DB_RELEASE_LOCK;
  }
}

void *serial_loop(void *serial_) {
  FILE *serial = (FILE *) serial_;
  int current_buffer_size = 0;

  while (true) {
    // int num_read = fread(fread_buffer, sizeof(uint8_t), 250, serial);
    int num_read = read(fileno(serial), &fread_buffer, 1);

    for (int i = 0; i < num_read; i++) {
      uint8_t byte_read = (uint8_t) fread_buffer[i];

      current_buffer_size = process_incoming_byte(byte_read, buffer, current_buffer_size);
      log_standard("read byte %02x ", byte_read);
      log_standard("now, current_buffer_size=%d\n", current_buffer_size);
      if (current_buffer_size > 255) {
        log_standard("resetting buffer, too big (>255)\n");
        current_buffer_size = 0;
      }
      if (current_buffer_size < 0) {
        if (-current_buffer_size == 260) {
          log_error("something went wrong ): 260-5=255 so overflow\n");
        }
        log_standard("Attempting to process message of size %d\n", -current_buffer_size);
        process_xbee_message(-current_buffer_size);
        current_buffer_size = 0;
      }
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("usage: serial-path serial-baud-number");
    return 1;
  }

  char *serial_path = argv[1];
  int serial_baud_number = atoi(argv[2]);

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

  log_standard("opening %s...\n", serial_path);
  FILE *serial = serial_initialize(serial_path, serial_baud_number);
  if (serial == NULL) {
    log_error("opening serial failed\n");
    return 1;
  }
  log_standard("serial_initialize done\n");

  pthread_t serial_thread;
  pthread_create(&serial_thread, NULL, serial_loop, (void *) serial);

#ifdef USE_SD_NOTIFY
  sd_notifyf(0, "READY=1\nSTATUS=listening on %s with baud %d", serial_path, serial_get_baud_name(serial_baud_number));
#else
  log_standard("ready\n");
#endif
  
  while (true) {
    // Update web server
    update_server(&manager, LOOP_TIMEOUT_MS);

    // Update command stack
    int write_len = update_command_stack(command_send_buffer);

    if (write_len > 0) {
      log_standard("writing %d bytes...\n", write_len);
      errno = 0;
      for (int i = 0; i < write_len; i ++) {
        log_standard("%02x ", command_send_buffer[i]);
      }
      log_standard("\n");
      int retval = write(fileno(serial), command_send_buffer, write_len);
      if (retval != 0) {
        printf("retval=%d\n", retval);
      }
      // fwrite(command_send_buffer, write_len, 1, serial);
      if (errno != 0) {
        perror("command sending failed");
      } else {
        log_standard("wrote %d bytes...\n", write_len);
      }
    }
  }
  return 0;
}
