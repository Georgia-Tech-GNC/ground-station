#ifndef GLOBAL_H
#define GLOBAL_H

#include <cstdint>
#include <atomic>

/***************** DEFINE STORED DATA TYPES *****************/
struct CommandData {
    uint_fast32_t command_id, command_uuid;

    bool operator=(const CommandData& other) { return (command_id == other.command_id) && (command_uuid == other.command_uuid); }
};

struct LogData {
    uint_fast32_t log_type, log_count_id;

    bool operator=(const LogData& other) { return (log_type == other.log_type) && (log_count_id == other.log_count_id); }
};

struct RocketKinematicsData {
    uint_fast16_t yaw, pitch;

    bool operator=(const RocketKinematicsData& other) { return (yaw == other.yaw) && (pitch == other.pitch); }
};

struct RocketSensorData {
    float heading_x, heading_y;

    bool operator=(const RocketSensorData& other) { return (heading_x == other.heading_x) && (heading_y == other.heading_y); }
};


/***************** DEFINE THE CONTAINER TYPES *****************/
struct DataContainer {
    CommandData command_data;
    LogData log_data;
    RocketKinematicsData rocket_kinematics_data;
    RocketSensorData rocket_sensor_data;

    template<typename T> inline T& get();
};
template<> inline CommandData&             DataContainer::get<>() { return command_data; }
template<> inline LogData&                 DataContainer::get<>() { return log_data; }
template<> inline RocketKinematicsData&    DataContainer::get<>() { return rocket_kinematics_data; }
template<> inline RocketSensorData&        DataContainer::get<>() { return rocket_sensor_data; }


union DataContainerUnion {
    CommandData command_data;
    LogData log_data;
    RocketKinematicsData rocket_kinematics_data;
    RocketSensorData rocket_sensor_data;

    template<typename T> inline T& get();
};
template<> inline CommandData&             DataContainerUnion::get<>() { return command_data; }
template<> inline LogData&                 DataContainerUnion::get<>() { return log_data; }
template<> inline RocketKinematicsData&    DataContainerUnion::get<>() { return rocket_kinematics_data; }
template<> inline RocketSensorData&        DataContainerUnion::get<>() { return rocket_sensor_data; }

/***************** DEFINE THE POINTER STORAGE TYPE *****************/
struct ContainerDirectory {
    std::atomic<CommandData*> command_data_ptr;
    std::atomic<LogData*> log_data_ptr;
    std::atomic<RocketKinematicsData*> rocket_kinematics_data_ptr;
    std::atomic<RocketSensorData*> rocket_sensor_data_ptr;

    template<typename T> std::atomic<T*>& get();


    ContainerDirectory(DataContainer& initial) :
        command_data_ptr(&initial.command_data),
        log_data_ptr(&initial.log_data),
        rocket_kinematics_data_ptr(&initial.rocket_kinematics_data),
        rocket_sensor_data_ptr(&initial.rocket_sensor_data) 
    { }
};
template<> std::atomic<CommandData*>& ContainerDirectory::get<>() { return command_data_ptr; }
template<> std::atomic<LogData*>& ContainerDirectory::get<>() { return log_data_ptr; }
template<> std::atomic<RocketKinematicsData*>& ContainerDirectory::get<>() { return rocket_kinematics_data_ptr; }
template<> std::atomic<RocketSensorData*>& ContainerDirectory::get<>() { return rocket_sensor_data_ptr; }


/** Reads data from the queue. This method makes no guarantee about the freshness of the data. 
 * The only guarantee made is that the data will always be valid, if not neccessarily the most recent.
 * 
 * In general, data should be at least as recent as the moment this method is called. 
 * Further guarantees are left to the ordering of atomic reads and writes by the scheduler.
 */
template<typename T> void read_queue(T& read_target);

/** Moves the tail pointer in the queue while ensuring that no stale readers are active during that time.
 * This function does NOT guarantee that all readers are inactive. It only guarantees that readers that began their read
 * before this method was called will finish their reads before the tail pointer is moved.
 */
void delete_queue(uint_fast32_t new_tail_value);

/** Increments the head pointer (potentially blocking if the queue if full) and 
 * stores the provided data as the new most recent data
 */
template<typename T> void write_queue(const T& data);

#endif