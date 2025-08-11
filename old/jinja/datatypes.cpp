#include "datatypes.h"

static bool GENERATED_PREPARED_STATEMENTS = false;

RocketKinematicsData::RocketKinematicsData(std::array<std::byte, 16> raw_array) {
    std::memcpy(&x, raw_array.begin() + 0, 4);
    std::memcpy(&y, raw_array.begin() + 4, 4);
    std::memcpy(&z, raw_array.begin() + 8, 4);
    std::memcpy(&roll, raw_array.begin() + 12, 2);
    std::memcpy(&pitch, raw_array.begin() + 14, 2);
}

RocketKinematicsData::RocketKinematicsData(const RocketKinematicsData& other) {
    x = other.x;
    y = other.y;
    z = other.z;
    roll = other.roll;
    pitch = other.pitch;
}

void RocketKinematicsData::bind_write_statement() {
    sqlite3_bind_double(RocketKinematicsData::write_statement, 1, x);
    sqlite3_bind_double(RocketKinematicsData::write_statement, 2, y);
    sqlite3_bind_double(RocketKinematicsData::write_statement, 3, z);
    sqlite3_bind_int(RocketKinematicsData::write_statement, 4, roll);
    sqlite3_bind_int(RocketKinematicsData::write_statement, 5, pitch);
}

RocketSensorData::RocketSensorData(std::array<std::byte, 16> raw_array) {
    std::memcpy(&gps_x, raw_array.begin() + 0, 8);
    std::memcpy(&gps_y, raw_array.begin() + 8, 4);
    std::memcpy(&gps_z, raw_array.begin() + 12, 4);
}

RocketSensorData::RocketSensorData(const RocketSensorData& other) {
    gps_x = other.gps_x;
    gps_y = other.gps_y;
    gps_z = other.gps_z;
}

void RocketSensorData::bind_write_statement() {
    sqlite3_bind_int64(RocketSensorData::write_statement, 1, gps_x);
    sqlite3_bind_int(RocketSensorData::write_statement, 2, gps_y);
    sqlite3_bind_int(RocketSensorData::write_statement, 3, gps_z);
}


int initialize_data_packet_sql(sqlite3* db) {
    if (GENERATED_PREPARED_STATEMENTS) {
        return SQLITE_OK;
    }

    int error_code;
    
    error_code = sqlite3_prepare_v2(db, "INSERT INTO RocketKinematicsData VALUES (?, ?, ?, ?, ?)", 56, &RocketKinematicsData::write_statement, nullptr);
    if (error_code != SQLITE_OK) return error_code;

    error_code = sqlite3_prepare_v2(db, "INSERT INTO RocketSensorData VALUES (?, ?, ?)", 46, &RocketSensorData::write_statement, nullptr);
    if (error_code != SQLITE_OK) return error_code;


    GENERATED_PREPARED_STATEMENTS = true;
    return SQLITE_OK;
}

bool build_all_tables(sqlite3* db) {
    char* errMsg;
    sqlite3_exec(db, RocketKinematicsData::sql_table_gen_command, nullptr, nullptr, &errMsg);
    if (errMsg) {
        sqlite3_free(errMsg);
        return false;
    }
    sqlite3_exec(db, RocketSensorData::sql_table_gen_command, nullptr, nullptr, &errMsg);
    if (errMsg) {
        sqlite3_free(errMsg);
        return false;
    }
    
    return true;
}