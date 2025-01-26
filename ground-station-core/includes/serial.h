#include <termios.h>

speed_t serial_get_speed(int baud_number);

char *serial_get_baud_name(int baud_number);

int serial_set_attribs(int fd, int baud_number);

FILE *serial_initialize(char *serial_path, int baud_number);
