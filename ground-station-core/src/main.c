#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <signal.h>
#include <errno.h>
#include <termios.h>
#include <pthread.h>

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
int serialBaudNumber;

void finalize() {
  sqlite3_close(db);
  mg_mgr_free(&manager);
}

void sigint_handler(int sig_no) {
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

FILE *serial_initialize() {
  errno = 0;
  FILE *serial = fopen(serialPath, "rb");
  if (serial == NULL || errno != 0) {
    perror("open serial file");
    return NULL;
  }
  
  // serial port setup
  struct termios sp;
  log_standard("tcgetattr...\n");
  errno = 0;
  tcgetattr(fileno(serial), &sp);
  if (errno != 0) {
    perror("tcgetattr");
    return NULL;
  }
  log_standard("tcgetattr done.\n");

  sp.c_cflag &= ~PARENB;
  sp.c_cflag &= ~CSTOPB;
  sp.c_cflag &= ~CSIZE;
  sp.c_cflag |= CS8;
  sp.c_cflag &= ~CRTSCTS;
  sp.c_cflag |= CREAD | CLOCAL;
  sp.c_lflag &= ~ICANON; // non-canonical req'd to NOT read line-by-line
  sp.c_lflag &= ~ECHO;
  sp.c_lflag &= ~ISIG;
  sp.c_iflag &= ~(IXON | IXOFF | IXANY); // software control flow
  sp.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes
  sp.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
  sp.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
  sp.c_cc[VTIME] = 1; // wait for 1+ bytes a max of 1/10 sec
  sp.c_cc[VMIN] = 1;

  log_standard("tcsetattr...\n");
  errno = 0;
  tcsetattr(fileno(serial), TCSANOW, &sp);
  if (errno != 0) {
    perror("tcsetattr");
    return NULL;
  }
  log_standard("tcsetattr done.\n");

  speed_t speeeeed;
  if (serialBaudNumber == 0) speeeeed = B1200;
  else if (serialBaudNumber == 1) speeeeed = B2400;
  else if (serialBaudNumber == 2) speeeeed = B4800;
  else if (serialBaudNumber == 3) speeeeed = B9600;
  else if (serialBaudNumber == 4) speeeeed = B19200;
  else if (serialBaudNumber == 5) speeeeed = B38400;
  else if (serialBaudNumber == 6) speeeeed = B57600;
  else if (serialBaudNumber == 7) speeeeed = B115200;
  else {
    printf("unsupported baud rate: %d\n", serialBaudNumber);
    exit(1);
  }

  errno = 0;
  cfsetispeed(&sp, speeeeed);
  if (errno != 0) {
    perror("cfsetispeed");
    return NULL;
  }
  log_standard("cfsetIspeed done.\n");

  errno = 0;
  cfsetospeed(&sp, speeeeed);
  if (errno != 0) {
    perror("cfsetispeed");
    return NULL;
  }
  log_standard("cfsetOspeed done.\n");

  struct stat ret_stat;
  errno = 0;
  if (fstat(fileno(serial), &ret_stat) != 0) {
    perror("fstat");
  }
  printf("st_rdev=%x\n", ret_stat.st_rdev);

  return serial;
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
          log_error("something went wrong ): 260-5=255 so overfloe QED\n");
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
    printf("must have one argument for serial path\n");
    return 1;
  }

  serialPath = argv[1];
  serialBaudNumber = atoi(argv[2]);

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
  FILE *serial = serial_initialize();
  if (serial == NULL) {
    log_error("opening serial failed\n");
    return 1;
  }

  pthread_t serial_thread;
  pthread_create(&serial_thread, NULL, serial_loop, (void *) serial);
  
  while (true) {
    // Update web server
    update_server(&manager, LOOP_TIMEOUT_MS);

    // Update command stack
    int write_len = update_command_stack(command_send_buffer);

    if (write_len > 0) {
      log_standard("writing %d bytes...\n", write_len);
      errno = 0;
      int retval = write(fileno(serial), command_send_buffer, write_len);
      if (retval != 0) {
        perror("command sending failed (retval)");
      }
      //fwrite(command_send_buffer, write_len, 1, serial);
      if (errno != 0) {
        perror("command sending failed");
      } else {
        log_standard("wrote %d bytes...\n", write_len);
      }
    }
  }
  return 0;
}
