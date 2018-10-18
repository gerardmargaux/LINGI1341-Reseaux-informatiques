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

  const char *data = argv[1];
  pkt_status_code err_code;
  pkt_t *packet = pkt_new();
  printf("Paquet créé\n");
  err_code = pkt_set_seqnum(packet, 142);
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

  pkt_t *packet2 = pkt_new();

  err_code = pkt_decode(buf, len, packet2);
  if(err_code != PKT_OK){
    pkt_del(packet);
    fprintf(stderr, "Erreur decode\n");
    return -1;
  }

  printf("Decode OK\n");

  printf("Type du paquet : %u\n", pkt_get_type(packet2));
  printf("Tr du paquet : %u\n", pkt_get_tr(packet2));
  printf("Window du paquet : %u\n", pkt_get_window(packet2));
  printf("Seqnum du paquet : %u\n", pkt_get_seqnum(packet2));
  printf("Length du paquet : %u\n", pkt_get_length(packet2));
  printf("Timestamp du paquet : %u\n", pkt_get_timestamp(packet2));
  printf("CRC1 du paquet : %u\n", pkt_get_crc1(packet2));
  printf("Payload du paquet : %s\n", pkt_get_payload(packet2));
  printf("CRC2 du paquet : %u\n", pkt_get_crc2(packet2));

  pkt_del(packet);
  free(buf);

  return 0;
}
