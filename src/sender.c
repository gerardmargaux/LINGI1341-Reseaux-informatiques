// @Titre : Projet LINGI1341 : Réseaux informatiques
// @Auteurs : Francois DE KEERSMAEKER (7367 1600) & Margaux GERARD (7659 1600)
// @Date : 22 octobre 2018

/*
 * Sender : programme qui envoie des paquets de données sur le réseau.
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


  int fd = STDIN; // File descriptor avec lequel on va lire les données
  int bytes_read; // Nombre de bytes lus à chaque itération
  pkt_status_code err_code; // Variable pour error check

  pkt_t* packet = pkt_new();


  // Prise en compte des arguments avec getopt()
  extern char* optarg;
  extern int optind, opterr, optopt;
  char* optstring = "f:";

  char c = (char) getopt(argc, argv, optstring);
  if(c == '?'){
    fprintf(stderr, "Option inconnue.\n");
    fprintf(stderr, "Lecture sur l'entrée standard.\n");
  }
  else if(c == -1){ // Lecture à partir de l'entrée standard
    printf("Lecture sur l'entrée standard.\n");
  }
  else if(c == 'f'){ // Lecture à partir d'un fichier
    char* filename = optarg;
    printf("Lecture dans le fichier %s\n", filename);
    fd = open(filename, O_RDONLY);
    if(fd == -1){
      perror("Erreur open fichier source");
      return -1;
    }
  }

  char* hostname = argv[optind];
  printf("Hostname : %s\n", hostname);
  char* port = argv[optind+1];
  printf("Port : %s\n", port);

while(1){

  char* payload_buf = (char*) malloc(MAX_PAYLOAD_SIZE*sizeof(char));
  if(payload_buf == NULL){
    fprintf(stderr, "Erreur malloc : payload_buf\n");
    return -1;
  }

  bytes_read = read(fd, payload_buf, MAX_PAYLOAD_SIZE);
  if(bytes_read == -1){
    perror("Erreur read");
    free(payload_buf);
    return -1;
  }
  else if(bytes_read == 0){
    printf("Lecture finie.\n");
    free(payload_buf);
    break;
  }
  else{
    if(fd == STDIN){
      *(payload_buf+strlen(payload_buf)-1) = '\0';
    }
    printf("Chaine lue : %s\n", payload_buf);
    err_code = pkt_set_payload(packet, (const char*) payload_buf,
    (const uint16_t) strlen(payload_buf));
    if(err_code != PKT_OK){
      fprintf(stderr, "Erreur set payload\n");
      free(payload_buf);
      pkt_del(packet);
      return -1;
    }
    printf("Données encodées dans le paquet : %s\n", pkt_get_payload(packet));
    free(payload_buf);
  }
}

  pkt_del(packet);
  return 0;
}
