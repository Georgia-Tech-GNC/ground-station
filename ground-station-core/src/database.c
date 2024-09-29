#include <stdio.h>
#include <stdbool.h>

#include "sqlite3.h"
#include "database.h"
#include "protocol.h"

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
    fprintf(stderr, "Creating database with name: %s\n", filename);

    // Create the database
    int result = sqlite3_open(filename, db);
    if (db == NULL || result != SQLITE_OK) {
        fprintf(stderr, "[ERROR] Database creation failed!\n\tError message: %s\n", sqlite3_errmsg(*db));
        return PROCESS_ABORT_CODE;
    } else {
        fprintf(stderr, "Successfully created database!\n");
    }

    result = initialize_empty_database(*db);
    if (result != PROCESS_SUCCESSFUL_CODE) {
        fprintf(stderr, "[ERROR] Failed to correctly initialize database tables!\n");
        return PROCESS_ABORT_CODE;
    } else {
        fprintf(stderr, "Successfully initialized database!\n");
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
        fprintf(stderr, "[WARNING] Attempt to read data contents of message code %d failed\n", message_id);
        return PROCESS_ABORT_CODE;
    }

    fprintf(stderr, "SQL Command: %s\n", command_buffer);


    int result = execute_sql_command_no_callback(db, command_buffer);
    if (result != PROCESS_SUCCESSFUL_CODE) {
        fprintf(stderr, "[WARNING] Attempt to copy data contents of message code %d to SQL database failed\n", message_id);
        return PROCESS_ABORT_CODE;
    } else {
        fprintf(stderr, "Copied contents of message code %d to SQL database\n", message_id);
        return PROCESS_SUCCESSFUL_CODE;
    }
}

int execute_sql_command_no_callback(sqlite3 *db, char *sql) {
    char *zErrMsg = NULL;

    int result = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);

    if(result != SQLITE_OK ){
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
      return PROCESS_ABORT_CODE;
    } 

    return PROCESS_SUCCESSFUL_CODE;
}