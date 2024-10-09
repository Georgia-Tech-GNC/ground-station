#include <stdio.h>
#include <stdarg.h>

#include "logger.h"

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
    fprintf(stderr, COLOR_DEFAULT);
    va_end (args);
}

inline void log_error(const char* format, ...) {
    va_list args;
    va_start (args, format);
    fprintf(stderr, COLOR_RED "[ERROR] ");
    vfprintf(stderr, format, args);
    fprintf(stderr, COLOR_DEFAULT);
    va_end (args);
}

inline void log_debug(const char* format, ...) {
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