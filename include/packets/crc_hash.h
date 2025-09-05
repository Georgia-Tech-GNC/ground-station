#ifndef CRC_HASH
#define CRC_HASH

#include <cstdint>
#include <cstddef>

/** 
 * Returns the 8-bit CRC hash of this data
 * 
 * @param raw_data   pointer to the data that the hash should be calculated for
 * @param data_size  size of the data in bytes
 * 
 * @return the CRC8 hash of the data
 */
uint8_t calculate_crc8_hash(const uint8_t *raw_data, const size_t data_size);

/**
 * Returns true if the crc8 hash is valid for this data. It assumes that the 8-bit crc hash immediately follows the data
 * 
 * @param data   pointer to the data that the hash should be verified for (last byte should be the hash)
 * @param data_size  size of the data in bytes (including the hash byte)
 * 
 * Note that size includes the hash byte. In other words, if you want to verify the hash of n bytes, pass in 
 * an n+1 byte array where the first n bytes are the data and the n+1-th byte is the hash. 
 * data_size should also equal n+1.
 * 
 * @return true if the hash was valid, and false otherwise
 */
inline bool verify_crc8_hash(const uint8_t *data, const size_t data_size) {
    return 0 == calculate_crc8_hash(data, data_size);
}

#endif