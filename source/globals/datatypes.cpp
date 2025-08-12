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


DataEntry::DataEntry(const DataEntry& other) noexcept(true): type(other.type) {
    switch (other.type) {
        case DataPacketType::ROCKET_KINEMATICS_DATA:
            data.rocket_kinematics_data = other.data.rocket_kinematics_data;
            break;
        case DataPacketType::ROCKET_SENSOR_DATA:
            data.rocket_sensor_data = other.data.rocket_sensor_data;
            break;
    }
}

template<> RocketKinematicsData DataCollection::load() { 
    return rocket_kinematics_data.load(std::memory_order_relaxed);
}

template<> RocketSensorData DataCollection::load() { 
    return rocket_sensor_data.load(std::memory_order_relaxed);
}

void DataCollection::store(const DataEntry& data) {
    switch (data.type) {
        case DataPacketType::ROCKET_KINEMATICS_DATA:
            rocket_kinematics_data.store(data.data.rocket_kinematics_data, std::memory_order_relaxed);
            break;
        case DataPacketType::ROCKET_SENSOR_DATA:
            rocket_sensor_data.store(data.data.rocket_sensor_data, std::memory_order_relaxed);
            break;
    }
}