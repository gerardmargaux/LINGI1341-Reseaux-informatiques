#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/poll.h>

#define STD_IN 0
#define STD_OUT 1


int main(int argc, char const *argv[]) {

  int err; // Variable pour error check
  struct addrinfo hints;
  struct addrinfo *servinfo;

  // addrinfo
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET6;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = 0;

  err = getaddrinfo("::1", "12345", &hints, &servinfo);
  if(err != 0){
    fprintf(stderr, "ERROR getaddrinfo() : %s\n", gai_strerror(err));
    return -1;
  }

  // socket
  int sfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
  if(sfd == -1){
    perror("ERROR socket()");
    return -1;
  }

  // bind
  err = bind(sfd, servinfo->ai_addr, servinfo->ai_addrlen);
  if(err == -1){
    perror("ERROR bind()");
    return -1;
  }

  // connect
  err = connect(sfd, servinfo->ai_addr, servinfo->ai_addrlen);
  if(err == -1){
    perror("ERROR connect()");
    return -1;
  }

  freeaddrinfo(servinfo);
  printf("Successful connection !\n");

  // Envoi du message
  char* message = "Incepserver";
  int message_len = strlen(message);
  err = send(sfd, message, message_len, 0);
  if(err == -1){
    perror("ERROR send()");
    return -1;
  }

  // Réception du message
  char* buffer = (char*) malloc(message_len*sizeof(char));
  if(buffer == NULL){
    fprintf(stderr, "ERROR malloc()\n");
    return -1;
  }
  err = recv(sfd, buffer, message_len, 0);
  if(err == -1){
    perror("ERROR recv()");
    free(buffer);
    return -1;
  }

  // Affichage du message
  printf("Message envoyé et reçu : %s\n", buffer);
  free(buffer);

  return 0;
}
