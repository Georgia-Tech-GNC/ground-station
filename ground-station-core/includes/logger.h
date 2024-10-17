#ifndef LOGGER_H
#define LOGGER_H

#include "sqlite3.h"

#define COLOR_BOLD_WHITE "\033[1;37m"
#define COLOR_RED "\033[0;31m"
#define COLOR_YELLOW "\033[0;33m" 
#define COLOR_GREEN "\033[0;32m"
#define COLOR_DEFAULT "\033[0m"

#define CREATE_LOGGER_SQL_DB_COMMAND "CREATE TABLE LogData (id int PRIMARY KEY, priority int, message text);"

enum PriorityLevel {
    SQL_DEBUG = 1,
    DEBUG = 2,
    WARNING = 3,
    ERROR = 4
};

int get_num_logged_messages();

void log_sql_debug(const char* format, ...);

void set_sql_database(sqlite3 *db);

void log_header(const char* format, ...);

void log_warning(const char* format, ...);

void log_error(const char* format, ...);

void log_debug(const char* format, ...);

void log_standard(const char* format, ...);

#endif  