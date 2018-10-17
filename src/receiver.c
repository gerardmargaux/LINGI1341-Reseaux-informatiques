// @Titre : Projet LINGI1341 : Réseaux informatiques
// @Auteurs : Francois DE KEERSMAEKER (7367 1600) & Margaux GERARD (7659 1600)
// @Date : 22 octobre 2018

/*
 * Receiver : programme qui reçoit des paquets de données depuis le réseau.
 *
 */

#include "lib.h"
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

#define STDIN 0
#define STDOUT 1
#define STDERR 2


/*
 * main : Fonction principale
 *
 */
int main(int argc, char *argv[]) {

  // Vérification du nombre d'arguments
  if(argc < 3){
    fprintf(stderr, "Pas assez d'arguments.\n");
    return -1;
  }
  else if(argc > 5){
    fprintf(stderr, "Trop d'arguments.\n");
    return -1;
  }


  int err; // Variable pour error check
  int fd = STDOUT; // File descriptor avec lequel on va écrire les données
  int bytes_received = 1; // Nombre de bytes reçus du sender
  int bytes_written; // Nombre de bytes écrits à chaque itération
  //pkt_status_code err_code; // Variable pour error check avec les paquets

  // Prise en compte des arguments avec getopt()
  extern char* optarg;
  extern int optind, opterr, optopt;
  char* optstring = "f:";

  char c = (char) getopt(argc, argv, optstring);
  if(c == '?'){
    fprintf(stderr, "Option inconnue.\n");
    fprintf(stderr, "Ecriture sur la sortie standard.\n");
  }
  else if(c == -1){ // Ecriture sur la sortie standard
    printf("Ecriture sur la sortie standard.\n");
  }
  else if(c == 'f'){ // Ecriture dans un fichier
    char* filename = optarg;
    printf("Ecriture dans le fichier %s\n", filename);
    fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if(fd == -1){
      perror("Erreur open fichier destination");
      return -1;
    }
  }

  char* hostname = argv[optind];
  printf("Hostname : %s\n", hostname);
  char* port = argv[optind+1];
  printf("Port : %s\n", port);

  // Création du socket
  struct sockaddr_in6 sender_addr, receiver_addr;
  memset(&sender_addr, 0, sizeof(sender_addr));
  memset(&receiver_addr, 0, sizeof(receiver_addr));
  int sockfd; // Variable qui va contenir le file descriptor du socket
  struct addrinfo hints, *servinfo;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET6;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = IPPROTO_UDP;

  err = getaddrinfo(hostname, port, &hints, &servinfo);
  if(err != 0){
    perror("Erreur getaddrinfo");
    close(fd);
    return -1;
  }

  sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
  if(sockfd == -1){
    perror("Erreur socket");
    freeaddrinfo(servinfo);
    close(fd);
    return -1;
  }

  err = bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
  if(err == -1){
    perror("Erreur bind");
    freeaddrinfo(servinfo);
    close(sockfd);
    close(fd);
    return -1;
  }

  freeaddrinfo(servinfo);

  while(bytes_received > 0){
    int addr_len;
    char buffer[MAX_PAYLOAD_SIZE];
    bytes_received = recvfrom(sockfd, buffer, MAX_PAYLOAD_SIZE, 0,
                              (struct sockaddr *) &sender_addr, (socklen_t *) &addr_len);
    if(strcmp(buffer, "STOP") == 0){
      printf("Fin de la réception de données\n");
      break;
    }
    if(fd == STDOUT){
      printf("Chaine reçue : %s\n", buffer);
    }
    else{
      bytes_written = write(fd, buffer, strlen(buffer));
      if(bytes_written < 0){
        perror("Erreur write");
        close(sockfd);
        close(fd);
        return -1;
      }
    }
  }


/*

  // Attente d'un message du sender
  int ret_wait = wait_for_client(sfd);
  if (ret_wait < 0){
    fprintf(stderr, "Erreur lors de l'attente d'un message\n");
    return -1;
  }

  // Reception d'un paquet du sender
  // Envoie d'un ACK

  pkt_t * nouveau = pkt_new();
  nouveau->type = 2;
*/

  close(sockfd);
  if(fd != 1){
    close(fd);
  }
  return 0;
}
