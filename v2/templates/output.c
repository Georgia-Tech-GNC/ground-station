#include "sql_task_queue.h"
#include "sqlite3.h"

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

#define NUM_POSSIBLE_SQL_STATEMENTS 1

sqlite3_stmt *compiled_sql_statements[NUM_POSSIBLE_SQL_STATEMENTS];

uint8_t execute_task(const Task* task) {

}

bool initialize_database_executer(sqlite3 *db) {
    int result;
    
    result = sqlite3_prepare_v2(db, "INSERT INTO Packet1 VALUES (?, ?);", 35, &(compiled_sql_statements[0]), NULL);

    if (result != SQLITE_OK) {
        printf("COMPILING PREPARED STATEMENT 0 FAILED WITH SQL ERROR CODE %d", result);
        return false;
    }
    

    return true;
}

void finalize_database_executer() {
    for (int i = 0; i < NUM_POSSIBLE_SQL_STATEMENTS; i++) {
        sqlite3_finalize(compiled_sql_statements[i]);
    }
}