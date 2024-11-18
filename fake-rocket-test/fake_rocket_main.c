#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>

#include "protocol.h"
#include "packet_encode.h"

char *serialPath;

uint8_t garbage[200];
uint8_t packetized[215];

void loop(FILE *serial, int time);

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("%s serial_path\n", argv[0]);
    return 1;
  }

  serialPath = argv[1];
  srand(atoi(argv[1]));

  printf("opening %s...\n", serialPath);
  FILE *serial = fopen(serialPath, "wb+");
  if (serial == NULL) {
    perror("open serial file");
    return 1;
  }

  for (int i = 0; i < 10000; i++) {
    loop(serial, i);
    sleep(1);
  }
}

void send_packet(uint8_t msg_id, uint8_t *payload, size_t payload_size, FILE *serial) {
  uint8_t *packetized_data = malloc(payload_size + 5);
  int packetizedLength = generate_packet(payload, payload_size, packetized_data, msg_id);
  if (packetizedLength != payload_size+5) {
    printf("packetizedLength != payload_size+5: %d != %d\n", packetizedLength, payload_size+5);
    exit(1);
  }
  for (int i = 0; i < packetizedLength; i ++) {
    printf("%02x\n", packetized_data[i]);
    fprintf(serial, "%c", packetized_data[i]);
  }
  printf("packetizedLength=%d\n",packetizedLength);
  fflush(serial);
}

uint8_t packet_buffer[500];

void loop(FILE *serial, int time) {
  struct RocketState rs = {
    .rocket_state = 1,
    .firing_channel_1 = 2,
    .firing_channel_2 = time % 2,
    .firing_channel_3 = 1,
    .timestamp=time
  };
  
  RocketState_encode(&rs, packet_buffer);
  send_packet(ROCKETSTATE_MSG_ID, packet_buffer, ROCKETSTATE_SIZE, serial);

  struct RocketStateVector rsv = {
    .velocity_x = time * 3,
    .velocity_y = time*time/16.853,
    .velocity_z = time*.23,
    .timestamp=time
  };
  
  RocketStateVector_encode(&rsv, packet_buffer);
  send_packet(ROCKETSTATEVECTOR_MSG_ID, packet_buffer, ROCKETSTATEVECTOR_SIZE, serial);

  struct RocketAnalogFeedbackData afd = {
    .current_fb_33 = time * 3,
    .pyro_0_cont = time*time/16.853,
    .pyro_channel_deploy = 1 | (1<<2),
    .timestamp=time
  };
  
  RocketAnalogFeedbackData_encode(&afd, packet_buffer);
  send_packet(ROCKETANALOGFEEDBACKDATA_MSG_ID, packet_buffer, ROCKETANALOGFEEDBACKDATA_SIZE, serial);

  /** YOU CAN ADD MORE DATA TO SEND HERE! */
  // Send some useless data for testing
  for (int i = 0; i < 10; i ++) {
    fprintf(serial, "%c", i);
  }
}
