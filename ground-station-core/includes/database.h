#ifndef DATABASE_H
#define DATABASE_H

#include <stdint.h>
#include <time.h>

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

int execute_sql_command_no_callback(sqlite3 *db, char *sql);

int generate_new_database(sqlite3 **db);

int parse_data_packet_into_sql(sqlite3 *db, int message_id, uint8_t *data);

#endif