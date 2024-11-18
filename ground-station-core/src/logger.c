#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <pthread.h>

#include "logger.h"
#include "sqlite3.h"
#include "database.h"

#define BUFFER1_LEN 1000

char buffer0[1000];
char buffer1[BUFFER1_LEN];


sqlite3 *DATABASE_REF; 
int counter = 1;
unsigned int num_packets_recieved = 0;
unsigned int num_packets_failed = 0;

void increment_packets_recieved() {
    __atomic_fetch_add(&num_packets_recieved, 1, __ATOMIC_RELAXED);
}

int get_num_packets_recieved() {
    return __atomic_load_n(&num_packets_recieved, __ATOMIC_RELAXED);
}

void increment_packets_failed() {
    __atomic_fetch_add(&num_packets_failed, 1, __ATOMIC_RELAXED);
}

int get_num_packets_failed() {
    return __atomic_load_n(&num_packets_failed, __ATOMIC_RELAXED);
}

void log_sql_message(sqlite3* db, int priority, char* message) {
    if (db == NULL) {
        printf("Database was null!\n");
        return;
    }
    
    // Clean message
    int len = strlen(message);
    for (int i = 0; i < len; i++) {
        if (isprint(message[i]) == 0) {
            message[i] = '?';
        } else if (message[i] == '"') {
            message[i] = '~';
        } else if (message[i] == '\'') {
            message[i] = '~';
        }
    }

    sprintf(buffer1, "INSERT INTO LogData VALUES (\"%d\", \"%d\", ?);", counter++, priority);
    struct sqlite3_stmt *statement; 
    if (sqlite3_prepare_v2(db, buffer1, BUFFER1_LEN, &statement, NULL) != SQLITE_OK) {
        printf("Error preparing log statement");
        sqlite3_finalize(statement);
        return;
    }

    sqlite3_bind_text(statement, 1, message, -1, SQLITE_STATIC);

    execute_sql_statement_no_callback(db, statement);

    sqlite3_finalize(statement);
}

void set_sql_database(sqlite3 *db) {
    DATABASE_REF = db;
}

int get_num_logged_messages() {
    return counter;
}

inline void log_header(const char* format, ...) {
    va_list args;
    va_start (args, format);
    fprintf(stderr, COLOR_BOLD_WHITE);
    vfprintf(stderr, format, args);
    fprintf(stderr, COLOR_DEFAULT);
    va_end (args);
}

inline void log_warning(const char* format, ...) {
    va_list args;
    va_start (args, format);
    fprintf(stderr, COLOR_YELLOW "[WARNING] ");
    vfprintf(stderr, format, args);
    vsprintf(buffer0, format, args);
    log_sql_message(DATABASE_REF, WARNING, buffer0);
    fprintf(stderr, COLOR_DEFAULT);
    va_end (args);
}

void log_warning_alt_db(sqlite3 *db, const char* format, ...) {
    va_list args;
    va_start (args, format);
    fprintf(stderr, COLOR_YELLOW "[WARNING] ");
    vfprintf(stderr, format, args);
    vsprintf(buffer0, format, args);
    log_sql_message(db, WARNING, buffer0);
    fprintf(stderr, COLOR_DEFAULT);
    va_end (args);
}

inline void log_error(const char* format, ...) {
    va_list args;
    va_start (args, format);
    fprintf(stderr, COLOR_RED "[ERROR] ");
    vfprintf(stderr, format, args);
    vsprintf(buffer0, format, args);
    log_sql_message(DATABASE_REF, ERROR, buffer0);
    fprintf(stderr, COLOR_DEFAULT);
    va_end (args);
}

inline void log_debug(const char* format, ...) {
    va_list args;
    va_start (args, format);
    fprintf(stderr, COLOR_GREEN "[DEBUG] ");
    vfprintf(stderr, format, args);
    vsprintf(buffer0, format, args);
    log_sql_message(DATABASE_REF, DEBUG, buffer0);
    fprintf(stderr, COLOR_DEFAULT);
    va_end (args);
}

inline void log_sql_debug(const char* format, ...) {
    va_list args;
    va_start (args, format);
    fprintf(stderr, COLOR_GREEN "[DEBUG] ");
    vfprintf(stderr, format, args);
    fprintf(stderr, COLOR_DEFAULT);
    va_end (args);
}

inline void log_standard(const char* format, ...) {
    va_list args;
    va_start (args, format);
    vfprintf(stderr, format, args);
    va_end (args);
}