#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "protocol.h"
#include "packet_encode.h"

char *serialPath;

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
    sleep(3);
  }
}

void loop(const FILE *serial) {
  struct RocketStateVector native = {
    .velocity_x = (float) rand() / RAND_MAX,
  };
  uint8_t *msg = malloc(sizeof(uint8_t) * ROCKETSTATEVECTOR_SIZE);
  uint8_t *packetizedData = malloc(sizeof(uint8_t) * (ROCKETSTATEVECTOR_SIZE+5));
  RocketStateVector_encode(&native, msg);
  int packetizedLength = generate_packet(msg, ROCKETSTATEVECTOR_SIZE, packetizedData, ROCKETSTATEVECTOR_MSG_ID);
  if (packetizedLength != ROCKETSTATEVECTOR_SIZE+5) {
    printf("packetizedLength != ROCKETSTATEVECTOR_SIZE+5: %d != %d\n", packetizedLength, ROCKETSTATEVECTOR_SIZE+5);
    exit(1);
  }
  printf("packetizedLength=%d\n",packetizedLength);

  // uint8_t *packetizedData = malloc(sizeof(uint8_t) * (4+5));
  // int packetizedLength = generate_packet("abc", 4, packetizedData, 5);
  // printf("packetizedLength=%d\n", packetizedLength);

  //fprintf(serial, "hello world\n");

  // printf("data[0]=%02x\n", packetizedData[0]);
  // printf("data[1]=%02x\n", packetizedData[1]);
  // printf("data[2]=%02x\n", packetizedData[2]);
  // printf("data[3]=%02x\n", packetizedData[3]);
  // printf("data[4]=%02x\n", packetizedData[4]);
  // printf("data[5]=%02x\n", packetizedData[5]);
  // printf("data[6]=%02x\n", packetizedData[6]);
  // printf("data[7]=%02x\n", packetizedData[7]);
  // printf("data[8]=%02x\n", packetizedData[8]);

  for (int i = 0; i < packetizedLength; i ++) {
    fprintf(serial, "%c", packetizedData[i]);
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
