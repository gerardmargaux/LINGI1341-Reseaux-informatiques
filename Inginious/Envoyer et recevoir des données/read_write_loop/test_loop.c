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
#include <errno.h>
#include <sys/time.h>

#define STD_IN 0
#define STD_OUT 1


/* Loop reading a socket and printing to stdout,
 * while reading stdin and writing to the socket
 * @sfd: The socket file descriptor. It is both bound and connected.
 * @return: as soon as stdin signals EOF
 */
void read_write_loop(int sfd){

  //int err; // Variable pour error check
  int n_sent;
  int n_rcvd;

  // Initialisation des variables pour select()
  int sret;
  int numfds = sfd + 1;
  fd_set readfds;
  struct timeval timeout;

  // Initialisation des buffers
  char send_buf[] = "Framinem7";
  char * recv_buf = (char*) malloc(50*sizeof(char));
  if(recv_buf == NULL){
    fprintf(stderr, "ERROR malloc()\n");
    return;
  }



    FD_ZERO(&readfds);
    FD_SET(sfd, &readfds);

    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    sret = select(numfds, &readfds, NULL, NULL, &timeout);
    n_sent = send(sfd, send_buf, strlen(send_buf), 0);
    fprintf(stderr, "Données envoyées sur le serveur : %s\n", send_buf);
    if(n_sent == -1){
      perror("ERROR send()");
      return;
    }
    if(sret == -1){
      perror("ERROR select()");
      return;
    }
    else if(sret == 0){
      fprintf(stderr, "select() return value : %d\n", sret);
      fprintf(stderr, "Timed out.\n");
    }
    else if(sret == 1){
      fprintf(stderr, "select() return value : %d\n", sret);
      n_rcvd = recv(sfd, recv_buf, sizeof(recv_buf), 0);
      if(n_rcvd == -1){
        perror("ERROR recv()");
        free(recv_buf);
      }
      fprintf(stderr, "Données reçues du serveur : %s\n", recv_buf);
    }

    free(recv_buf);
    return;
}


// Teste la fonction read_write_loop
int main(int argc, char const *argv[]) {

  int err; // Variable pour error check
  struct addrinfo hints;
  struct addrinfo *servinfo;

  // addrinfo
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET6;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = 0;

  err = getaddrinfo("::1", "12345", &hints, &servinfo);
  if(err != 0){
    fprintf(stderr, "ERROR getaddrinfo() : %s\n", gai_strerror(err));
    return -1;
  }

  // socket
  int sfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
  if(sfd == -1){
    perror("ERROR socket()");
    return -1;
  }

  // bind
  err = bind(sfd, servinfo->ai_addr, servinfo->ai_addrlen);
  if(err == -1){
    perror("ERROR bind()");
    return -1;
  }

  // connect
  err = connect(sfd, servinfo->ai_addr, servinfo->ai_addrlen);
  if(err == -1){
    perror("ERROR connect()");
    return -1;
  }

  printf("Successful connection !\n");

  read_write_loop(sfd);

  close(sfd);
  return 0;
}
