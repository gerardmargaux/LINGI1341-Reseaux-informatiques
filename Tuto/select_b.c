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
  int sret;
  int bytes_sent;
  int bytes_read;
  int bytes_received;
  char* sendbuf = (char*) malloc(512);
  char* recvbuf = (char*) malloc(512);

  char* hostname = "::1";
  char* port = "12345";

  struct timeval tv;
  fd_set readfds;

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

  printf("Socket créé\n");


  while(1){

    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);

    tv.tv_sec = 2;
    tv.tv_usec = 500000;

    struct sockaddr a;
    socklen_t alen = sizeof(struct sockaddr);

    bytes_read = read(STDIN, sendbuf, 512);
    if(bytes_read == 0){
      printf("Function terminated.\n");
      break;
    }
    *(sendbuf+strlen(sendbuf)-1) = '\0';
    printf("Chaine lue : %s\n", sendbuf);

    bytes_sent = sendto(sockfd, sendbuf, strlen(sendbuf), 0, servinfo->ai_addr, servinfo->ai_addrlen);
    if(bytes_sent < 0){
      perror("sendto");
    }
    sret = select(sockfd+1, &readfds, NULL, NULL, &tv);

    while(sret == 0){
      printf("Renvoi du paquet\n");
      bytes_sent = sendto(sockfd, sendbuf, strlen(sendbuf), 0, servinfo->ai_addr, servinfo->ai_addrlen);
      if(bytes_sent < 0){
        perror("sendto");
      }

      FD_ZERO(&readfds);
      FD_SET(sockfd, &readfds);

      tv.tv_sec = 2;
      tv.tv_usec = 500000;
      
      sret = select(sockfd+1, &readfds, NULL, NULL, &tv);
    }
      if(FD_ISSET(sockfd, &readfds)){
        bytes_received = recvfrom(sockfd, recvbuf, 512, 0, &a, &alen);
        if(bytes_received < 0) {
          perror("recvfrom");
        }
        printf("ACK reçu : %s\n", recvbuf);
      }

    memset(sendbuf, 0, 512);
    memset(recvbuf, 0, 512);

  }

  free(sendbuf);
  free(recvbuf);
  freeaddrinfo(servinfo);
  close(sockfd);
  return 0;
}
