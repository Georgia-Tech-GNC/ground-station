#ifndef DATATYPES_H
#define DATATYPES_H

#include <cstdint>
#include <cstddef>
#include <array>

#include "sqlite3.h"

struct RocketKinematicsData {
    float x, y, z;
    uint16_t roll, pitch;

    RocketKinematicsData(std::array<std::byte, 16>); 
    RocketKinematicsData(const RocketKinematicsData&); 

    static constexpr char* sql_table_gen_command = "CREATE TABLE IF NOT EXISTS RocketKinematicsData (timestamp INTEGER PRIMARY KEY, x REAL, y REAL, z REAL, roll INTEGER, pitch INTEGER)";
    static thread_local sqlite3_stmt* write_statement;
    void bind_write_statement();
};

struct RocketSensorData {
    int64_t gps_x;
    int32_t gps_y, gps_z;

    RocketSensorData(std::array<std::byte, 16>); 
    RocketSensorData(const RocketSensorData&); 

    static constexpr char* sql_table_gen_command = "CREATE TABLE IF NOT EXISTS RocketSensorData (timestamp INTEGER PRIMARY KEY, gps_x INTEGER, gps_y INTEGER, gps_z INTEGER)";
    static thread_local sqlite3_stmt* write_statement;
    void bind_write_statement();
};


int initialize_data_packet_sql(sqlite3* db);

bool build_all_tables(sqlite3* db);

#endif