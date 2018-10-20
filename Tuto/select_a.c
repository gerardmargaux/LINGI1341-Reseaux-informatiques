/*
** select.c -- a select() demo
*/

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

#define STDIN 0  // file descriptor for standard input

int main(void)
{

  int err;
  char* recvbuf = (char*) malloc(512);
  //int sret;
  int bytes_received;
  int bytes_sent;

  char* hostname = "::1";
  char* port = "12345";


  int sockfd; // Variable qui va contenir le file descriptor du socket
  struct addrinfo hints, *servinfo;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET6;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = IPPROTO_UDP;

  err = getaddrinfo(hostname, port, &hints, &servinfo);
  if(err != 0){
    perror("Erreur getaddrinfo");
    return -1;
  }

  sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
  if(sockfd == -1){
    perror("Erreur socket");
    freeaddrinfo(servinfo);
    return -1;
  }

  err = bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
  if(err == -1){
    perror("Erreur bind");
    freeaddrinfo(servinfo);
    close(sockfd);
    return -1;
  }

  printf("Socket créé\n");
  freeaddrinfo(servinfo);

  while(1){

      struct sockaddr b;
      socklen_t blen = sizeof(struct sockaddr);


      bytes_received = recvfrom(sockfd, recvbuf, 512, 0, &b, &blen);
      if(bytes_received < 0){
        perror("recvfrom");
      }
      printf("Chaine reçue : %s\n", recvbuf);


      memset(recvbuf, 0, 512);

    }

    free(recvbuf);
    close(sockfd);
    return 0;
}
