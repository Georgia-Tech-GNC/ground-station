#include "packet_helpers.h"

#include <string.h>
#include <stdio.h>

#define PID_ACK_FLAG 0b0100'0000
#define PID_CRC8_ZERO_FLAG 0b1000'0000
#define PID_ID_MASK 0b0011'1111

/** Appliess COBS encoding to a buffer in-place 
 *  Buffers MUST be at most 255 bytes in length
 */
static size_t cobs_encode(uint8_t* bytes, const size_t length) {
    size_t last_zero = length + 1;
    
    for (int i = length - 1; i >= 0; i--) {
        if (bytes[i] == 0) {
            bytes[i] = last_zero;
            last_zero = i;
        }
    }

    return last_zero;
}

/** Appliess COBS decoding to a buffer in-place 
 * Buffers MUST be at most 255 bytes in length
*/
static void cobs_decode(uint8_t* bytes, const size_t length, size_t first_zero) {
    while (first_zero < length) {
        size_t new_first_zero = bytes[first_zero];
        bytes[first_zero] = 0;
        first_zero = new_first_zero;
    }
}

// Precompute all hashes for 0xe7
const uint8_t crctable[256] = {0, 231, 41, 206, 82, 181, 123, 156, 164, 67, 141, 106, 246, 17,
    223, 56, 175, 72, 134, 97, 253, 26, 212, 51, 11, 236, 34, 197, 89, 190, 112, 151, 185, 94, 
    144, 119, 235, 12, 194, 37, 29, 250, 52, 211, 79, 168, 102, 129, 22, 241, 63, 216, 68, 163, 
    109, 138, 178, 85, 155, 124, 224, 7, 201, 46, 149, 114, 188, 91, 199, 32, 238, 9, 49, 214, 24, 
    255, 99, 132, 74, 173, 58, 221, 19, 244, 104, 143, 65, 166, 158, 121, 183, 80, 204, 43, 229, 2, 
    44, 203, 5, 226, 126, 153, 87, 176, 136, 111, 161, 70, 218, 61, 243, 20, 131, 100, 170, 77, 209, 
    54, 248, 31, 39, 192, 14, 233, 117, 146, 92, 187, 205, 42, 228, 3, 159, 120, 182, 81, 105, 142, 64, 
    167, 59, 220, 18, 245, 98, 133, 75, 172, 48, 215, 25, 254, 198, 33, 239, 8, 148, 115, 189, 90, 116, 
    147, 93, 186, 38, 193, 15, 232, 208, 55, 249, 30, 130, 101, 171, 76, 219, 60, 242, 21, 137, 110, 160, 71, 
    127, 152, 86, 177, 45, 202, 4, 227, 88, 191, 113, 150, 10, 237, 35, 196, 252, 27, 213, 50, 174, 73, 135, 96, 
    247, 16, 222, 57, 165, 66, 140, 107, 83, 180, 122, 157, 1, 230, 40, 207, 225, 6, 200, 47, 179, 84, 154, 125, 69, 
    162, 108, 139, 23, 240, 62, 217, 78, 169, 103, 128, 28, 251, 53, 210, 234, 13, 195, 36, 184, 95, 145, 118
};

/** 
 * Returns the 8-bit CRC hash of this data
 * 
 * @param rawData   pointer to the data that the hash should be calculated for
 * @param dataSize  size of the data in bytes
 * 
 * @return the CRC8 hash of the data
 */
static uint8_t calculate_crc8_hash(const uint8_t* const bytes, const size_t length) {
    uint8_t crc = 0;
    for (size_t i = 0; i < length; i++) 
        crc = crctable[bytes[i] ^ crc];

    return crc;
}

void sign_packet(packet_metadata_t metadata, uint8_t* buffer) {
    buffer[0] = 0x00;
    buffer[1] = metadata.message_id;
    if (metadata.is_ack_packet) buffer[1] |= PID_ACK_FLAG;
    buffer[2] = metadata.counter;
    size_t checksum_index = 3;

    if (metadata.payload_size > 0) {
        buffer[2] = cobs_encode(buffer + 3, metadata.payload_size);
        checksum_index += metadata.payload_size;
    }

    buffer[checksum_index] = calculate_crc8_hash(buffer, checksum_index);
    if (buffer[checksum_index] == 0) {
        buffer[checksum_index] = 0xFF;
        buffer[1] |= 0x80;
    }    
}

uint8_t* unsign_packet(packet_metadata_t* metadata, uint8_t* buffer, size_t buffer_length) {
    if (buffer_length < 4 || buffer_length > 259) return NULL;
    
    uint8_t old_hash = 0;
    if (buffer[1] & PID_CRC8_ZERO_FLAG) { // Check for zero checksum flag
        buffer[1] &= ~PID_CRC8_ZERO_FLAG;
        old_hash = buffer[buffer_length - 1];
        buffer[buffer_length - 1] = 0x00;
    }

    if (calculate_crc8_hash(buffer, buffer_length)) {
        if (old_hash != 0) {
            // Restore modifications to buffer.
            buffer[1] |= PID_CRC8_ZERO_FLAG;
            buffer[buffer_length - 1] = old_hash;
        }
        return NULL; // Invalid checksum
    }

    metadata->message_id = buffer[1] & PID_ID_MASK;
    metadata->is_ack_packet = buffer[1] & PID_ACK_FLAG;
    metadata->counter = buffer[2];
    metadata->payload_size = buffer_length - 4;
    if (buffer_length > 4) {
        cobs_decode(buffer + 3, buffer_length - 4, buffer[2]);
        metadata->payload_size = buffer_length - 4;
    }

    return buffer + 3;
} 
