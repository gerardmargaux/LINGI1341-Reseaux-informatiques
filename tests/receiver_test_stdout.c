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


struct __attribute__((__packed__)) pkt {
  char * payload;
  // Ne pas oublier d'inverser le sens des bits
  uint8_t window:5; // Encode sur 5 bits
  uint8_t tr:1; // Encode sur 1 bit
  uint8_t type:2; // Encode sur 2 bits
  uint8_t seqnum; // Encode sur 8 bits
  uint16_t length; // Encode sur 16 bits
  uint32_t timestamp; // Encode sur 32 bits (4 octets)
  uint32_t crc1; // Encode sur 32 bits (4 octets)
  uint32_t crc2; // Encode sur 32 bits (4 octets)
};

struct __attribute__((__packed__)) ack {
  uint8_t window:5; // Encode sur 5 bits
  uint8_t tr:1; // Encode sur 1 bit
  uint8_t type:2; // Encode sur 2 bits
  uint8_t seqnum; // Encode sur 8 bits
  uint16_t length; // Encode sur 16 bits
  uint32_t timestamp; // Encode sur 32 bits (4 octets)
  uint32_t crc1; // Encode sur 32 bits (4 octets)
};


/*
 * main : Fonction principale
 *
 */
int main(int argc, char *argv[]) {


  uint8_t window = MAX_WINDOW_SIZE;
  uint8_t min_window = 0;
  uint8_t max_window = min_window + MAX_WINDOW_SIZE;
  int err; // Variable pour error check

  pkt_status_code err_code; // Variable pour error check avec les paquets
  int fd = STDOUT; // File descriptor avec lequel on va écrire les données
  int bytes_received = 1; // Nombre de bytes reçus du sender
  //int bytes_written; // Nombre de bytes écrits à chaque itération
  int bytes_sent; // Nombre de bytes renvoyes au sender (ack)


  pkt_t **buffer_recept = (pkt_t**) calloc(window, sizeof(pkt_t*));
  if(buffer_recept == NULL){
    fprintf(stderr, "Erreur malloc\n");
    return -1;
  }


  pkt_t * packet_recv = pkt_new();
  ack_t * packet_ack = ack_new();

  printf("Ecriture sur la sortie standard\n");
  char* hostname = "::1";
  printf("Hostname : %s\n", hostname);
  char* port = "12345";
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

    struct sockaddr_in6 sender_addr, receiver_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in6);
    memset(&sender_addr, 0, sizeof(sender_addr));
    memset(&receiver_addr, 0, sizeof(receiver_addr));

	// Réception des données
    uint8_t* data_received = (uint8_t*) malloc(528);
    bytes_received = recvfrom(sockfd, data_received, 528, 0, (struct sockaddr *) &sender_addr, &addr_len);
    
    /*
    if(strcmp((const char*) data_received, "") == 0){
		break;
	}
	*/ 
		
		
	
    // Decodage du buffer recu sur le reseau
    const size_t len = 528;

    err_code = pkt_decode(data_received, len, packet_recv);
    
    free(data_received);
    
    if (err_code != PKT_OK){
      printf("Paquet ignoré\n");
    }
    else{
    
    if(pkt_get_length(packet_recv) == 0){
		printf("Déconnexion...\n");
		uint8_t seqnum_recv = pkt_get_seqnum(packet_recv);
        
        window = window - err;
        packet_ack->seqnum = seqnum_recv+1;
        packet_ack->window = window;
		
        uint8_t * buffer_encode = (uint8_t *)malloc(16*sizeof(uint8_t));
        if (buffer_encode == NULL){
          fprintf(stderr, "Erreur malloc : buffer_encode\n");
          return -1;
        }

        size_t len_buffer_encode = 16;

        // Encodage du paquet a envoyer sur le reseau
        err_code =  ack_encode(packet_ack, buffer_encode, len_buffer_encode);
        if(err_code != PKT_OK){
          free(packet_ack);
          close(sockfd);
          close(fd);
          return -1;
        }
        
        // Envoi du ack sur le reseau
        bytes_sent = sendto(sockfd, (void *)buffer_encode, len_buffer_encode, 0, (struct sockaddr *) &sender_addr, addr_len);
        if(bytes_sent < 0){
          perror("Erreur send ack");
          close(sockfd);
          close(fd);
          return -1;
        }
        
        free(buffer_encode);
        
        break;
        
	}
	
    uint8_t seqnum_recv = pkt_get_seqnum(packet_recv);

    // Teste si le numero de sequence est dans la fenetre
    int val = in_window(seqnum_recv, min_window, max_window);

    // Si le paquet reçu n'est pas dans la fenêtre de réception, on l'ignore
    if (val == -1){
      return -1;
    }
    else{

    // Si le paquet recu est tronque
    // On renvoie un paquet de type NACK au sender
    if (pkt_get_tr(packet_recv) == 1){
		
		printf("Paquet tronqué !\n");
		
		ack_t * packet_nack = ack_new();
		packet_nack->type = PTYPE_NACK;
		packet_nack->tr = 0;
		packet_nack->window = window;
        packet_nack->seqnum = seqnum_recv;
        packet_nack->length = 0;

        uint8_t * buffer_encode = (uint8_t *)malloc(12*sizeof(uint8_t));
        if (buffer_encode == NULL){
          fprintf(stderr, "Erreur malloc : buffer_encode\n");
          return -1;
        }

        size_t len_buffer_encode = 12;

        // Encodage du paquet a envoyer sur le reseau
        int return_code =  ack_encode(packet_nack, buffer_encode, len_buffer_encode);
        if(return_code != PKT_OK){
          free(packet_nack);
          close(sockfd);
          close(fd);
          return -1;
        }

        bytes_sent = sendto(sockfd, (void *)buffer_encode, len_buffer_encode, 0, (struct sockaddr *) &sender_addr, addr_len);
        if (bytes_sent == -1){
          free(packet_nack);
          free(buffer_encode);
          close(sockfd);
          close(fd);
          return -1;
        }
      }

      else { // Si le paquet recu n'est pas tronque
          
          // Ajout du buffer au buffer de reception
          if(buffer_plein(buffer_recept) == 0){
            ajout_buffer(packet_recv, buffer_recept, min_window);
            window--;
            err = write_buffer(fd, buffer_recept, &min_window, &max_window);
            if (err == -1){
              free(packet_ack);
              close(sockfd);
              close(fd);
              return -1;
            }
           window = window - err;
            
           packet_ack->seqnum = seqnum_recv+1;
            

          packet_ack->window = window;

		
        uint8_t * buffer_encode = (uint8_t *)malloc(16*sizeof(uint8_t));
        if (buffer_encode == NULL){
          fprintf(stderr, "Erreur malloc : buffer_encode\n");
          return -1;
        }

        size_t len_buffer_encode = 16;

        // Encodage du paquet a envoyer sur le reseau
        err_code =  ack_encode(packet_ack, buffer_encode, len_buffer_encode);
        if(err_code != PKT_OK){
          free(packet_ack);
          close(sockfd);
          close(fd);
          return -1;
        }
        
        // Envoi du ack sur le reseau
        bytes_sent = sendto(sockfd, (void *)buffer_encode, len_buffer_encode, 0, (struct sockaddr *) &sender_addr, addr_len);
        if(bytes_sent < 0){
          perror("Erreur send ack");
          close(sockfd);
          close(fd);
          return -1;
        }

        // Retrait du buffer encode du buffer de reception
        int err_retire_buffer = retire_buffer(buffer_recept, packet_ack->seqnum);
        if (err_retire_buffer == -1){
          fprintf(stderr, "Erreur retire buffer\n");
          close(sockfd);
          close(fd);
          return -1;
        }

        free(buffer_encode);
        strcpy(packet_recv->payload, "");
        memset(packet_ack, 0, 12);
        packet_ack->type = PTYPE_ACK;
      }

      else{ // Le buffer est plein
        printf("Le buffer est plein :/\n");
        break;
      }
    }
  }
}
}

  pkt_del(packet_recv);
  free(packet_ack);

  free(buffer_recept);

  close(sockfd);
  if(fd != 1){
    close(fd);
  }
  
  printf("Fin de la transmission.\n");
  return 0;

}
