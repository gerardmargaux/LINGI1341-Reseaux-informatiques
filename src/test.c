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

  size_t len = 528;
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

  err_code = pkt_encode(packet, buf, len);
  if(err_code != PKT_OK){
    pkt_del(packet);
    fprintf(stderr, "Erreur encode\n");
    return -1;
  }

  printf("Encode OK\n");

  uint8_t result8;
  uint16_t result16;
  uint32_t result32;

  memcpy(&result8, buf, 1);
  printf("1er byte : %u\n", result8);

  memcpy(&result8, buf+1, 1);
  printf("Seqnum : %u\n", result8);

  memcpy(&result16, buf+2, 2);
  printf("Longueur : %u\n", ntohs(result16));

  memcpy(&result32, buf+4, 4);
  printf("Timestamp : %u\n", ntohl(result32));

  memcpy(&result32, buf+8, 4);
  printf("CRC1 : %u\n", ntohl(result32));

  char* data_received = (char*) malloc(512*sizeof(char));
  if(data_received == NULL){
    pkt_del(packet);
    free(buf);
    return -1;
  }
  memcpy(data_received, buf+12, pkt_get_length(packet));
  printf("Payload : %s\n", data_received);

  memcpy(&result32, buf+12+pkt_get_length(packet), 4);
  printf("CRC2 : %u\n", ntohl(result32));


  pkt_del(packet);
  free(buf);

  return 0;
}
