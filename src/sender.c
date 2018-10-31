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

  int err; // Variable pour error check

  // Vérification du nombre d'arguments
  err = arg_check(argc, 3, 5);
  if(err == -1){
    return -1;
  }


  int fd = STDIN; // File descriptor avec lequel on va lire les données
  int bytes_read; // Nombre de bytes lus sur l'entrée standard / le fichier source
  int bytes_sent; // Nombre de bytes envoyés au receiver
  int bytes_received; // Nombre de bytes reçus du receiver
  pkt_status_code err_code; // Variable pour error check avec les paquets

  uint8_t window = MAX_WINDOW_SIZE;
  uint8_t min_window = 0;
  uint8_t max_window = min_window + MAX_WINDOW_SIZE;

  uint8_t seqnum = 0;


  // Prise en compte des arguments en ligne de commande
  int a = 1;
  char* hostname;
  int host_set = 0;
  char* port;
  for(; a < argc; a++){
    if(strcmp(argv[a], "-f") == 0){
      a++;
      printf("Lecture dans le fichier %s\n", argv[a]);
      fd = open(argv[a], O_RDONLY);
      if(fd == -1){
        perror("Erreur open fichier source");
        return -1;
      }
    }
    else if(host_set == 0){
      hostname = argv[a];
      printf("Hostname : %s\n", hostname);
      host_set = 1;
    }
    else{
      port = argv[a];
      printf("Port : %s\n", port);
    }
  }


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
    return -1;
  }

  sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
  if(sockfd == -1){
    perror("Erreur socket");
    freeaddrinfo(servinfo);
    return -1;
  }
  socklen_t addr_len = sizeof(struct sockaddr_in6);
  struct sockaddr * addr_sendto = (struct sockaddr *) servinfo->ai_addr;


  pkt_t* packet = pkt_new();
  pkt_t* ack_received = pkt_ack_new();
  char* payload_buf = (char*) malloc(MAX_PAYLOAD_SIZE*sizeof(char));

  pkt_t ** buffer_envoi = (pkt_t **) malloc(MAX_WINDOW_SIZE*sizeof(pkt_t*));
  if (buffer_envoi == NULL){
    fprintf(stderr, "Erreur malloc : buffer_envoi\n");
    return -1;
  }

  int sret; // Variable de retour de la fonction select;
  struct timeval tv;
  fd_set readfds;

  while(1){

    while(in_window(seqnum, min_window, max_window) == -1){
      fprintf(stderr, "Numéro de séquence hors de la fenêtre d'envoi\n");
      seqnum_inc(&seqnum);
    }

    uint8_t * ack_buffer = (uint8_t*) malloc(12 * sizeof(uint8_t));
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
      bytes_sent = sendto(sockfd, "STOP", 5, 0, servinfo->ai_addr, addr_len);
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

        err_code = pkt_set_payload(packet, payload_buf, strlen(payload_buf));
        if(err_code != PKT_OK){
          pkt_del(packet);
          close(sockfd);
          close(fd);
          return -1;
        }
        memset(payload_buf, 0, 512);

        printf("Packet payload : %s\n", pkt_get_payload(packet));

        err_code = pkt_set_seqnum(packet, seqnum);
        if(err_code != PKT_OK){
          pkt_del(packet);
          close(sockfd);
          close(fd);
          return -1;
        }

        printf("Packet seqnum : %u\n", pkt_get_seqnum(packet));

        err = seqnum_inc(&seqnum);
        if(err == -1){
          pkt_del(packet);
          close(sockfd);
          close(fd);
          return -1;
        }

        printf("Packet seqnum : %u\n", pkt_get_seqnum(packet));

        window--;
        err_code = pkt_set_window(packet, window);
        if(err == -1){
          pkt_del(packet);
          close(sockfd);
          close(fd);
          return -1;
        }

        // Ajout du paquet au buffer d'envoi
      	ajout_buffer(packet, buffer_envoi, min_window);
        printf("Packet seqnum : %u\n", pkt_get_seqnum(*buffer_envoi));

        uint8_t * buffer_encode = (uint8_t *)malloc(528);
        if (buffer_encode == NULL){
          fprintf(stderr, "Erreur malloc : buffer_encode\n");
          return -1;
        }

        size_t len_buffer_encode = 528;


        // Encodage du paquet a envoyer sur le reseau
      	err_code = pkt_encode(packet, buffer_encode, 528);
      	if(err_code != PKT_OK){
      		fprintf(stderr, "Erreur encode\n");
      		return -1;
      	}

        // Envoi du packet sur le reseau
        // Affichage de l'adresse du receiver
        /*
        struct sockaddr_in6 * test = (struct sockaddr_in6 *) servinfo->ai_addr;
        char * addresse = (char*) malloc(1024);
        inet_ntop(AF_INET6, &(test->sin6_addr), addresse, 1024);
        printf("Addresse : %s\n", addresse);
        free(test);
        free(addresse);
        */

        bytes_sent = sendto(sockfd, (void *) buffer_encode, len_buffer_encode, 0, addr_sendto, addr_len);
      	if(bytes_sent == -1){
      		perror("Erreur sendto packet");
      		return -1;
      	}

        printf("Fin de l'envoi du packet\n");

        struct sockaddr_in6 receiver_addr;
        memset(&receiver_addr, 0, sizeof(receiver_addr));
        socklen_t addr_len = sizeof(struct sockaddr);


        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        tv.tv_sec = 2;
        tv.tv_usec = 500000;

        sret = select(sockfd+1, &readfds, NULL, NULL, &tv);
        while(1){

        while(sret == 0){
          printf("Renvoi du paquet avec numéro de séquence %u\n", pkt_get_seqnum(packet));
          bytes_sent = sendto(sockfd, (void *) buffer_encode, len_buffer_encode, 0, (struct sockaddr *) servinfo->ai_addr, addr_len);
          if(bytes_sent == -1){
            perror("Erreur sendto packet");
            free(payload_buf);
            pkt_del(packet);
            close(sockfd);
            close(fd);
            return -1;
          }

          FD_ZERO(&readfds);
          FD_SET(sockfd, &readfds);

          tv.tv_sec = 2;
          tv.tv_usec = 500000;

          sret = select(sockfd+1, &readfds, NULL, NULL, &tv);
        }

          bytes_received = recvfrom(sockfd, ack_buffer, 16, 0, (struct sockaddr *) &receiver_addr, &addr_len);
          if(bytes_received < 0){
            perror("Erreur receive ACK");
            close(sockfd);
            close(fd);
            return -1;
          }



        err_code = pkt_decode(ack_buffer, 16, ack_received);
        free(ack_buffer);
        if(err_code != PKT_OK){
          fprintf(stderr, "Erreur decode\n");
          pkt_del(ack_received);
          close(sockfd);
          close(fd);
          return -1;
        }

        if(pkt_get_type(ack_received) == PTYPE_ACK){

        printf("Reçu ACK pour paquet %d\n", pkt_get_seqnum(ack_received));

        // On retire les pquets du buffer d'envoi
        uint8_t seqnum_ack_received = pkt_get_seqnum(ack_received);
          int i;
		      for(i = min_window; i <= seqnum_ack_received; i++){
            int err_retire_buffer = retire_buffer(buffer_envoi, i);
            printf("Packet %u retiré du buffer d'envoi\n", i);
            printf("Return retire buffer : %u\n", err_retire_buffer);
            if (err_retire_buffer == -1){
              fprintf(stderr, "Erreur retire buffer\n");
              pkt_del(ack_received);
              close(sockfd);
              close(fd);
              return -1;
            }
        window++;
        printf("Window : %u\n", window);
        decale_window(&min_window, &max_window);
        printf("Min Window : %u\n", min_window);
        printf("Max Window : %u\n", max_window);
        }
        break;
      }
      else {
        fprintf(stderr, "Erreur : le paquet recu n'est pas un acquittement\n");
        pkt_del(ack_received);
        close(sockfd);
        close(fd);
        return -1;
      }
    }
        printf("Objet ack_received 1 : %p \n", ack_received);
        printf("Objet packet 1: %p \n", packet);
        printf("Objet buffer_encode : %p \n", buffer_encode);
        printf("Objet ack_buffer : %p \n", ack_buffer);
        memset(packet, 0, 528);
        err = pkt_set_type(packet, PTYPE_DATA);
        memset(ack_received, 0, 12);
        err = pkt_set_type(ack_received, PTYPE_ACK);
        free(buffer_encode); // Sinon probleme de memoire
      }
    }
  }

  pkt_del(packet);
  pkt_del(ack_received);
  free(payload_buf);
  free(buffer_envoi);

  close(sockfd);
  if(fd != 0){
    close(fd);
  }
  return 0;
}
