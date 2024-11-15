#ifndef DATABASE_H
#define DATABASE_H

#include <stdint.h>
#include <time.h>
#include "sqlite3.h"

#define PROCESS_ABORT_CODE 0
#define PROCESS_SUCCESSFUL_CODE 1

/**
 * Creates a new database file extension using a prespecified format
 * 
 * Filename: database-YYYY-MM-DD-HH-mm-ss.db
 * 
 * @param buffer    a 32-character string that will store the filename (including terminating byte)
 */
void generate_database_name(char *buffer);

/**
 * Executes an SQL command on the specified database without issuing a callback (any returned columns are disregarded)
 * 
 * @param db    the SQL database
 * @param sql   the SQL command, as a string
 * 
 * @return PROCESS_SUCCESSFUL_CODE if the operation was successful, and PROCESS_ABORT_CODE otherwise
 */
int execute_sql_command_no_callback(sqlite3 *db, char *sql);

/**
 * Creates a new SQL database using the pre-determined naming convention and initializes it with the tables specified by protocol.h
 * 
 * @param db            a pointer to the SQL database reference
 * @param database_name if not null, the name of the created database will be stored here
 * 
 * @return PROCESS_SUCCESSFUL_CODE if the operation was successful, and PROCESS_ABORT_CODE otherwise
 */
int generate_new_database(sqlite3 **db);

/**
 * Takes a data packet and message id (see protocol.h for format) and adds that data to the database
 * 
 * @param db            the SQL database
 * @param message_id    the id of the recieved message
 * @param data          the payload of the recieved message
 * @param data_size     the payload size
 * 
 * @return PROCESS_SUCCESSFUL_CODE if the operation was successful, and PROCESS_ABORT_CODE otherwise
 */
int parse_data_packet_into_sql(sqlite3 *db, int message_id, uint8_t *data, size_t data_size);

/**
 * Calls an SQL command, and stores the last row returned to the inputted string array
 * 
 * @param db                the SQL database
 * @param data_buffer_ptr   an array of pointers to strings, each representing one column from the queried SQL command
 *                          NOTE THAT THE USER MUST FREE THESE STRINGS MANUALLY
 * @param sql_cmd           the SQL command that should be run
 * 
 * @return PROCESS_SUCCESSFUL_CODE if the operation was successful, and PROCESS_ABORT_CODE otherwise
 */
int query_sql_with_callback(sqlite3 *db, char** data_buffer_ptr, char* sql_cmd);

/** Identical to execute_sql_command_no_callback, but with a prepared statement instead */
int execute_sql_statement_no_callback(sqlite3 *db, struct sqlite3_stmt *sql);

int query_sql_get_logs(sqlite3 *db, int min_index, char** return_json_str);

#endif
