// @Titre : Projet LINGI1341 : Réseaux informatiques
// @Auteurs : Francois DE KEERSMAEKER (7367 1600) & Margaux GERARD (7659 1600)
// @Date : 22 octobre 2018

/*
 * Receiver : programme qui reçoit des paquets de données depuis le réseau et
 *            répond par un acknowledgement au sender.
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
  pkt_status_code err_code; // Variable pour error check
  int fd = STDOUT; // File descriptor avec lequel on va écrire les données
  int bytes_received = 1; // Nombre de bytes reçus du sender
  int bytes_written; // Nombre de bytes écrits à chaque itération
  //pkt_status_code err_code; // Variable pour error check avec les paquets
  int bytes_sent; // Nombre de bytes renvoyes au sender (ack)
  uint8_t ** buffer_recept = (uint8_t **)malloc(1024*sizeof(uint8_t)); // Buffer de buffer de reception des paquets
  if (buffer_recept == NULL){
    fprintf(stderr, "Erreur malloc : buffer_recept\n");
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
    bytes_received = recvfrom(sockfd, buffer, MAX_PAYLOAD_SIZE, 0,(struct sockaddr *) &sender_addr, (socklen_t *) &addr_len);
    if (bytes_received == -1){
      free(buffer);
      close(sockfd);
      close(fd);
      return -1;
    }
    if(strcmp(buffer, "STOP") == 0){
      printf("Fin de la réception de données\n");
      break;
    }
    if(fd == STDOUT){
      printf("Chaine reçue : %s\n", buffer);
    }
    else{

      // Decodage du buffer recu sur le reseau
      const size_t len = sizeof(buffer);
      pkt_t * new_packet = pkt_new();

      err_code = pkt_decode((uint8_t *)buffer, len, new_packet);
      if (err_code != PKT_OK){
        pkt_del(new_packet);
        close(sockfd);
        close(fd);
        return -1;
      }

      // Si le paquet recu est tronque
      // On renvoie un paquet de type NACK au sender
      if (pkt_get_type(new_packet) == PTYPE_NACK){

        uint8_t seqnum = pkt_get_seqnum(new_packet);
        uint8_t window = 3;
        pkt_t * packet_nack = pkt_new();

        err_code = pkt_set_seqnum(packet_nack, seqnum);
        if (err_code != PKT_OK){
          pkt_del(packet_nack);
          close(sockfd);
          close(fd);
          return -1;
        }

        err_code = pkt_set_type(packet_nack, PTYPE_NACK);
        if (err_code != PKT_OK){
          pkt_del(packet_nack);
          close(sockfd);
          close(fd);
          return -1;
        }

        err_code = pkt_set_window(packet_nack, window);
        if (err_code != PKT_OK){
          pkt_del(packet_nack);
          close(sockfd);
          close(fd);
          return -1;
        }

        uint8_t * buffer_encode = (uint8_t *)malloc(1024*sizeof(uint8_t));
        if (buffer_encode == NULL){
          fprintf(stderr, "Erreur malloc : buffer_encode\n");
          return -1;
        }

        size_t len_buffer_encode = sizeof(buffer_encode);

        // Encodage du paquet a envoyer sur le reseau
        int return_code =  pkt_encode(packet_nack, buffer_encode, len_buffer_encode);
        if(return_code != PKT_OK){
          pkt_del(packet_nack);
          close(sockfd);
          close(fd);
          return -1;
        }

        bytes_sent = sendto(sockfd, (void *)buffer_encode, len_buffer_encode, 0, servinfo->ai_addr, servinfo->ai_addrlen);
        if (bytes_sent == -1){
          pkt_del(packet_nack);
          free(buffer_encode);
          close(sockfd);
          close(fd);
          return -1;
        }
      }

      else { // Si le paquet recu n'est pas tronque

      // Ecriture du buffer recu dans un fichier ou sur la sortie standard
      bytes_written = write(fd, buffer, strlen(buffer));
      if(bytes_written < 0){
        perror("Erreur write");
        close(sockfd);
        close(fd);
        return -1;
      }

      uint8_t seqnum = pkt_get_seqnum(new_packet);
      uint8_t window = 3;
      uint8_t min_window = 2;
      uint8_t max_window = 5;

      // Teste si le numero de sequence est dans la fenetre
      int val = in_window(seqnum, min_window, max_window);
      if (val == -1 || val == 1){
        pkt_del(new_packet);
        close(sockfd);
        close(fd);
        return -1;
      }
      else {
        // Ajout du buffer au buffer de reception
        ajout_buffer((uint8_t *)buffer, buffer_recept);
        pkt_t * packet_ack = pkt_new();

        err_code = pkt_set_seqnum(packet_ack, seqnum);
        if (err_code != PKT_OK){
          pkt_del(packet_ack);
          close(sockfd);
          close(fd);
          return -1;
        }

        err_code = pkt_set_type(packet_ack, PTYPE_ACK);
        if (err_code != PKT_OK){
          pkt_del(packet_ack);
          close(sockfd);
          close(fd);
          return -1;
        }

        err_code = pkt_set_window(packet_ack, window);
        if (err_code != PKT_OK){
          pkt_del(packet_ack);
          close(sockfd);
          close(fd);
          return -1;
        }

        uint8_t * buffer_encode = (uint8_t *)malloc(1024*sizeof(uint8_t));
        if (buffer_encode == NULL){
          fprintf(stderr, "Erreur malloc : buffer_encode\n");
          return -1;
        }

        size_t len_buffer_encode = sizeof(buffer_encode);

        // Encodage du paquet a envoyer sur le reseau
        int return_code =  pkt_encode(packet_ack, buffer_encode, len_buffer_encode);
        if(return_code != PKT_OK){
          pkt_del(packet_ack);
          close(sockfd);
          close(fd);
          return -1;
        }

        // Envoi du packet sur le reseau
        bytes_sent = sendto(sockfd, (void *)buffer_encode, len_buffer_encode, 0, servinfo->ai_addr, servinfo->ai_addrlen);
        if (bytes_sent == -1){
          pkt_del(packet_ack);
          free(buffer_encode);
          close(sockfd);
          close(fd);
          return -1;
        }
        printf("Fin de l'envoi du packet\n");
        free(buffer_encode);
      }
    }
    }
  }

  close(sockfd);
  if(fd != 1){
    close(fd);
  }
  return 0;
}
