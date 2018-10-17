#include "packet_interface.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <arpa/inet.h>

struct __attribute__((__packed__)) pkt {
	char * payload;
  // Ne pas oublier d'inverser le sens des bits
  uint8_t window:5; // Encode sur 5 bits
  uint8_t tr:1; // Encode sur 1 bit
  ptypes_t type:2; // Encode sur 2 bits
  uint8_t seqnum; // Encode sur 8 bits
  uint16_t length; // Encode sur 16 bits
  uint32_t timestamp; // Encode sur 32 bits (4 octets)
  uint32_t crc1; // Encode sur 32 bits (4 octets)
  uint32_t crc2; // Encode sur 32 bits (4 octets)
};

pkt_t* pkt_new()
{
	pkt_t * new = (pkt_t *) malloc(sizeof(pkt_t));
  if (new == NULL){
    fprintf(stderr, "Erreur du malloc");
    return NULL;
  }
  new->window = 5;
  new->tr = 0;
  new->type = 01;
  new->seqnum = (uint8_t) 19;
  new->length = 2;
  new->timestamp = 1;
  new->crc1 = 4;
  new->crc2 = 4;
	new->payload =  (char *)malloc(512 * sizeof(char));
  if (new->payload == NULL){
    fprintf(stderr, "Erreur du malloc");
    return NULL;
  }
  return new;
}


/* Libere le pointeur vers la struct pkt, ainsi que toutes les
 * ressources associees*/
void pkt_del(pkt_t *pkt)
{
	if (pkt->type == PTYPE_DATA){
		free (pkt->payload);
	}
	free (pkt);
}

/*
 * Decode des donnees recues et cree une nouvelle structure pkt.
 * Le paquet recu est en network byte-order.
 * La fonction verifie que:
 * - Le CRC32 du header recu est le mÃªme que celui decode a la fin
 *   du header (en considerant le champ TR a 0)
 * - S'il est present, le CRC32 du payload recu est le meme que celui
 *   decode a la fin du payload
 * - Le type du paquet est valide
 * - La longueur du paquet et le champ TR sont valides et coherents
 *   avec le nombre d'octets recus.
 *
 * @data: L'ensemble d'octets constituant le paquet recu
 * @len: Le nombre de bytes recus
 * @pkt: Une struct pkt valide
 * @post: pkt est la representation du paquet recu
 *
 * @return: Un code indiquant si l'operation a reussi ou representant
 *         l'erreur rencontree.
 */
 pkt_status_code pkt_decode(const char *data, const size_t len, pkt_t *pkt)
 {
   if (len == 0){ // Le paquet est incoherent
    return E_UNCONSISTENT;
   }
   else if (len < 4){ // Il n'y a pas de header car il est encode sur 4 bytes
    return E_NOHEADER;
   }
	 else {
		uint16_t length = ntohs(pkt_get_length(pkt));
		size_t size_len = ntohs(pkt_get_length(pkt));
		ptypes_t type = pkt_get_type(pkt);
		unsigned char variable_copy;

   // On copie le header
	 	memcpy(&variable_copy, data, 1);
		pkt->type = variable_copy>>6 & 0b00000011;
		pkt->tr = variable_copy>>7 & 0b00000001;
		pkt->window = variable_copy>>1 & 0b00011111;
		pkt->length = ntohs(pkt->length);

   memcpy(&pkt->seqnum, (data+1), sizeof(uint8_t)); // Copie de seqnum
	 memcpy(&pkt->length, (data+2), sizeof(uint16_t)); // Copie de length
	 memcpy(&pkt->timestamp, (data+4), sizeof(uint32_t)); // Copie de timestamp

	 if(type != PTYPE_DATA && type != PTYPE_NACK && type != PTYPE_ACK){
 			return E_TYPE;
 	}

 	// on regarde la longueur du payload et si le paquet est tronqué
 	if (pkt_get_length(pkt) == 0){
 		return E_LENGTH;
 	}
	if(pkt_get_tr(pkt) != 0){
		return E_TR;
	}


   uLong new_crc1 = crc32(0L, Z_NULL, 0);
   new_crc1 = crc32(new_crc1,(const Bytef*) data, 8);
	 /*if (crc1 != new_crc1){
			 return E_CRC;
	 }*/
	 // On decode le crc1
	 memcpy(&pkt->crc1, (data+8), sizeof(uint32_t));

   // On decode le payload
	 memcpy(&pkt->payload, (data+12), size_len);

	 // On decode le crc2
   uLong new_crc2 = crc32(0L, Z_NULL, 0);
   new_crc2 = crc32(new_crc2,(const Bytef *) data+12, size_len);
   /* if (crc2 != new_crc2){ // Si le crc2 n'est pas verifie
     return E_CRC;
   } */
	 memcpy(&pkt->crc2, (data+12+size_len), sizeof(uint32_t));

}
printf("Fin de la fonction decode\n");
 return PKT_OK;
}
/*
 * Encode une struct pkt dans un buffer, pret a etre envoye sur le reseau
 * (c-a-d en network byte-order), incluant le CRC32 du header et
 * eventuellement le CRC32 du payload si celui-ci est non nul.
 *
 * @pkt: La structure a encoder
 * @buf: Le buffer dans lequel la structure sera encodee
 * @len: La taille disponible dans le buffer
 * @len-POST: Le nombre de d'octets ecrit dans le buffer
 * @return: Un code indiquant si l'operation a reussi ou E_NOMEM si
 *         le buffer est trop petit.
 */
pkt_status_code pkt_encode(const pkt_t* pkt, char *buf, size_t *len)
 {
  // Gerer le header
  uint8_t window = pkt_get_window(pkt);
  ptypes_t type = pkt_get_type(pkt);
  uint8_t tr = pkt_get_tr(pkt);
  uint8_t seqnum = pkt_get_seqnum(pkt);
	uint32_t timestamp = pkt_get_timestamp(pkt);
  // window + type + tr = 1 byte
  //seqnum =  1 byte
  uint16_t length = htons(pkt_get_length(pkt)); // 2 bytes
	size_t size_len = htons(pkt_get_length(pkt));
  // + 4 bytes timestamp + 4 bytes crc1 = 12 bytes

	if(type != PTYPE_DATA && type != PTYPE_NACK && type != PTYPE_ACK){
		 return E_TYPE;
  }
	if(size_len > *len-12){ // Longueur du paquet - header
 	 return E_LENGTH;
  }
	if(MAX_WINDOW_SIZE < window){
		return E_WINDOW;
	}

	if (MAX_PAYLOAD_SIZE < size_len){
		return E_LENGTH;
	}
	if (size_len == 0){
		length = 0;
	}

  // On encode le header
  uint8_t premier_byte = type<<6 | tr<<5; // premier byte = 0110 0000
	//printf("J'affiche le premier byte %hhu\n", premier_byte);
  buf[0] = premier_byte | window; // Pour completer le byte

	memcpy(buf + sizeof(uint8_t), &seqnum, sizeof(uint8_t)); // seqnum
  memcpy(buf + sizeof(uint16_t), &length, sizeof(uint16_t)); // length
	memcpy(buf + sizeof(uint32_t), &timestamp, sizeof(uint32_t)); // timestamp

  // Gerer les CRC
  uLong crc1 = crc32(0L, Z_NULL, 0);
  crc1 = htonl(crc32(crc1,(const Bytef *) buf, 8));

  // On encode le crc1
	memcpy(buf+8, &crc1, 4);

	// On encode le payload
	if (length > 0){
		memcpy(buf+12, pkt->payload, size_len);
	}

  if(tr == 0){ // Si le paquet n'est pas tronque --> crc2
		uLong crc2 = crc32(0L, Z_NULL, 0);
 	 	crc2 = htonl(crc32(crc2,(const Bytef *)buf+12, size_len));

 	 // On encode le crc2
	 memcpy(buf+size_len+12, &crc2, 4); // Le crc2 apres 12 bytes + la longueur du payload
 }
	*len = 16 + size_len;
	printf("Fin de la fonction encode\n");
  return PKT_OK;
 }

ptypes_t pkt_get_type  (const pkt_t * pkt)
{
	return pkt->type;
}

uint8_t  pkt_get_tr(const pkt_t * pkt)
{
	return pkt->tr;
}

uint8_t  pkt_get_window(const pkt_t * pkt)
{
	return pkt->window;
}

uint8_t  pkt_get_seqnum(const pkt_t * pkt)
{
	return pkt->seqnum;
}

uint16_t pkt_get_length(const pkt_t * pkt)
{
	if (pkt == NULL){
		return 0;
	}
	return pkt->length;
}

uint32_t pkt_get_timestamp   (const pkt_t * pkt)
{
	return pkt->timestamp;
}

uint32_t pkt_get_crc1   (const pkt_t * pkt)
{
	return pkt->crc1;
}

uint32_t pkt_get_crc2   (const pkt_t * pkt)
{
	return pkt->crc2;
}

const char* pkt_get_payload(const pkt_t* pkt)
{
    return pkt->payload;
}

pkt_status_code pkt_set_type(pkt_t *pkt, const ptypes_t type)
{
	if(type == 1 || type == 2 || type == 3){
    pkt->type = type;
    return PKT_OK;
  }
  return E_TYPE;
}

pkt_status_code pkt_set_tr(pkt_t *pkt, const uint8_t tr)
{
	if (tr != 0 && tr != 1){
    return E_TR;
  }
  pkt->tr = tr;
  return PKT_OK;
}

pkt_status_code pkt_set_window(pkt_t *pkt, const uint8_t window)
{
	if (window > MAX_WINDOW_SIZE){
    return E_WINDOW;
  }
  pkt->window = window;
  return PKT_OK;
}

pkt_status_code pkt_set_seqnum(pkt_t *pkt, const uint8_t seqnum)
{
	pkt->seqnum = seqnum;
  return PKT_OK;
}

pkt_status_code pkt_set_length(pkt_t *pkt, const uint16_t length)
{
	if (length > MAX_PAYLOAD_SIZE){
    return E_LENGTH;
  }
  pkt->length = length;
  return PKT_OK;
}

pkt_status_code pkt_set_timestamp(pkt_t *pkt, const uint32_t timestamp)
{
	pkt->timestamp = timestamp;
  return PKT_OK;
}

pkt_status_code pkt_set_crc1(pkt_t *pkt, const uint32_t crc1)
{
	pkt->crc1 = crc1;
  return PKT_OK;
}

pkt_status_code pkt_set_crc2(pkt_t *pkt, const uint32_t crc2)
{
	pkt->crc2 = crc2;
  return PKT_OK;
}

pkt_status_code pkt_set_payload(pkt_t *pkt, const char *data, const uint16_t length)
{
	if (length > MAX_PAYLOAD_SIZE){
    return E_LENGTH;
  }
  pkt->payload = realloc(pkt->payload, length);
  memcpy(pkt->payload, data, length);
  return PKT_OK;
}

int main(int argc, char *argv[]) {
	// Test de la fonction encode
	pkt_t * nouveau = pkt_new();
	char buffer[12] = "s0al0uteThan";
	const size_t len = 4;
	//printf("J'affiche le buffer %s\n", buffer);
	size_t * count = (size_t *)calloc(1, sizeof(size_t)); // Nombre de places dispo dans le buffer
	if (count == NULL){
		return -1;
	}
	for(int i = 0; i<strlen(buffer); i++){
		if (buffer[i] == '0'){
			*count = *count + 1;
		}
	}
	pkt_status_code return_code_encode = pkt_encode(nouveau, buffer, count);
	printf("Return code encode = %u \n", return_code_encode);
	pkt_status_code ret_code_decode = pkt_decode(buffer, len, nouveau);
	printf("Return code decode = %u \n", ret_code_decode);
	return 0;
}
