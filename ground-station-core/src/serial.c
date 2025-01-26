#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <unistd.h>
#include "serial.h"

//#define IN_OFF_SERIAL_TESTING_MODE

speed_t serial_get_speed(int baud_number) {
  switch (baud_number) {
  case 0: return B1200;
  case 1: return B2400;
  case 2: return B4800;
  case 3: return B9600;
  case 4: return B19200;
  case 5: return B38400;
  case 6: return B57600;
  case 7: return B115200;
  default: 
    printf("Invalid baud number inputted: %d", baud_number);
    exit(1);
  }
}

char *serial_get_baud_name(int baud_number) {
  switch (baud_number) {
  case 0: return "1200";
  case 1: return "2400";
  case 2: return "4800";
  case 3: return "9600";
  case 4: return "19200";
  case 5: return "38400";
  case 6: return "57600";
  case 7: return "115200";
  default: 
    printf("Invalid baud number inputted: %d", baud_number);
    exit(1);
  }
}

int serial_set_attribs(int fd, int baud_number) {
  struct termios tty;

  if (tcgetattr(fd, &tty) < 0) {
    perror("tcgetattr");
    return -1;
  }

  printf("cfsetospeed and cfsetispeed with baud %s...\n", serial_get_baud_name(baud_number));
  if (0 != cfsetospeed(&tty, serial_get_speed(baud_number))) {
    perror("cfsetospeed");
    return -1;
  }
  if (0 != cfsetispeed(&tty, serial_get_speed(baud_number))) {
    perror("cfsetispeed");
    return -1;
  }

  tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
  tty.c_cflag &= ~CSIZE;
  tty.c_cflag |= CS8;         /* 8-bit characters */
  tty.c_cflag &= ~PARENB;     /* no parity bit */
  tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
  tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

  /* setup for non-canonical mode */
  tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
  tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
  tty.c_oflag &= ~OPOST;

  /* fetch bytes as they become available */
  tty.c_cc[VMIN] = 1;
  tty.c_cc[VTIME] = 1;

  if (tcsetattr(fd, TCSANOW, &tty) != 0) {
    perror("tcsetattr");
    return -1;
  }
  return 0;
}

FILE *serial_initialize(char *serial_path, int baud_number) {
  errno = 0;
  FILE *serial = fopen(serial_path, "r+b");
  printf("fopen finished\n");
  if (serial == NULL || errno != 0) {
    perror("open serial file");
    return NULL;
  }
  printf("file opened\n");

#ifndef IN_OFF_SERIAL_TESTING_MODE
  if (serial_set_attribs(fileno(serial), baud_number) != 0) {
    printf("serial_set_attribs failed\n");
    exit(1);
  }
#endif
  return serial;
}
