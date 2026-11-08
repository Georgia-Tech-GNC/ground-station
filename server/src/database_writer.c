#include <stdlib.h>
#include <string.h>

#include "sqlite3.h"
#include "packet_types.h"
#include "ring_buffer.h"

// Declare SQLite prepared statements to write different types of packet data
static sqlite3_stmt *adis_data_statement;
static sqlite3_stmt *lsm_data_statement;
static sqlite3_stmt *barometer_data_statement;
static sqlite3_stmt *controls_statement;
static sqlite3_stmt *flight_computer_statement;

// Declare SQLite database object
static sqlite3* db_handle;

// Track most recent sqlite error message string, if applicable
static char* sql_error_msg;

// Ring buffer used to store pending writes
ring_buffer_t write_buffer;

bool initialize_db_writer(const char* db_filepath) {
    // Create write buffer
    if (!initialize_ring_buffer(&write_buffer, 4096)) { return false; }

    // Create SQLite database handle
    if (sqlite3_open(db_filepath, &db_handle) != SQLITE_OK) {
        sqlite3_close(db_handle);
        return false;
    }

    // Create all tables and initialize write statements
    if (
        sqlite3_exec(db_handle,
            "CREATE TABLE ADISSensorData ("
            "time INTEGER PRIMARY KEY, "
            "omega_x REAL, omega_y REAL, omega_z REAL, "
            "accel_x REAL, accel_y REAL, accel_z REAL);",
        nullptr, nullptr, &sql_error_msg ) != SQLITE_OK ||
        sqlite3_prepare_v2(db_handle,
            "INSERT INTO ADISSensorData VALUES (?, ?, ?, ?, ?, ?, ?);",
        56, &adis_data_statement, nullptr) != SQLITE_OK
    ) { return false; }

    if (
        sqlite3_exec(db_handle,
            "CREATE TABLE LSMSensorData ("
            "time INTEGER PRIMARY KEY, "
            "omega_x REAL, omega_y REAL, omega_z REAL, "
            "accel_x REAL, accel_y REAL, accel_z REAL);",
        nullptr, nullptr, &sql_error_msg ) != SQLITE_OK ||
        sqlite3_prepare_v2(db_handle,
            "INSERT INTO LSMSensorData VALUES (?, ?, ?, ?, ?, ?, ?);",
        56, &lsm_data_statement, nullptr) != SQLITE_OK
    ) { return false; }

    if (
        sqlite3_exec(db_handle,
            "CREATE TABLE BarometerSensorData (time INTEGER PRIMARY KEY, pressure REAL);",
        nullptr, nullptr, &sql_error_msg ) != SQLITE_OK ||
        sqlite3_prepare_v2(db_handle,
            "INSERT INTO BarometerSensorData VALUES (?, ?);",
        47, &barometer_data_statement, nullptr) != SQLITE_OK
    ) { return false; }

    if (
        sqlite3_exec(db_handle,
            "CREATE TABLE ControlsData ("
            "time INTEGER PRIMARY KEY, state TEXT, "
            "servo1_command REAL, servo2_command REAL, "
            "attitude_w REAL, attitude_x REAL, attitude_y REAL, attitude_z REAL, "
            "velocity_x REAL, velocity_y REAL, velocity_z REAL, "
            "altitude REAL);",
        nullptr, nullptr, &sql_error_msg ) != SQLITE_OK ||
        sqlite3_prepare_v2(db_handle,
            "INSERT INTO ControlsData VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);",
        70, &controls_statement, nullptr) != SQLITE_OK
    ) { return false; }

    if (
        sqlite3_exec(db_handle,
            "CREATE TABLE FlightComputerStatus ("
            "time INTEGER PRIMARY KEY, "
            "pyro_inhibit_status BOOLEAN, pyro1_command BOOLEAN, pyro1_status BOOLEAN, pyro2_command BOOLEAN, pyro2_status BOOLEAN, "
            "board_voltage REAL, board_current REAL);",
        nullptr, nullptr, &sql_error_msg ) != SQLITE_OK ||
        sqlite3_prepare_v2(db_handle,
            "INSERT INTO FlightComputerStatus VALUES (?, ?, ?, ?, ?, ?, ?, ?);",
        66, &flight_computer_statement, nullptr) != SQLITE_OK
    ) { return false; }

    return true;
}

static void commit_statement(sqlite3_stmt* stmt) {
    sqlite3_step(stmt);
    sqlite3_clear_bindings(stmt);
    sqlite3_reset(stmt);
}

static void write_adis_data_packet(const adis_data_packet_t* data) {
    if (
        sqlite3_bind_int64(adis_data_statement, 1, data->timestamp) != SQLITE_OK ||

        sqlite3_bind_double(adis_data_statement, 2, data->omega_x) != SQLITE_OK ||
        sqlite3_bind_double(adis_data_statement, 3, data->omega_y) != SQLITE_OK ||
        sqlite3_bind_double(adis_data_statement, 4, data->omega_z) != SQLITE_OK ||

        sqlite3_bind_double(adis_data_statement, 5, data->accel_x) != SQLITE_OK ||
        sqlite3_bind_double(adis_data_statement, 6, data->accel_y) != SQLITE_OK ||
        sqlite3_bind_double(adis_data_statement, 7, data->accel_z) != SQLITE_OK
    ) {

    } else { commit_statement(adis_data_statement); }
}

static void write_lsm_data_packet(const lsm_data_packet_t* data) {
    if (
        sqlite3_bind_int64(lsm_data_statement, 1, data->timestamp) != SQLITE_OK ||

        sqlite3_bind_double(lsm_data_statement, 2, data->omega_x) != SQLITE_OK ||
        sqlite3_bind_double(lsm_data_statement, 3, data->omega_y) != SQLITE_OK ||
        sqlite3_bind_double(lsm_data_statement, 4, data->omega_z) != SQLITE_OK ||

        sqlite3_bind_double(lsm_data_statement, 5, data->accel_x) != SQLITE_OK ||
        sqlite3_bind_double(lsm_data_statement, 6, data->accel_y) != SQLITE_OK ||
        sqlite3_bind_double(lsm_data_statement, 7, data->accel_z) != SQLITE_OK
    ) {

    } else { commit_statement(adis_data_statement); }
}

static void write_barometer_data_packet(const barometer_data_packet_t* data) {
    if (
        sqlite3_bind_int64(barometer_data_statement, 1, data->timestamp) != SQLITE_OK ||
        sqlite3_bind_double(barometer_data_statement, 2, data->pressure) != SQLITE_OK
    ) {

    } else { commit_statement(barometer_data_statement); }
}

static void write_controls_data_packet(const controls_data_packet_t* data) {
    if (
        sqlite3_bind_int64(controls_statement, 1, data->timestamp) != SQLITE_OK ||

        sqlite3_bind_text(controls_statement, 2, data->state, -1, SQLITE_STATIC) != SQLITE_OK ||
        sqlite3_bind_double(controls_statement, 3, data->servo1_command) != SQLITE_OK ||
        sqlite3_bind_double(controls_statement, 4, data->servo2_command) != SQLITE_OK ||

        sqlite3_bind_double(controls_statement, 5, data->attitude_w) != SQLITE_OK ||
        sqlite3_bind_double(controls_statement, 6, data->attitude_x) != SQLITE_OK ||
        sqlite3_bind_double(controls_statement, 7, data->attitude_y) != SQLITE_OK ||
        sqlite3_bind_double(controls_statement, 8, data->attitude_z) != SQLITE_OK ||

        sqlite3_bind_double(controls_statement, 9, data->velocity_x) != SQLITE_OK ||
        sqlite3_bind_double(controls_statement, 10, data->velocity_y) != SQLITE_OK ||
        sqlite3_bind_double(controls_statement, 11, data->velocity_z) != SQLITE_OK ||

        sqlite3_bind_double(controls_statement, 12, data->altitude) != SQLITE_OK
    ) {

    } else { commit_statement(controls_statement); }
}

static void write_flight_computer_data_packet(const flight_computer_data_packet_t* data) {
    if (
        sqlite3_bind_int64(flight_computer_statement, 1, data->timestamp) != SQLITE_OK ||
        sqlite3_bind_int(flight_computer_statement, 2, data->pyro_inhibit_status) != SQLITE_OK ||
        sqlite3_bind_int(flight_computer_statement, 3, data->pyro1_command) != SQLITE_OK ||
        sqlite3_bind_int(flight_computer_statement, 4, data->pyro1_status) != SQLITE_OK ||
        sqlite3_bind_int(flight_computer_statement, 5, data->pyro2_command) != SQLITE_OK ||
        sqlite3_bind_int(flight_computer_statement, 6, data->pyro2_status) != SQLITE_OK ||

        sqlite3_bind_double(flight_computer_statement, 7, data->board_voltage) != SQLITE_OK ||
        sqlite3_bind_double(flight_computer_statement, 8, data->board_current) != SQLITE_OK
    ) {

    } else { commit_statement(flight_computer_statement); }
}

void db_add_data(packet_type_t packet_type, const void* data) {
    size_t struct_size = get_packet_struct_size(packet_type);
    void* data_buffer = malloc(struct_size);
    memcpy(data_buffer, data, struct_size);
    push_to_buffer(&write_buffer, packet_type, data_buffer);
}