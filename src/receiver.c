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
 #include <netdb.h>
 #include <arpa/inet.h>
 #include <netinet/in.h>
 #include <sys/select.h>
 #include <sys/time.h>
 #include <unistd.h>
 #include <getopt.h>
 #include <ctype.h>
 #include <zlib.h>

#define STD_IN 0
#define STD_OUT 1


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


  // Prise en compte des arguments avec getopt()
  extern char* optarg;
  extern int optind, opterr, optopt;
  char* optstring = "f:";

  char c = (char) getopt(argc, argv, optstring);
  if(c == '?'){
    fprintf(stderr, "Option inconnue.\n");
    fprintf(stderr, "Ecriture sur la sortie standard.\n");
  }
  else if(c == -1){
    printf("Ecriture sur la sortie standard.\n");
  }
  else if(c == 'f'){
    char* filename = optarg;
    printf("Ecriture dans le fichier %s\n", filename);
  }

  char* hostname = argv[optind];
  printf("Hostname : %s\n", hostname);
  char* port = argv[optind+1];
  printf("Port : %s\n", port);

  // Creation socket
  int sfd = socket(AF_INET6, SOCK_DGRAM, 0);
  if (sfd < 0){
    fprintf(stderr, "Erreur de creation du socket\n");
    return -1;
  }

  // Attente d'un message du sender
  int ret_wait = wait_for_client(sfd);
  if (ret_wait < 0){
    fprintf(stderr, "Erreur lors de l'attente d'un message\n");
    return -1;
  }

  // Reception d'un paquet du sender
  pkt_t * buffer = (pkt_t *)malloc(32*sizeof(pkt_t));
  if (buffer == NULL){
    fprintf(stderr, "ERROR : fonction malloc()\n");
    return -1
  }

  // Envoi d'un ACK
  pkt_t * nouveau = pkt_new();
  nouveau->type = 2;


  return 0;
}
