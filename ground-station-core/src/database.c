#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define INCLUDE_PROTOCOL_SQL_MACROS

#include "sqlite3.h"
#include "database.h"
#include "protocol.h"
#include "logger.h"

int initialize_empty_database(sqlite3 *db) {
    char* create_cmds[] = {ALL_SQL_TABLE_CREATE_COMMANDS};

    for (int i = 0; i < NUM_SQL_TABLES; i++) {
        int r = execute_sql_command_no_callback(db, create_cmds[i]);
        if (r == PROCESS_ABORT_CODE) return PROCESS_ABORT_CODE;
    }

    return PROCESS_SUCCESSFUL_CODE;
}

int generate_new_database(sqlite3 **db) {
    // Define the filename
    char filename[32];
    generate_database_name(filename);
    log_debug("Creating database with name: %s\n", filename);

    // Create the database
    int result = sqlite3_open(filename, db);
    if (db == NULL || result != SQLITE_OK) {
        log_error("Database creation failed!\n\tError message: %s\n", sqlite3_errmsg(*db));
        return PROCESS_ABORT_CODE;
    } else {
        log_debug("Successfully created database!\n");
    }

    result = initialize_empty_database(*db);
    if (result != PROCESS_SUCCESSFUL_CODE) {
        log_error("Failed to correctly initialize database tables!\n");
        return PROCESS_ABORT_CODE;
    } else {
        log_debug("Successfully initialized database!\n");
    }

    return PROCESS_SUCCESSFUL_CODE;
}

void generate_database_name(char *buffer) {
    time_t rawtime;
    time(&rawtime);
    strftime(buffer, 32, "database-%Y-%M-%d-%H-%M-%S.db", localtime(&rawtime));
}

int parse_data_packet_into_sql(sqlite3 *db, int message_id, uint8_t *data) {
    char command_buffer[MAX_SQL_WRITE_CMD_SIZE] = {0};

    bool successful = convert_data_msg_to_sql_cmd(message_id, data, command_buffer);

    if (!successful) {
        log_warning("Attempt to read data contents of message code %d failed\n", message_id);
        return PROCESS_ABORT_CODE;
    }

    log_debug("Running SQL Command: %s\n", command_buffer);


    int result = execute_sql_command_no_callback(db, command_buffer);
    if (result != PROCESS_SUCCESSFUL_CODE) {
        log_warning( "Attempt to copy data contents of message code %d to SQL database failed\n", message_id);
        return PROCESS_ABORT_CODE;
    } else {
        log_debug("Copied contents of message code %d to SQL database\n", message_id);
        return PROCESS_SUCCESSFUL_CODE;
    }
}

int execute_sql_command_no_callback(sqlite3 *db, char *sql) {
    char *zErrMsg = NULL;

    int result = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);

    if(result != SQLITE_OK ){
      log_warning("SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
      return PROCESS_ABORT_CODE;
    } 

    return PROCESS_SUCCESSFUL_CODE;
}

int sql_column_callback(void* data_buffer_ptr, int num_cols, char** values, char** names) {
    char** data_buffer = data_buffer_ptr;
    for (int i = 0; i < num_cols; i++) {
        data_buffer[i] = malloc(strlen(values[i]) + 1);
        strcpy(data_buffer[i], values[i]);
    }
    
    return 0;
}

int query_sql_with_callback(sqlite3 *db, char** data_buffer_ptr, char* sql_cmd) {
    char *zErrMsg = NULL;

    log_debug("Running SQL Command: %s\n", sql_cmd);

    int result = sqlite3_exec(db, sql_cmd, sql_column_callback, data_buffer_ptr, &zErrMsg);


    if(result != SQLITE_OK ){
      log_warning("SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
      return PROCESS_ABORT_CODE;
    } 

    return PROCESS_SUCCESSFUL_CODE;
}

