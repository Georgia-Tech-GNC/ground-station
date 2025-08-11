#ifndef DATATYPES_H
#define DATATYPES_H

#include <cstdint>
#include <cstddef>

static_assert(sizeof(float) == 4);
static_assert(sizeof(double) == 8);

struct CommandData {
    uint32_t command_id, command_uuid;

    static constexpr size_t PAYLOAD_BYTE_SIZE = 8;
    
    /** Constructs a CommandData struct from a provided byte array
     * The size of this array must be exactly CommandData::PAYLOAD_BYTE_SIZE
     */
    CommandData(const std::byte[]);
    void store_to_buffer(std::byte[]);
};

struct RocketKinematicsData {
    uint16_t yaw, pitch;

    static constexpr size_t PAYLOAD_BYTE_SIZE = 4;
    
    /** Constructs a RocketKinematicsData struct from a provided byte array
     * The size of this array must be exactly RocketKinematicsData::PAYLOAD_BYTE_SIZE
     */
    RocketKinematicsData(const std::byte[]);
    void store_to_buffer(std::byte[]);
};

#endif