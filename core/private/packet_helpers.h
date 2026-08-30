#ifndef PACKET_HELPERS_H
#define PACKET_HELPERS_H

#include <stddef.h>
#include <stdint.h>

typedef struct PacketMetadata {
    uint8_t message_id;
    bool is_ack_packet;
    uint8_t counter;
    uint8_t payload_size;
} packet_metadata_t;

/** "Signs" a packet by modifying the desired encoding in-place as specified by the metadata.
 *
 * Any desired payload should be pre-filled in buffer starting at the fourth element (index 3)
 *
 * This method can be "undone" by unsign_payload, which yields the provided metadata and returns 
 * the payload portion ONLY of the buffer to it's original form.
 * 
 * The length of the buffer must be at least the payload size + 4. The payload size must reflect
 * the size specified in the provided metadata.
 */
void sign_packet(packet_metadata_t metadata, uint8_t* buffer);

/** "Unsigns" a packet, effectively undoing sign_packet 
 * This method restores the original metadata of the packet, along with the packet payload, which
 * can be located starting at the fourth element of the buffer (index 3). For convenience, a pointer
 * to this value is also returned when the method succeeds.
 * 
 * Note that the counter parameter in the metadata will be set to 0 (unused) when a payload is present.
 *
 * @return a pointer to the payload, or NULL if the packet was invalid (in which case buffer will not 
 * be modified)
 */
uint8_t* unsign_packet(packet_metadata_t* metadata, uint8_t* buffer, size_t buffer_length);

#endif