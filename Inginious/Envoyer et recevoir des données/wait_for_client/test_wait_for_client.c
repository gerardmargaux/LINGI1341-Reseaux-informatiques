#include "wait_for_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>


/* Block the caller until a message is received on sfd,
 * and connect the socket to the source addresse of the received message.
 * @sfd: a file descriptor to a bound socket but not yet connected
 * @return: 0 in case of success, -1 otherwise
 * @POST: This call is idempotent, it does not 'consume' the data of the message,
 * and could be repeated several times blocking only at the first call.
 */
int wait_for_client(int sfd){

	struct sockaddr_in6 * address = (struct sockaddr_in6 *)calloc(1024,sizeof(struct sockaddr_in6));
	if(address == NULL){
    fprintf(stderr, "ERROR : fonction calloc()\n");
    return -1;
  }

	char * buf = (char *)malloc(1024*sizeof(char));
	if(buf == NULL){
    fprintf(stderr, "ERROR : fonction malloc()\n");
    return -1;
  }

	socklen_t len = sizeof(*address);
  int err;

  err = recvfrom(sfd, buf, sizeof(buf), MSG_PEEK, (struct sockaddr *) address, &len);
  if(err < 0){
    fprintf(stderr, "Erreur de la fonction recvfrom\n");
    return -1;
  }

  err = connect(sfd, (struct sockaddr *)address, len);
  if(err < 0){
    fprintf(stderr, "Erreur de connexion\n");
  }
  return 0;
}

/*
Fonction de test qui appelle wait_for_client et renvoie la valeur de retour
de cette fonction.
Retourne -1 en cas d'erreur
*/

int main (int argc, char const *argv[]) {
  int err;
  struct addrinfo *hints = (struct addrinfo *) calloc(1, sizeof(struct addrinfo));
  if(hints == NULL){
    fprintf(stderr, "ERROR : fonction calloc()\n");
    return -1;
  }
  hints->ai_family = AF_INET6;
  hints->ai_socktype = SOCK_DGRAM;
  hints->ai_protocol = 0;

  struct addrinfo *res;

  err = getaddrinfo("::1", "12345", hints, &res);
  if(err != 0){
    perror("getaddrinfo");
    free(hints);
    return -1;
  }

  int sfd = socket(AF_INET6, SOCK_DGRAM, 0);
  if (sfd < 0){
    fprintf(stderr, "ERROR : sfd negatif\n");
    return -1;
  }
  bind(sfd, res->ai_addr, res->ai_addrlen);
	printf("J'attends un message\n");
  int val = wait_for_client(sfd);
  return val;
}
