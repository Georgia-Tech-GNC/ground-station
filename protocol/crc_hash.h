
#ifndef CRC_HASH
#define CRC_HASH

#include <stdint.h>
#include <stddef.h>


/** 
 * Returns the 8-bit CRC hash of this data
 */
uint8_t calculate_crc8_hash(const uint8_t *rawData, const size_t dataSize);

/**
 * Returns true if the crc8 hash is valid for this data. It assumes that the 8-bit crc hash immediately follows the data
 */
bool verify_crc8_hash(const uint8_t *data, const size_t dataSize);

#endif