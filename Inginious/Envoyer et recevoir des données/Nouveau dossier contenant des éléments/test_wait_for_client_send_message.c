#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

int main (int argc, char const *argv[]) {
  struct addrinfo *hints = (struct addrinfo *) calloc(1, sizeof(struct addrinfo));
  if(hints == NULL){
    fprintf(stderr, "ERROR : fonction calloc()\n");
    return -1;
  }
  hints->ai_family = AF_INET6;
  hints->ai_socktype = SOCK_DGRAM;
  hints->ai_protocol = 0;

  struct addrinfo *res;

  err = getaddrinfo("::1", NULL, hints, &res);
  if(err != 0){
    perror("getaddrinfo");
    free(hints);
    return -1;
  }
  int sfd = socket(AF_INET6, SOCK_DGRAM, 0);
  if (sfd < 0){
    fprintf(stderr, "ERROR : sfd negatif\n", );
    return -1;
  }
  const void * message = "Je t'envoie un message\n";
  int rep = sendto(int sfd, message, int len, 0, "::1", res->ai_addr);
  if(rep < 0){
    fprintf(stderr, "Erreur lors de l'appel de la fonction sendto\n", );
    return -1;
  }
  return 0;
}
