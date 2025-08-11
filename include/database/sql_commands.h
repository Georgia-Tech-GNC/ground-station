#ifndef SQL_COMMANDS
#define SQL_COMMANDS

#include "sqlite3.h"
#include "datatypes.h"
#include "config.h"

#include <string>

class SQLDatabase {

private:
    sqlite3_stmt *rocket_kinematics_data_insert_cmd, *rocket_sensor_data_insert_cmd;
    sqlite3* db;

    bool is_transaction_open;

     /** Starts a SQL transaction */
    void begin_transaction();

    /** Ends an SQL transaction and commits all changes */
    void close_transaction();

public:

    /** Allocates an SQL database object and creates all of the required tables. */
    SQLDatabase(const std::string& name);
    SQLDatabase(const SQLDatabase&) = delete;

    /** Deallocates the SQL database object and frees all prepared statements*/
    ~SQLDatabase();

    /** Adds the provided entry to the SQL database */
    void insert_data(const DataEntry& data, timestamp_t timestamp);
};

#endif