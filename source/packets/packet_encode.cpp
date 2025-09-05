#include "packet_encode.h"
#include "crc_hash.h"

const size_t PACKET_START_IDX = 0;
const size_t MESSAGE_ID_IDX = 1;
const size_t COBS_IDX = 2;
const size_t PAYLOAD_START_IDX = 3;
#define CRC_IDX(payload_len) ((payload_len) + PAYLOAD_START_IDX)

const uint8_t COMMAND_MSG_ID = 

#include <cstring>

/**
 * Iterates through the inputted array and replaces all zeroes.
 * Note that the first element in this array will be the COBS byte and will always be overwritten.
 * 
 * @param data_to_encode    the array of data to encode, including the initial COBS byte (will be modified in place)
 * @param data_size         length of the array (including the first COBS byte)
 * 
 * It is guaranteed that the last replaced 0 will point to one after the end of the array
 * @return the index of the last modified byte.
 */
int cobs_encode(uint8_t *data_to_encode, const uint8_t data_size) {
    if (data_size == 0) return 0;
    int last_stuffed_byte = 0;
    data_to_encode[0] = data_size;

    for (int i = 1; i < data_size; i++) {
        if (data_to_encode[i] == 0x00) { // Add to internal linked list
            data_to_encode[last_stuffed_byte] = i - last_stuffed_byte;
            last_stuffed_byte = i;
            data_to_encode[i] = data_size - i;
        }
    }

    return last_stuffed_byte;
}

int generate_packet(const uint8_t *raw_data, const uint8_t data_size, uint8_t *packetized_data, const uint8_t message_id) {
    packetized_data[PACKET_START_IDX] = 0x00;                               // Byte 1: 0x00
    packetized_data[MESSAGE_ID_IDX] = message_id;                            // Byte 2: message id
    std::memcpy(packetized_data+PAYLOAD_START_IDX, raw_data, data_size);           // Copy payload starting at byte 4
    int cobs_overwrite_index = cobs_encode(packetized_data + COBS_IDX, data_size + 2);  // Apply COBS encoding

    // Calculate the CRC-8 checksum
    uint8_t crc = calculate_crc8_hash(packetized_data, data_size + 4);
                                                   
    if (crc == 0x00) {                                      // Byte data_size+5: checksum
        packetized_data[CRC_IDX(data_size)] = 1;
    } else {
        packetized_data[CRC_IDX(data_size)] = crc;
        packetized_data[cobs_overwrite_index + 2]++; // cobs encoding automatically assumed that crc == 0, so we correct the assumption by
                                                    // incrementing the last jump by one
    }

    return data_size + 4;
}

bool verify_packet(uint8_t *raw_data, const size_t packet_size) {
    if (packet_size < 4) return false;
    // First either de-stuff the crc_hash byte if needed or move back the last cobs pointer

    if (raw_data[PACKET_START_IDX] != 0) return false; // Leading byte was not 0.
    
    size_t next_cobs_byte = 2;
    while (next_cobs_byte < packet_size) {
        size_t increment = raw_data[next_cobs_byte];
        if (increment == 0) return false; // Avoid infinite loops
        else if (next_cobs_byte + increment == packet_size - 1) {
            raw_data[packet_size-1] = 0;
            break;
        } else if (next_cobs_byte + increment >= packet_size) {
            raw_data[next_cobs_byte]--;
        }

        next_cobs_byte += increment;
    } 

    return verify_crc8_hash(raw_data, packet_size);
}

/** Extracts the stored data from a packet and copies it to the extractedData location 
 * 
 * This method assumes that the packet was verified. Please use verify_packet first.
 * 
 * @param rawData   the packet to extract
 * @param dataSize  the size of the packet (including the 5 overhead bytes)
 * @param extractedData the buffer where the extracted data should be stored
 * 
 * This method will store exactly dataSize-5 bytes in the extractedData buffer
 * 
 * @return      the message id, or 0 if the message was corrupted
 */
uint8_t extract_packet(uint8_t *rawData, const size_t dataSize, uint8_t *extractedData) {
    if (dataSize < 5) return 0;

    uint8_t message_id = rawData[1];

    if (message_id == 0) return 0;

    // First de-stuff the packet
    size_t next_cobs_byte = 2;
    while (next_cobs_byte < dataSize - 1) {
        uint8_t increment = rawData[next_cobs_byte];
        if (increment == 0) return 0;
        rawData[next_cobs_byte] = 0;

        next_cobs_byte += increment;
    }  

    memcpy(extractedData, rawData + 4, dataSize - 5);

    return message_id;
}

size_t get_msg_size(const uint8_t message_id) {
    if (message_id == 
}

int process_incoming_byte(const uint8_t inputted_byte, uint8_t *buffer, const int current_buffer_size) {
    if (inputted_byte == 0) { // New message started
        buffer[0] = 0;
        return 1;
    }

    if (current_buffer_size == 0)  // No currently active message
        return 0;
    
    buffer[current_buffer_size] = inputted_byte;

    if (current_buffer_size >= 2) { // We know the size
        int msg_size = (buffer[2] == 1)?5:(buffer[3] + 5);
        if (current_buffer_size + 1 >= msg_size) {
            return -msg_size;
        }
    }
    
    return current_buffer_size + 1;
}