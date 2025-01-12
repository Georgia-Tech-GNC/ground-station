#include "sql_task_queue.h"
#include "sqlite3.h"

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

#define NUM_POSSIBLE_SQL_STATEMENTS 2

sqlite3_stmt *compiled_sql_statements[NUM_POSSIBLE_SQL_STATEMENTS];

uint8_t execute_task(const Task* task) {

}

bool initialize_database_executer(sqlite3 *db) {
    // Initialize all statements
    int result;
   
    result = sqlite3_prepare_v2(db, "INSERT INTO TEST_ONE VALUES (?, ?, ?)", 38, &(compiled_sql_statements[0]), NULL);

    if (result != SQLITE_OK) {
        printf("COMPILING PREPARED STATEMENT 0 FAILED WITH SQL ERROR CODE %d", result);
        return false;
    }

    result = sqlite3_prepare_v2(db, "INSERT INTO TEST_TWO VALUES (?, ?, ?, ?)", 41, &(compiled_sql_statements[1]), NULL);

    if (result != SQLITE_OK) {
        printf("COMPILING PREPARED STATEMENT 1 FAILED WITH SQL ERROR CODE %d", result);
        return false;
    }

    return true;
}

void finalize_database_executer() {
    for (int i = 0; i < NUM_POSSIBLE_SQL_STATEMENTS; i++) {
        sqlite3_finalize(compiled_sql_statements[i]);
    }
}