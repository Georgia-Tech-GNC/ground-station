#ifndef DATATYPES_H
#define DATATYPES_H

#include <cstdint>
#include <cstddef>
#include <array>

#include "sqlite3.h"

enum class DataPacketType {
    ROCKET_KINEMATICS_DATA,
    ROCKET_SENSOR_DATA,
};

struct RocketKinematicsData {
    float x, y, z;
    uint16_t roll, pitch;

    RocketKinematicsData(const std::byte (&raw_array)[16]); 
    RocketKinematicsData(float _x, float _y, float _z, uint16_t _roll, uint16_t _pitch):
        x(_x), y(_y), z(_z), roll(_roll), pitch(_pitch) { }
    RocketKinematicsData(const RocketKinematicsData& other):
        x(other.x), y(other.y), z(other.z), roll(other.roll), pitch(other.pitch) { }
};

struct RocketSensorData {
    int64_t gps_x;
    int32_t gps_y, gps_z;

    RocketSensorData(const std::byte (&raw_array)[16]); 
    RocketSensorData(int64_t _gps_x, int32_t _gps_y, int32_t _gps_z):
        gps_x(_gps_x), gps_y(_gps_y), gps_z(_gps_z) { }
    RocketSensorData(const RocketSensorData& other):
        gps_x(other.gps_x), gps_y(other.gps_y), gps_z(other.gps_z) { }
};

struct DataEntry {
    DataPacketType type;
    union DataEntryUnionType {
        RocketKinematicsData rocket_kinematics_data;
        RocketSensorData rocket_sensor_data;

        /** Default (empty) constructor */
        DataEntryUnionType() { }
        DataEntryUnionType(RocketKinematicsData _rocket_kinematics_data): rocket_kinematics_data(_rocket_kinematics_data) { }
        DataEntryUnionType(RocketSensorData _rocket_sensor_data): rocket_sensor_data(_rocket_sensor_data) { }
    } data;

    /** Default (empty) constructor */
    DataEntry() { }

    /** Copy constructor */
    DataEntry(const DataEntry& other) noexcept(true);

    /** Constructor for DataEntry that initializes to hold a RocketKinematicsData */
    DataEntry(RocketKinematicsData _rocket_kinematics_data): 
        type(DataPacketType::ROCKET_KINEMATICS_DATA), data(_rocket_kinematics_data) { }

    /** Constructor for DataEntry that initializes to hold a RocketSensorData */
    DataEntry(RocketSensorData _rocket_sensor_data): 
        type(DataPacketType::ROCKET_SENSOR_DATA), data(_rocket_sensor_data) { }
};

#endif