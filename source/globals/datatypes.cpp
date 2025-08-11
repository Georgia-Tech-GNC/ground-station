#include "datatypes.h"

RocketKinematicsData::RocketKinematicsData(const std::byte (&raw_array)[16]) {
    std::memcpy(&x, &raw_array[0], 4);
    std::memcpy(&y, &raw_array[4], 4);
    std::memcpy(&z, &raw_array[8], 4);
    std::memcpy(&roll, &raw_array[12], 2);
    std::memcpy(&pitch, &raw_array[14], 2);
}

RocketSensorData::RocketSensorData(const std::byte (&raw_array)[16]) {
    std::memcpy(&gps_x, &raw_array[0], 8);
    std::memcpy(&gps_y, &raw_array[8], 4);
    std::memcpy(&gps_z, &raw_array[12], 4);
}

