/*
** select.c -- a select() demo
*/

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>
#include <string.h>

#define STDIN 0  // file descriptor for standard input

int main(void)
{

  char buf[512];
  int sret;
  int bytes_read;

  struct timeval tv;
  fd_set readfds;


  while(1){

      FD_ZERO(&readfds);
      FD_SET(STDIN, &readfds);

      tv.tv_sec = 7;
      tv.tv_usec = 500000;

      // don't care about writefds and exceptfds:
      sret = select(STDIN+1, &readfds, NULL, NULL, &tv);

      if(sret == 0){
        printf("Timed out.\n");
      }
      else{
        memset(buf, 0, 512);
        bytes_read = read(STDIN, buf, 512);
        if(bytes_read == 0){
          printf("Function terminated.\n");
          break;
        }
        else{
          printf("Le buffer a été activé !\n");
          *(buf+strlen(buf)-1) = '\0';
          printf("Buffer : %s\n", buf);
        }
      }
    }

    return 0;
}
