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
  int sfd = socket(AF_INET6, SOCK_DGRAM, 0);
  if (sfd < 0){
    fprintf(stderr, "ERROR : sfd negatif\n", );
    return -1;
  }
  const void * message = "Je t'envoie un message\n";
  int rep = sendto(int sfd, message, int len, 0, "::1", 0);
  if(rep < 0){
    fprintf(stderr, "Erreur lors de l'appel de la fonction sendto\n", );
    return -1;
  }
  return 0;
}
