
#include "datatypes.h"

#include <cstdint>
#include <cstddef>
#include <cstring>

CommandData::CommandData(const std::byte buffer[]) {
    command_id = *reinterpret_cast<const uint32_t*>(buffer),
    command_uuid = *reinterpret_cast<const uint32_t*>(buffer + 4);
}

void CommandData::store_to_buffer(std::byte buffer[]) {
    std::memcpy(buffer, reinterpret_cast<const std::byte*>(&command_id), 4);
    std::memcpy(buffer + 4, reinterpret_cast<const std::byte*>(&command_uuid), 4);
}