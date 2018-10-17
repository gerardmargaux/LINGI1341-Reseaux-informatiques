#include "lib.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <getopt.h>
#include <ctype.h>
#include <zlib.h>
#include <errno.h>
#include <fcntl.h>

#define STDIN 0
#define STDOUT 1
#define STDERR 2


int main(int argc, char const *argv[]) {
  uint8_t type = 2;
  uint8_t tr = 0;
  uint8_t window = 28;
  uint8_t type_format = type<<6 & 0b00000011000000;
	uint8_t tr_format = tr<<5 & 0b00000100000;
	uint8_t window_format = window & 0b00011111;
  printf("type_format = %d\n", type_format);
  printf("tr_format = %d\n", tr_format);
  printf("window_format = %d\n", window_format);
  uint8_t byte = (type_format | tr_format | window_format) & 0b00000011111111;
  printf("byte = %d\n", byte);
  return 0;
}
