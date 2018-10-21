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


int main(int argc, char const *argv[]) {

  int err;
  uint8_t seqnum = 0;
  uint8_t min_window = 0;
  uint8_t max_window = 31;
  pkt_status_code err_code;
  pkt_t *packet = pkt_new();
  printf("Paquet créé\n");
  err_code = pkt_set_seqnum(packet, seqnum);
  printf("Seqnum set\n");
  if(err_code != PKT_OK){
    pkt_del(packet);
    fprintf(stderr, "Erreur set_seqnum\n");
    return -1;
  }

  err = seqnum_inc(&seqnum);
  err = seqnum_inc(&seqnum);
  err = seqnum_inc(&seqnum);
  if(err == -1){
    pkt_del(packet);
    fprintf(stderr, "Erreur seqnum_inc\n");
    return -1;
  }

  err_code = pkt_set_payload(packet, "Bonjour", 8);
  printf("Payload set\n");
  if(err_code != PKT_OK){
    pkt_del(packet);
    fprintf(stderr, "Erreur set_payload\n");
    return -1;
  }

  pkt_t** big_buffer = (pkt_t**) malloc(31*sizeof(pkt_t*));

  ajout_buffer(packet, big_buffer, min_window);

  printf("Paquet en 1e place du big buffer : %s\n", pkt_get_payload(*(big_buffer)));

  pkt_t *packet2 = pkt_new();
  err_code = pkt_set_seqnum(packet2, seqnum);
  if(err_code != PKT_OK){
    pkt_del(packet);
    fprintf(stderr, "Erreur set_seqnum\n");
    return -1;
  }


  err = seqnum_inc(&seqnum);
  if(err == -1){
    pkt_del(packet);
    fprintf(stderr, "Erreur seqnum_inc\n");
    return -1;
  }

  err_code = pkt_set_payload(packet2, "Ca va ?", 8);
  printf("Payload set\n");
  if(err_code != PKT_OK){
    pkt_del(packet);
    fprintf(stderr, "Erreur set_payload\n");
    return -1;
  }

  ajout_buffer(packet2, big_buffer, min_window);

  printf("Paquet en 3e place du big buffer : %s\n", pkt_get_payload(*(big_buffer+3)));

  write_buffer(STDOUT, big_buffer);

  pkt_del(packet);

  return 0;
}
