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


  int err; // Variable pour error check
  int fd = STDIN; // File descriptor avec lequel on va lire les données
  int bytes_read; // Nombre de bytes lus sur l'entrée standard / le fichier source
  int bytes_sent; // Nombre de bytes envoyés au receiver
  int bytes_received; // Nombre de bytes reçus du receiver
  pkt_status_code err_code; // Variable pour error check avec les paquets

  int window = 4;
  int seqnum = 142;

  struct sockaddr_in6 receiver_addr;
  memset(&receiver_addr, 0, sizeof(receiver_addr));
  socklen_t addr_len = sizeof(struct sockaddr);



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


  // Création du socket
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


  freeaddrinfo(servinfo);


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
      bytes_sent = sendto(sockfd, "STOP", 5, 0, servinfo->ai_addr, servinfo->ai_addrlen);
      printf("Fin de l'envoi de données.\n");
      free(payload_buf);
      break;
    }
    else{
      if(fd == STDIN){ // Remplacement du caractère de linefeed par le caractère de fin de string
        *(payload_buf+strlen(payload_buf)-1) = '\0';
      }
      if(*payload_buf != '\0'){ // Si on a effectivement écrit quelque chose
        printf("Chaine lue : %s\n", payload_buf);


        pkt_t* packet = pkt_new();
        err_code = pkt_set_payload(packet, payload_buf, strlen(payload_buf));
        if(err_code != PKT_OK){
          pkt_del(packet);
          close(sockfd);
          close(fd);
          return -1;
        }

        err_code = pkt_set_seqnum(packet, seqnum);
        if(err_code != PKT_OK){
          pkt_del(packet);
          close(sockfd);
          close(fd);
          return -1;
        }

        err = seqnum_inc(&seqnum);
        if(err == -1){
          pkt_del(packet);
          close(sockfd);
          close(fd);
          return -1;
        }

        err_code = pkt_set_window(packet, window);
        if(err == -1){
          pkt_del(packet);
          close(sockfd);
          close(fd);
          return -1;
        }

        uint8_t * buffer_encode = (uint8_t *)malloc(528);
        if (buffer_encode == NULL){
          fprintf(stderr, "Erreur malloc : buffer_encode\n");
          return -1;
        }

        size_t len_buffer_encode = 528;

        // Encodage du paquet a envoyer sur le reseau
        err_code =  pkt_encode(packet, buffer_encode, len_buffer_encode);
        if(err_code != PKT_OK){
          fprintf(stderr, "Erreur encode\n");
          pkt_del(packet);
          close(sockfd);
          close(fd);
          return -1;
        }


        // Envoi du packet sur le reseau
        bytes_sent = sendto(sockfd, (void *) buffer_encode, len_buffer_encode, 0, servinfo->ai_addr, servinfo->ai_addrlen);
        if(bytes_sent == -1){
          perror("Erreur sendto packet");
          free(payload_buf);
          pkt_del(packet);
          close(sockfd);
          close(fd);
          return -1;
        }
        printf("Fin de l'envoi du packet\n");
        free(payload_buf);
        pkt_del(packet);



        uint8_t* ack_buffer = (uint8_t*) malloc(16);
        printf("Test 1\n");
        bytes_received = recvfrom(sockfd, ack_buffer, 16, 0, (struct sockaddr *) &receiver_addr, &addr_len);
        if(bytes_received < 0){
          perror("Erreur receive ACK");
          close(sockfd);
          close(fd);
          return -1;
        }

        pkt_t* ack_received = pkt_ack_new();

        err_code = pkt_decode(ack_buffer, 16, ack_received);
        free(ack_buffer);
        if(err_code != PKT_OK){
          fprintf(stderr, "Erreur decode\n");
          pkt_del(ack_received);
          close(sockfd);
          close(fd);
          return -1;
        }

        printf("Reçu ACK pour paquet %d\n", pkt_get_seqnum(ack_received));

        free(ack_received);

      }
    }
  }

  close(sockfd);
  if(fd != 0){
    close(fd);
  }
  return 0;
}
