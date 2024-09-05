#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

const uint8_t generator = 0xe7;
uint8_t crctable[256];

struct TestData {
    int a;
    int b;
    bool c;
    float d;
} a;

/** 
 * This method precomputes crc values for all 256 possible bit values.  
 * This method must be called once before attempting to packetize anything
 * 
 * Calling this method multiple times has no effect.
 */
void populate_crc_lookup() {
    for (int dividend = 0; dividend < 256; dividend++)
    {
        uint8_t currByte = (uint8_t)dividend;
        /* calculate the CRC-8 value for current byte */
        for (uint8_t bit = 0; bit < 8; bit++)
        {
            if ((currByte & 0x80) != 0)
            {
                currByte <<= 1;
                currByte ^= generator;
            }
            else
            {
                currByte <<= 1;
            }
        }
        /* store CRC value in lookup table */
        crctable[dividend] = currByte;
    }
}


/** 
 * Applies the COBS algorithm to an input rawData.
 * This method removes all instances of the 0x00 byte by stuffing a linked list of bytes into the packet to represent the missing bytes. 
 * This last instance of the 0x00 byte will point to the index after the array
 * Note that the raw data may NOT be more than 254 bytes long. 
 * The encoded data will be exactly one byte longer than the raw data.
 * 
 * @param   rawData                 A pointer to the start of a uint8_t[] representing the data to encode
 * @param   encodedData             A pointer to the start of a uint8_t[]. This array will be overwritten with the encoded data
 * @param   dataSize                The length of the raw data (in bytes)
 * @param   encodedDataStartIndex   Represents the index at which this method should begin storing the encoded data in the encodedData array
 * @return                          The index of the final stuffed bit in the encodedData array
 */
int cobs_encode(uint8_t *rawData, uint8_t *encodedData, uint8_t dataSize, int encodedDataStartIndex) {
    int lastStuffedByteIndex = 0;
    encodedData[encodedDataStartIndex] = dataSize + 1;

    for (int i = 1; i <= dataSize; i++) {
        if (rawData[i - 1] == 0x00) {
            // Apply COBS algorithm and set index from previous COBS byte to point here
            encodedData[encodedDataStartIndex + lastStuffedByteIndex] = i - lastStuffedByteIndex;
            lastStuffedByteIndex = i;
            encodedData[encodedDataStartIndex + i] = dataSize + 1 - i;
        } else {
            encodedData[encodedDataStartIndex + i] = rawData[i - 1];
        }
    }

    return lastStuffedByteIndex + encodedDataStartIndex;
}

/** 
 * Takes any raw data, and converts into a packet (complete with byte stuffing and a checksum)
 * Packet format: 0x00 [messageID - 1 byte] [payload size - 1 byte] [COBS] [payload] [checksum(CRC-8) - 1 byte]
 * 
 * @param rawData           the raw payload data as an array of uint8_t
 * @param packetizedData    a pointer to a uint8_t array where the generated packet should be stored
 * @param dataSize          the length of the raw data in bytes
 * @param messageID         a one-byte message id that should be associated with this data (must not be 0)
 * @return                  The length of the packetized data in bytes
 */
int packetize(uint8_t *rawData, uint8_t *packetizedData, uint8_t dataSize, uint8_t messageID) {
    packetizedData[0] = 0x00;                               // Byte 1: 0x00
    packetizedData[1] = messageID;                          // Byte 2: message id
    packetizedData[2] = dataSize;                           // Byte 3: payload size
    printf("%x\n", packetizedData[2]);
    int cobs_overwrite_index = cobs_encode(rawData, packetizedData, dataSize, 3);  
                                                            // Byte 3 COBS byte         
                                                            // Bytes 4 - dataSize+3: payload

    // Calculate the CRC-8 checksum
    uint8_t crc = 0;
    for (int i = 0; i < dataSize + 4; i++)
    {
        // XOR-in next input byte and get current CRC value = remainder 
        crc = crctable[packetizedData[i] ^ crc];
    }

                                                            
    if (crc == 0x00) {                                      // Byte dataSize+4: checksum
        packetizedData[dataSize + 4] = 0xFF;
    } else {
        packetizedData[dataSize + 4] = crc;
        packetizedData[cobs_overwrite_index]++;     // cobs encoding automatically assumed that crc == 0, so we correct the assumption by
                                                    // incrementing the last jump by one
    }

    return dataSize + 5;
}

void print_bytes(uint8_t *data, size_t dataSize) {
    for (int i = 0; i < dataSize; i++) {
        printf("%x ", data[i]);
    }
}


uint8_t packet[260];

int main() {
    populate_crc_lookup();

    a.a = 123120;
    a.b = 4346479;
    a.c = true;
    a.d = 0.434256314f;

    printf("Original packet:\n");
    print_bytes((uint8_t*)&a, sizeof(a));

    int packet_size = packetize((uint8_t*)&a, packet, sizeof(a), 15);

    printf("\n\nEncoded packed:\n");
    print_bytes(packet, packet_size);
    
}