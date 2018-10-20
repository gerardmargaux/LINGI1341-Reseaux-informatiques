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

  uint8_t seqnum = 142;
  const char *data = argv[1];
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

  err_code = pkt_set_payload(packet, data, strlen(data));
  printf("Payload set\n");
  if(err_code != PKT_OK){
    pkt_del(packet);
    fprintf(stderr, "Erreur set_payload\n");
    return -1;
  }

  uint8_t *buf = (uint8_t*) malloc(528);
  if(buf == NULL){
    pkt_del(packet);
    fprintf(stderr, "Erreur malloc\n");
    return -1;
  }

  size_t len = 16 + pkt_get_length(packet);
  err_code = pkt_encode(packet, buf, len);
  if(err_code != PKT_OK){
    pkt_del(packet);
    fprintf(stderr, "Erreur encode\n");
    return -1;
  }

  printf("Encode OK\n");

  uint8_t compare;
  memcpy(&compare, buf+1, 1);
  printf("Compare : %u\n", compare);
  if(compare == seqnum){
    printf("Compare = seqnum\n");
  }

  uint8_t** big_buffer = (uint8_t**) malloc(MAX_WINDOW_SIZE*sizeof(uint8_t*));

  printf("Premiere place du big buffer : %s\n", *big_buffer);

  int err = ajout_buffer(buf, big_buffer);
  if(err != 0){
    fprintf(stderr, "Erreur ajout_buffer\n");
    pkt_del(packet);
    free(buf);
    return -1;
  }

  char payload[512];
  memcpy(payload, (*(big_buffer))+12, pkt_get_length(packet));

  printf("Premiere place du big buffer : %s\n", payload);
  memset(payload, 0, 512);

  uint8_t* packet2 = get_from_buffer(big_buffer, pkt_get_seqnum(packet));
  memcpy(payload, packet2+12, pkt_get_length(packet));
  printf("Packet récupéré : %s\n", payload);
  memset(payload, 0, 512);

  memcpy(payload, (*(big_buffer))+12, pkt_get_length(packet));
  printf("Premiere place du big buffer : %s\n", payload);





  pkt_del(packet);
  free(buf);

  return 0;
}
