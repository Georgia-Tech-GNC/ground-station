#include <stdio.h>
#include <stdarg.h>

#include "logger.h"
#include "sqlite3.h"
#include "database.h"

char buffer0[1000];
char buffer1[1000];


sqlite3 *DATABASE_REF; 
int counter = 1;

void log_sql_message(int priority, char* message) {
    if (DATABASE_REF == NULL) return;
    sprintf(buffer1, "INSERT INTO LogData VALUES (\"%d\", \"%d\", \"%s\");", counter++, priority, message);
    execute_sql_command_no_callback(DATABASE_REF, buffer1);
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
    log_sql_message(WARNING, buffer0);
    fprintf(stderr, COLOR_DEFAULT);
    va_end (args);
}

inline void log_error(const char* format, ...) {
    va_list args;
    va_start (args, format);
    fprintf(stderr, COLOR_RED "[ERROR] ");
    vfprintf(stderr, format, args);
    vsprintf(buffer0, format, args);
    log_sql_message(ERROR, buffer0);
    fprintf(stderr, COLOR_DEFAULT);
    va_end (args);
}

inline void log_debug(const char* format, ...) {
    va_list args;
    va_start (args, format);
    fprintf(stderr, COLOR_GREEN "[DEBUG] ");
    vfprintf(stderr, format, args);
    vsprintf(buffer0, format, args);
    log_sql_message(DEBUG, buffer0);
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