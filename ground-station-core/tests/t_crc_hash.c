#include "tst.h"
#include "crc_hash.h"

const int NUM_FUZZY_ATTEMPTS_CRC_HASH = 5000;


tstsuite("CRC Hashing") {
  int sizes[] = {0, 13, 20, 23, 40, 53, 255, 260, 257}; 

  for (int size_i = 0; size_i < 8; size_i++) {
    tstcase("Fuzzy testing CRC of array of size %d bytes", sizes[size_i]) {
      uint8_t *buffer = malloc(sizeof(uint8_t)*(sizes[size_i] + 1));
      for (int attempt = 0; attempt < NUM_FUZZY_ATTEMPTS_CRC_HASH; attempt++) {

        for (int i = 0; i < sizes[size_i]; i++) {
          buffer[i] = rand();
        }

        buffer[sizes[size_i]] = calculate_crc8_hash(buffer, sizes[size_i]);

        tstassert(verify_crc8_hash(buffer, sizes[size_i] + 1));
      }
      free(buffer);
    }
  }
}