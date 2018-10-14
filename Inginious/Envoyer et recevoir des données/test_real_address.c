#include "real_address.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

const char *real_address(const char *address, struct sockaddr_in6 *rval){
  
    int err;

    struct addrinfo *hints = (struct addrinfo *) calloc(1, sizeof(struct addrinfo));
    if(hints == NULL){
      return "Erreur d'alloction de mémoire.";
    }
    struct addrinfo *results;

    //memset(hints, 0, sizeof(hints)); inutile si on fait calloc() au-dessus
    hints->ai_family = AF_INET6;
    hints->ai_protocol = IPPROTO_UDP;
		hints->ai_socktype = SOCK_DGRAM; // Car UDP packets
		// hints.ai_socktype = SOCK_STREAM; si TCP packets

    err = getaddrinfo(address, "http", hints, &results);
    if(err != 0){
        return "Erreur d'exécution de la fonction addrinfo.";
    }

    struct sockaddr *temp = results->ai_addr;
    struct sockaddr_in6 *final = (struct sockaddr_in6 *) temp;
    *rval = *final;

    freeaddrinfo(results);
    return NULL;
}

// Teste la fonction real_address()
int main(int argc, char* argv[]){

  char* err = (char*) malloc(100*sizeof(char));
  if(err == NULL){
    fprintf(stderr, "ERROR : fonction malloc()\n");
    return -1;
  }

  char* address = argv[1];

  struct sockaddr_in6 *rval = (struct sockaddr_in6 *) malloc(sizeof(struct sockaddr_in6));
  if(rval == NULL){
    fprintf(stderr, "ERROR : fonction malloc()\n");
    return -1;
  }

  err = real_address(address, rval);
  if(err != NULL){
    fprintf(stderr, "%s\n", err);
    free(err);
    free(rval);
    return -1;
  }
  char* net_address = (rval->sin6_addr).s6_addr;
  char* printable_address = (char*) malloc(100*sizeof(char));
  if(printable_address == NULL){
    fprintf(stderr, "ERROR : fonction malloc()\n");
    return -1;
  }
  inet_ntop(AF_INET6, net_address, printable_address, 100);

  printf("Address name : %s\n", printable_address);

  return 0;
}
