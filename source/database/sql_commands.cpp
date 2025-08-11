#include "sql_commands.h"
    
/** Allocates an SQL database object and creates all of the required tables. 
    * This method should be called once accross all threads at the start of the program
    */
SQLDatabase::SQLDatabase(const std::string& name) {
    sqlite3_open(name.c_str(), &db);

    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS RocketKinematicsData (timestamp INTEGER PRIMARY KEY, x REAL, y REAL, z REAL, roll INTEGER, pitch INTEGER)", nullptr, nullptr, nullptr);
    sqlite3_prepare_v2(db, "INSERT INTO RocketKinematicsData VALUES (?, ?, ?, ?, ?, ?)", 59, &rocket_kinematics_data_insert_cmd, nullptr);

    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS RocketSensorData (timestamp INTEGER PRIMARY KEY, gps_x INTEGER, gps_y INTEGER, gps_z INTEGER)", nullptr, nullptr, nullptr);
    sqlite3_prepare_v2(db, "INSERT INTO RocketSensorData VALUES (?, ?, ?, ?)", 49, &rocket_sensor_data_insert_cmd, nullptr);
}

/** Deallocates the SQL database object 
    * This method should be called once at the end of the program
*/
SQLDatabase::~SQLDatabase() {
    // Commit uncommited changes
    if (is_transaction_open) {
        close_transaction();
    }

    sqlite3_close(db);
    
    sqlite3_finalize(rocket_kinematics_data_insert_cmd);
    sqlite3_finalize(rocket_sensor_data_insert_cmd);
   
}

void SQLDatabase::close_transaction() {
    if (!is_transaction_open) return; 

    sqlite3_exec(db, "END TRANSACTION", nullptr, nullptr, nullptr);
    is_transaction_open = false;
}

void SQLDatabase::begin_transaction() {
    if (is_transaction_open) return; 

    sqlite3_exec(db, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);
    is_transaction_open = true;
}

void SQLDatabase::insert_data(const DataEntry& data, timestamp_t timestamp) {
    sqlite3_stmt* target_stmt;

    switch (data.type) {
        case DataPacketType::ROCKET_KINEMATICS_DATA:
            target_stmt = rocket_kinematics_data_insert_cmd;

            sqlite3_bind_int(target_stmt, 1, timestamp);
            sqlite3_bind_double(target_stmt, 2, data.data.rocket_kinematics_data.x);
            sqlite3_bind_double(target_stmt, 3, data.data.rocket_kinematics_data.y);
            sqlite3_bind_double(target_stmt, 4, data.data.rocket_kinematics_data.z);
            sqlite3_bind_int(target_stmt, 5, data.data.rocket_kinematics_data.roll);
            sqlite3_bind_int(target_stmt, 6, data.data.rocket_kinematics_data.pitch);

            break;
        case DataPacketType::ROCKET_SENSOR_DATA:
            target_stmt = rocket_sensor_data_insert_cmd;

            sqlite3_bind_int(target_stmt, 1, timestamp);
            sqlite3_bind_int64(target_stmt, 2, data.data.rocket_sensor_data.gps_x);
            sqlite3_bind_int(target_stmt, 3, data.data.rocket_sensor_data.gps_y);
            sqlite3_bind_int(target_stmt, 4, data.data.rocket_sensor_data.gps_z);

            break;
    }

    sqlite3_step(target_stmt);
    sqlite3_reset(target_stmt);
}