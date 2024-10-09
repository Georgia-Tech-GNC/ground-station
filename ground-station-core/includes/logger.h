#ifndef LOGGER_H
#define LOGGER_H

#define COLOR_BOLD_WHITE "\033[1;37m"
#define COLOR_RED "\033[0;31m"
#define COLOR_YELLOW "\033[0;33m" 
#define COLOR_GREEN "\033[0;32m"
#define COLOR_DEFAULT "\033[0m"

void log_header(const char* format, ...);

void log_warning(const char* format, ...);

void log_error(const char* format, ...);

void log_debug(const char* format, ...);

void log_standard(const char* format, ...);

#endif  