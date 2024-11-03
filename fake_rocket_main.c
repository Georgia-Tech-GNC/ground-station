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

void loop(FILE *serial);

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("%s serial_path srand\n", argv[0]);
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

  while (1) {
    loop(serial);
    exit(0);
    sleep(3);
  }
}

void loop(FILE *serial) {
  struct RocketState native = {
    .rocket_state = 1,
  };
  uint8_t *msg = malloc(sizeof(uint8_t) * ROCKETSTATE_SIZE);
  uint8_t *packetizedData = malloc(sizeof(uint8_t) * (ROCKETSTATE_SIZE+5));
  RocketState_encode(&native, msg);
  int packetizedLength = generate_packet(msg, ROCKETSTATE_SIZE, packetizedData, ROCKETSTATE_MSG_ID);
  if (packetizedLength != ROCKETSTATE_SIZE+5) {
    printf("packetizedLength != ROCKETSTATE_SIZE+5: %d != %d\n", packetizedLength, ROCKETSTATE_SIZE+5);
    exit(1);
  }
  printf("packetizedLength=%d\n",packetizedLength);

  // uint8_t *packetizedData = malloc(sizeof(uint8_t) * (4+5));
  // int packetizedLength = generate_packet("abc", 4, packetizedData, 5);
  // printf("packetizedLength=%d\n", packetizedLength);

  //fprintf(serial, "hello world\n");

  for (int i = 0; i < packetizedLength; i ++) {
    printf("%02x\n", packetizedData[i]);
    fprintf(serial, "%c", packetizedData[i]);
  }
  printf("\n");
  fflush(serial);

  for (int i = 0; i < 50; i++) {
    for (int j = 0; j < 200; j++) garbage[j] = rand();
    int l = generate_packet(garbage, 200, packetized, 12);

    for (int j=0; j < l; j++) {
      printf("%02x\n", packetized[j]);
      fprintf(serial, "%c", packetized[j]);
    }
    fflush(serial);
  }

  fflush(serial);
  // fprintf(serial, "\r\n");
  // size_t written = fwrite(packetizedData, sizeof(uint8_t), packetizedLength, serial);
  // // size_t written = fwrite(packetizedData, sizeof(uint8_t), packetizedLength, stderr);
  // if (errno != 0) {
  //   perror("fwrite to radio");
  // }
  // printf("written: %d\n", written);
}
