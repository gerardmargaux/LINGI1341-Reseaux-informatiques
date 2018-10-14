#include "create_socket.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>


/* Creates a socket and initialize it
 * @source_addr: if !NULL, the source address that should be bound to this socket
 * @src_port: if >0, the port on which the socket is listening
 * @dest_addr: if !NULL, the destination address to which the socket should send data
 * @dst_port: if >0, the destination port to which the socket should be connected
 * @return: a file descriptor number representing the socket,
 *         or -1 in case of error (explanation will be printed on stderr)
 */
int create_socket(struct sockaddr_in6 *source_addr, int src_port, struct sockaddr_in6 *dest_addr, int dst_port){

  int err; // Variable pour error check

  // Création du socket
  int socketfd = socket(PF_INET6, SOCK_DGRAM, 0);
  if(socketfd < 0){
    fprintf(stderr, "ERROR : fonction socket()\n");
    return -1;
  }

  // Liaison à la source
  if(source_addr != NULL && src_port > 0){
    source_addr->sin6_port = htons(src_port); // Utilisation de htons pour convertir en Network byte order
    err = bind(socketfd, (struct sockaddr*) source_addr, sizeof(source_addr));
    // Error check
    if(err != 0){
      fprintf(stderr, "ERROR : fonction bind() source\n");
      return -1;
    }
  }

  // Liaison à la destination
  if(dest_addr != NULL && dst_port > 0){
    dest_addr->sin6_port = htons(dst_port); // Utilisation de htons pour convertir en Network byte order
    err = connect(socketfd, (struct sockaddr*) dest_addr, sizeof(dest_addr));
    // Error check
    if(err != 0){
      fprintf(stderr, "ERROR : fonction connect() destination\n");
      return -1;
    }
  }

  return socketfd;
}


/*
 * Teste la fonction create_socket avec la machine courante
 */
int main(int argc, char* argv[]){

  int err; // Variable pour error check
  int sfd;

  char *hostname = (char*) malloc(128*sizeof(char));
  if(hostname == NULL){
    fprintf(stderr, "ERROR : fonction malloc()\n");
    return -1;
  }

  err = gethostname(hostname, sizeof(hostname));
  if(err != 0){
    fprintf(stderr, "ERROR : fonction gethostname()\n");
    free(hostname);
    return -1
  }

  struct addrinfo *hints = (struct addrinfo *) calloc(1, sizeof(struct addrinfo));
  if(hints == NULL){
    fprintf(stderr, "ERROR : fonction calloc()\n");
    free(hostname);
    return -1;
  }
  hints->ai_family = AF_INET6;
  hints->ai_socktype = SOCK_DGRAM;
  hints->ai_protocol = 0;

  struct addrinfo *ai;

  err = getaddrinfo(hostname, "9002", hints, &ai);
  if(err != 0){
    fprintf(stderr, "ERROR : fonction getaddrinfo()\n");
    free(hostname);
    freeaddrinfo(hints);
    return -1;
  }
  freeaddrinfo(hints);

  struct sockaddr_in6 *sa = (struct sockaddr_in6 *) ai->ai_addr;

  sfd = create_socket(sa, sa->sin6_port, sa, sa->sin6_port);
  if(sfd == -1){
    fprintf(stderr, "ERROR : fonction create_socket()\n");
    free(hostname);
  }

  free(hostname);
  printf("SUCCESS !\n");

  return 0;
}
