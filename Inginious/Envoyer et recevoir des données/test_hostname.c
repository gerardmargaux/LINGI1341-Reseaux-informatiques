#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>


int main(int argc, char const *argv[]) {
  int err; // Error check
  char* hostname = (char*) malloc(sizeof(char)*128);
  err = gethostname(hostname, sizeof(hostname));
  if(err == -1){
    fprintf(stderr, "ERROR : fonction gethostname()\n");
    free(hostname);
    return -1;
  }
  printf("Hostname : %s\n", hostname);
  return 0;
}
