// @Titre : Projet LINGI1341 : Réseaux informatiques
// @Auteurs : Francois DE KEERSMAEKER (7367 1600) & Margaux GERARD (7659 1600)
// @Date : 22 octobre 2018

/*
 * Sender : programme qui envoie des paquets de données sur le réseau.
 *
 */

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
 #include <getopt.h>
 #include <ctype.h>

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
    fprintf(stderr, "Lecture sur la sortie standard.\n");
  }
  else if(c == -1){
    printf("Lecture sur la sortie standard.\n");
  }
  else if(c == 'f'){
    char* filename = optarg;
    printf("Lecture dans le fichier %s\n", filename);
  }

  char* hostname = argv[optind];
  printf("Hostname : %s\n", hostname);
  char* port = argv[optind+1];
  printf("Port : %s\n", port);


  return 0;
}
