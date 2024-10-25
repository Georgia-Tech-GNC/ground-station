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
  size_t written = fwrite(packetizedData, sizeof(uint8_t), packetizedLength, serial);
  if (errno != 0) {
    perror("fwrite to radio");
  }
  printf("written: %d\n", written);
}
