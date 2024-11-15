#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define INCLUDE_PROTOCOL_SQL_MACROS

#include "sqlite3.h"
#include "database.h"
#include "protocol.h"
#include "logger.h"

struct SLLNode {
    struct SLLNode* next;
    char* data;
};

struct SLL {
    struct SLLNode* head;
    size_t total_size_bytes;
    int length;
};

void free_sll(struct SLL* sll) {
    if (sll == NULL) return;

    while (sll->head) {
        struct SLLNode *current_node = sll->head;
        if (current_node->data) free(current_node->data);
        sll->head = current_node->next;
        free(current_node);
    }
}

int initialize_empty_database(sqlite3 *db) {
    char* create_cmds[] = {CREATE_LOGGER_SQL_DB_COMMAND, ALL_SQL_TABLE_CREATE_COMMANDS};

    for (int i = 0; i < NUM_SQL_TABLES + 1; i++) {
        int r = execute_sql_command_no_callback(db, create_cmds[i]);
        if (r == PROCESS_ABORT_CODE) return PROCESS_ABORT_CODE;
    }

    // Now generate the logger database
    

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

int parse_data_packet_into_sql(sqlite3 *db, int message_id, uint8_t *data, size_t data_size) {
    char command_buffer[MAX_SQL_WRITE_CMD_SIZE] = {0};

    bool successful = convert_data_msg_to_sql_cmd(message_id, data, data_size, command_buffer);

    if (!successful) {
        log_warning("Attempt to read data contents of message code %d failed\n", message_id);
        return PROCESS_ABORT_CODE;
    }

    //log_sql_debug("Running SQL Command: %s\n", command_buffer);


    int result = execute_sql_command_no_callback(db, command_buffer);
    if (result != PROCESS_SUCCESSFUL_CODE) {
        log_warning( "Attempt to copy data contents of message code %d to SQL database failed\n", message_id);
        return PROCESS_ABORT_CODE;
    } else {
        log_sql_debug("Copied contents of message code %d to SQL database\n", message_id);
        return PROCESS_SUCCESSFUL_CODE;
    }
}

int execute_sql_command_no_callback(sqlite3 *db, char *sql) {
    char *zErrMsg = NULL;

    int result = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);

    if(result != SQLITE_OK ){
        if (zErrMsg == NULL) {
            printf("SQL error: no message given\n");
        } else {
            printf("Message: %s\n", sql);
            printf("SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
      
      return PROCESS_ABORT_CODE;
    } 

    return PROCESS_SUCCESSFUL_CODE;
}

int execute_sql_statement_no_callback(sqlite3 *db, struct sqlite3_stmt *sql) {

    int result = sqlite3_step(sql);

    if(result != SQLITE_DONE ) {
      printf("SQL error: no message given (log statement with no provided feedback, message id %d)\n", result);
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

int sql_log_message_callback(void* sll_void_pointer, int num_cols, char** values, char** names) {
    // values[0] -> msg_id, values[1] -> message
    struct SLL *sll = sll_void_pointer;

    size_t msg_len = strlen(values[2]) + strlen(values[1]) + strlen(values[0]) + sizeof("\"\": {\"priority\":\"\",\"value\":\"\"}"); // "msg_id": {"priority":A,"value":B}

    struct SLLNode *nodeToAdd = malloc(sizeof(struct SLLNode));
    nodeToAdd->data = malloc(msg_len + 1);

    strcpy(nodeToAdd->data, "\"");       // First "
    strcat(nodeToAdd->data, values[0]);  // Copy the message id
    strcat(nodeToAdd->data, "\": {\"priority\":\"");   // Add ": {"priority":"
    strcat(nodeToAdd->data, values[1]);  // Add the priority
    strcat(nodeToAdd->data, "\",\"value\":\"");  // Add ","value":"
    strcat(nodeToAdd->data, values[2]);  // Copy the message
    strcat(nodeToAdd->data, "\"}");       // Terminating "}

    nodeToAdd->next = sll->head;         // Add to linked list
    sll->head = nodeToAdd;

    sll->total_size_bytes += msg_len;   // Add to total message length (COUNTING NULL TERMINATOR)
    sll->length++;                      // Increment length\

    return 0;
}

int query_sql_get_logs(sqlite3 *db, int min_index, char** return_json_str) {
    char *zErrMsg = NULL;

    char sql_command[65];
    sprintf(sql_command, "Select id, priority, message FROM LogData WHERE id > %d", min_index);

    struct SLL result = {
        .head = NULL, 
        .total_size_bytes = 0,
        .length = 0
    };

    int op_code = sqlite3_exec(db, sql_command, sql_log_message_callback, &result, &zErrMsg);

    if(op_code != SQLITE_OK ){
        log_warning("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        free_sll(&result);

        *return_json_str = malloc(sizeof("{\"empty\": \"True\"}"));
        strcpy(*return_json_str, "{\"empty\": \"True\"}");

        return PROCESS_ABORT_CODE;
    } 

    if (!result.head) {
        *return_json_str = malloc(sizeof("{\"empty\": \"True\"}"));
        strcpy(*return_json_str, "{\"empty\": \"True\"}");
        return PROCESS_SUCCESSFUL_CODE;
    }

    *return_json_str = malloc(result.total_size_bytes + sizeof("{}") + (result.length*sizeof(", ")));

    strcpy(*return_json_str, "{");

    struct SLLNode *currentNode = result.head;
    for (int i = 0; i < result.length; i++) {
        if (!currentNode) break;
        if (i != 0) strcat(*return_json_str, ", ");

        strcat(*return_json_str, currentNode->data);
        currentNode = currentNode->next;
    }

    strcat(*return_json_str, "}");

    free_sll(&result);

    return PROCESS_SUCCESSFUL_CODE;
}

int query_sql_with_callback(sqlite3 *db, char** data_buffer_ptr, char* sql_cmd) {
    char *zErrMsg = NULL;

    //log_sql_debug("Running SQL Command: %s\n", sql_cmd);

    int result = sqlite3_exec(db, sql_cmd, sql_column_callback, data_buffer_ptr, &zErrMsg);


    if(result != SQLITE_OK ){
      printf("Message: %s\n", sql_cmd);
      printf("SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
      return PROCESS_ABORT_CODE;
    } 

    return PROCESS_SUCCESSFUL_CODE;
}

