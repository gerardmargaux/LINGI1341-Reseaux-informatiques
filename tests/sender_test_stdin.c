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
#include <time.h>
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

  int err; // Variable pour error check

  int fd = STDIN; // File descriptor avec lequel on va lire les données
  int bytes_read; // Nombre de bytes lus sur l'entrée standard / le fichier source
  int bytes_sent; // Nombre de bytes envoyés au receiver
  int bytes_received; // Nombre de bytes reçus du receiver
  pkt_status_code err_code; // Variable pour error check avec les paquets

  uint8_t window = MAX_WINDOW_SIZE;
  uint8_t min_window = 0;
  uint8_t max_window = min_window + MAX_WINDOW_SIZE;

  uint8_t seqnum = 0;


  char* hostname = "::1";
  printf("Hostname : %s\n", hostname);
  char* port = "12345";
  printf("Port : %s\n", port);
  printf("Lecture sur l'entrée standard\n");

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


  pkt_t* packet = pkt_new();
  if(packet == NULL){
	  fprintf(stderr, "Erreur de création du paquet \n");
	  return -1;
  }

  ack_t* ack_received = ack_new();
  if(ack_received == NULL){
	  fprintf(stderr, "Erreur de création du paquet d'acquittement \n");
	  return -1;
  }

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

    char * payload_buf = (char*) malloc(MAX_PAYLOAD_SIZE*sizeof(char));
    if (payload_buf == NULL){
      fprintf(stderr, "Erreur malloc : payload_buf\n");
      return -1;
    }
    uint8_t * ack_buffer = (uint8_t*) malloc(12);
    if(ack_buffer == NULL){
      fprintf(stderr, "Erreur malloc : ack_buffer\n");
      return -1;
    }

    bytes_read = read(fd, payload_buf, MAX_PAYLOAD_SIZE);
    if(bytes_read == -1){
      perror("Erreur read");
      free(payload_buf);
      free(ack_buffer);
      return -1;
    }

    // Déconnexion
    else if(bytes_read == 0){

		printf("Déconnexion...\n");

		//bytes_sent = sendto(sockfd, "", 1, 0, servinfo->ai_addr, servinfo->ai_addrlen);

		uint8_t seqnum_end = seqnum + 1;

		err_code = pkt_set_seqnum(packet, seqnum);
		if(err_code != PKT_OK){
      		fprintf(stderr, "Erreur set_seqnum\n");
      		return -1;
      	}

		err_code = pkt_set_length(packet, 0);
		if(err_code != PKT_OK){
      		fprintf(stderr, "Erreur set_length\n");
      		return -1;
      	}

		uint8_t * buffer_encode = (uint8_t *)malloc(528);
        if (buffer_encode == NULL){
          fprintf(stderr, "Erreur malloc : buffer_encode\n");
          return -1;
        }

		size_t len_buffer_encode = 528;

        // Encodage du paquet a envoyer sur le reseau
      	err_code = pkt_encode(packet, buffer_encode, len_buffer_encode);
      	if(err_code != PKT_OK){
      		fprintf(stderr, "Erreur encode\n");
      		return -1;
      	}


		bytes_sent = sendto(sockfd, (void*) buffer_encode, 528, 0, servinfo->ai_addr, servinfo->ai_addrlen);

		struct sockaddr_in6 receiver_addr;
        memset(&receiver_addr, 0, sizeof(receiver_addr));
        socklen_t addr_len = sizeof(struct sockaddr);


        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        tv.tv_sec = 4;
        tv.tv_usec = 500000;

        sret = select(sockfd+1, &readfds, NULL, NULL, &tv);
        while(1){

		//struct timeval * rto = (struct timeval *)malloc(sizeof(struct timeval));

        while(sret == 0){
		  //gettimeofday (rto, NULL);
		  //int temps_actuel = rto->tv_sec * 1000 + rto->tv_usec;
		  //if (pkt_get_timestamp(packet) + tv.tv_sec * 1000 + tv.tv_usec < temps_actuel) {

          printf("Renvoi du paquet de déconnexion\n");
          bytes_sent = sendto(sockfd, (void *) buffer_encode, len_buffer_encode, 0, servinfo->ai_addr, servinfo->ai_addrlen);
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
        //free(rto);

          bytes_received = recvfrom(sockfd, ack_buffer, 528, 0, (struct sockaddr *) &receiver_addr, &addr_len);
          if(bytes_received < 0){
            perror("Erreur receive ACK");
            close(sockfd);
            close(fd);
            return -1;
          }


        err_code = ack_decode(ack_buffer, 12, ack_received);
        if(err_code != PKT_OK){
          fprintf(stderr, "Erreur decode\n");
          free(ack_received);
          close(sockfd);
          close(fd);
          return -1;
        }

        if(ack_received->seqnum == seqnum_end){
			printf("Reçu ACK de déconnexion.\n");
			break;
		}

		}


		free(ack_buffer);
		free(payload_buf);
		free(buffer_encode);

		break;


	}

	// Lecture de données
    else{

      if(fd == STDIN){ // Remplacement du caractère de linefeed par le caractère de fin de string
        int i;
        for(i=0; i<strlen(payload_buf); i++){
			if(*(payload_buf+i) == '\n'){
				*(payload_buf+i) = '\0';
				break;
			}
		}
      }
      if(*payload_buf != '\0'){ // Si on a effectivement écrit quelque chose
        err_code = pkt_set_payload(packet, payload_buf, strlen(payload_buf));
        if (err_code == -1){
			fprintf(stderr, "Erreur set payload dans la boucle \n");
			return -1;
		}

		strcpy(payload_buf, "");

        err_code = pkt_set_seqnum(packet, seqnum);
        if(err_code != PKT_OK){
          pkt_del(packet);
          close(sockfd);
          close(fd);
          return -1;
        }

		err_code = pkt_set_timestamp(packet);
      	if(err_code != PKT_OK){
      		fprintf(stderr, "Erreur set_timestamp\n");
      		return -1;
      	}


        err = seqnum_inc(&seqnum);
        if(err == -1){
          pkt_del(packet);
          close(sockfd);
          close(fd);
          return -1;
        }

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


		// Envoi du paquet sur le réseau
      	bytes_sent = sendto(sockfd, (void *) buffer_encode, len_buffer_encode, 0, servinfo->ai_addr, servinfo->ai_addrlen);
      	if(bytes_sent == -1){
      		perror("Erreur sendto packet");
      		return -1;
      	}


        struct sockaddr_in6 receiver_addr;
        memset(&receiver_addr, 0, sizeof(receiver_addr));
        socklen_t addr_len = sizeof(struct sockaddr);

        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        tv.tv_sec = 4;
        tv.tv_usec = 500000;

        sret = select(sockfd+1, &readfds, NULL, NULL, &tv);
        while(1){

		//struct timeval * rto = (struct timeval *)malloc(sizeof(struct timeval));

        while(sret == 0){
		  //gettimeofday (rto, NULL);
		  //int temps_actuel = rto->tv_sec * 1000 + rto->tv_usec;
		  //if (pkt_get_timestamp(packet) + tv.tv_sec * 1000 + tv.tv_usec < temps_actuel) {

          printf("Renvoi du paquet avec numéro de séquence %u\n", pkt_get_seqnum(packet));
          bytes_sent = sendto(sockfd, (void *) buffer_encode, len_buffer_encode, 0, servinfo->ai_addr, servinfo->ai_addrlen);
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
        //free(rto);

          bytes_received = recvfrom(sockfd, ack_buffer, 528, 0, (struct sockaddr *) &receiver_addr, &addr_len);
          if(bytes_received < 0){
            perror("Erreur receive ACK");
            close(sockfd);
            close(fd);
            return -1;
          }


        err_code = ack_decode(ack_buffer, 12, ack_received);
        if(err_code != PKT_OK){
          fprintf(stderr, "Erreur decode\n");
          free(ack_received);
          close(sockfd);
          close(fd);
          return -1;
        }

        if(ack_received->type == PTYPE_ACK){

        // On retire les paquets du buffer d'envoi
        uint8_t seqnum_ack_received = ack_received->seqnum;
          int i;
		  for(i = min_window; i <= seqnum_ack_received-1; i++){
            int err_retire_buffer = retire_buffer(buffer_envoi, i);
            if (err_retire_buffer == -1){
              fprintf(stderr, "Erreur retire buffer\n");
              free(ack_received);
              close(sockfd);
              close(fd);
              return -1;
            }
          window++;
          decale_window(&min_window, &max_window);
		  }
        break;
		}
      else {
        uint8_t seqnum = ack_received->seqnum;
        pkt_t* packet_renvoi = get_from_buffer(buffer_envoi, seqnum);
        uint8_t * buffer_encode2 = (uint8_t *)malloc(528);
        if (buffer_encode2 == NULL){
          fprintf(stderr, "Erreur malloc : buffer_encode\n");
          return -1;
        }


        err_code = pkt_encode(packet_renvoi, buffer_encode2, 528);
        if(err_code != PKT_OK){
      		fprintf(stderr, "Erreur encode\n");
      		return -1;
      	}


      	bytes_sent = sendto(sockfd, (void *) buffer_encode2, 528, 0, servinfo->ai_addr, servinfo->ai_addrlen);
      	if(bytes_sent == -1){
      		perror("Erreur sendto packet");
      		return -1;
      	}

      	sret = select(sockfd+1, &readfds, NULL, NULL, &tv);

      	while(sret == 0){
          printf("Renvoi du paquet avec numéro de séquence %u\n", pkt_get_seqnum(packet_renvoi));
          bytes_sent = sendto(sockfd, (void *) buffer_encode2, 528, 0, servinfo->ai_addr, servinfo->ai_addrlen);
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

        free(buffer_encode2);
      }

	}
        err = pkt_set_type(packet, PTYPE_DATA);
        memset(ack_received, 0, 12);
        packet->crc1 = 0;
        packet->crc2 = 0;
        packet->window = 0;
		packet->tr = 0;
		packet->length = 1;
		strcpy(packet->payload, "");
        ack_received->type = PTYPE_ACK;

      }
	}
  }

  free(ack_received);
  free(buffer_envoi);
  pkt_del(packet);


  freeaddrinfo(servinfo);
  close(sockfd);
  if(fd != 0){
    close(fd);
  }

  printf("Fin de la transmission.\n");
  return 0;
}
