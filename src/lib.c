#include "lib.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <getopt.h>
#include <ctype.h>
#include <zlib.h>

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

pkt_t* pkt_new()
{
	pkt_t * new = (pkt_t *) malloc(sizeof(pkt_t));
  if (new == NULL){
    fprintf(stderr, "Erreur du malloc");
    return NULL;
  }
  new->window = 0; // Par definition, on fait commencer la fenetre à 1
  new->tr = 0;
  new->type = 1;
  new->seqnum = 0;
  new->length = 0;
  new->timestamp = 0;
  new->crc1 = 0;
  new->crc2 = 0;
	new->payload = (char *)malloc(MAX_PAYLOAD_SIZE * sizeof(char));
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
    free (pkt->payload);
    free (pkt);
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
	return pkt->length;
}

uint32_t pkt_get_timestamp(const pkt_t * pkt)
{
	return pkt->timestamp;
}

uint32_t pkt_get_crc1(const pkt_t * pkt)
{
	return pkt->crc1;
}

uint32_t pkt_get_crc2(const pkt_t * pkt)
{
	return pkt->crc2;
}

const char* pkt_get_payload(const pkt_t * pkt)
{
	if ((pkt->length) <= 0){
    return NULL;
  }
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
	if(seqnum > 255 || seqnum < 0){
		return E_SEQNUM;
	}
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
  memcpy(pkt->payload, data, length+1);
	pkt->length = length;
  return PKT_OK;
}

/*
 * Crée un paquet et initialise tous ses champs avec les arguments de la fonction
 */
pkt_t* pkt_init(ptypes_t type, uint8_t tr, uint8_t window, uint8_t seqnum,
								uint16_t length, uint32_t timestamp, uint32_t crc1, uint32_t crc2,
								const char* payload)
{
	pkt_status_code err_code;
	pkt_t *packet = pkt_new();

	err_code = pkt_set_payload(packet, payload, strlen(payload));
	if(err_code != PKT_OK){
		return NULL;
	}

	err_code = pkt_set_type(packet, type);
	if(err_code != PKT_OK){
		pkt_del(packet);
		return NULL;
	}
	err_code = pkt_set_tr(packet, tr);
	if(err_code != PKT_OK){
		pkt_del(packet);
		return NULL;
	}
	err_code = pkt_set_window(packet, window);
	if(err_code != PKT_OK){
		pkt_del(packet);
		return NULL;
	}
	err_code = pkt_set_seqnum(packet, seqnum);
	if(err_code != PKT_OK){
		pkt_del(packet);
		return NULL;
	}
	err_code = pkt_set_length(packet, length);
	if(err_code != PKT_OK){
		pkt_del(packet);
		return NULL;
	}
	err_code = pkt_set_timestamp(packet, timestamp);

	err_code = pkt_set_crc1(packet, crc1);

	err_code = pkt_set_crc2(packet, crc2);

	return packet;
}


/*
 * Decode des donnees recues et cree une nouvelle structure pkt.
 * Le paquet recu est en network byte-order.
 * La fonction verifie que:
 * - Le CRC32 du header recu est le même que celui decode a la fin
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
pkt_status_code pkt_decode(uint8_t *data, const size_t len, pkt_t *pkt){

	if (len == 0){ // Le paquet est incoherent
  	return E_UNCONSISTENT;
  }
  else if (len < 12){ // Il n'y a pas de header car il est encode sur 12 bytes
    return E_NOHEADER;
  }

	// Initialisation des variables
	ptypes_t type;
	uint8_t tr;
	uint8_t window;
	uint8_t seqnum;
	uint16_t length;
	uint32_t timestamp;
	uint32_t crc1_recv;
	uint32_t crc2_recv;

	// Premier byte : type, tr, window
	uint8_t first_byte;
	memcpy(&first_byte, data, 1);

	type = first_byte>>6;
	if(type != PTYPE_DATA && type != PTYPE_ACK && type != PTYPE_NACK){
		fprintf(stderr, "Erreur type\n");
		return E_TYPE;
	}
	tr = first_byte>>5 & 0b00000001;
	if(tr != 1 && tr != 0){
		fprintf(stderr, "Erreur tr\n");
		return E_TR;
	}
	window = first_byte & 0b00011111;
	if(window > 31 || window < 0){
		fprintf(stderr, "Erreur window\n");
		return E_WINDOW;
	}

	// Deuxième byte : seqnum
	memcpy(&seqnum, data+1, 1);
	if(seqnum < 0 || seqnum > 255){
		fprintf(stderr, "Erreur seqnum\n");
		return E_SEQNUM;
	}

	// 3e et 4e bytes : length
	memcpy(&length, data+2, 2);
	length = ntohs(length);
	if(length < 0 || length > 512){
		fprintf(stderr, "Erreur length\n");
		return E_LENGTH;
	}

	// 5e -> 8e bytes : timestamp
	memcpy(&timestamp, data+4, 4);
	timestamp = ntohl(timestamp);

	// 9e -> 12e bytes : CRC1
	memcpy(&crc1_recv, data+8, 4);
	crc1_recv = ntohl(crc1_recv);
	// On vérifie si les deux CRC sont les mêmes
	uint32_t crc1_check = crc32(0, (const Bytef *) data, 8);
	if(crc1_recv != crc1_check){
		fprintf(stderr, "Erreur CRC1\n");
		return E_CRC;
	}


	char * payload = (char *) malloc(512*sizeof(char));
	if(payload == NULL){
		fprintf(stderr, "Erreur payload\n");
		return E_NOMEM;
	}

	if(type == PTYPE_DATA){

		// Payload
		memcpy(payload, data+12, length);

		// CRC2
		memcpy(&crc2_recv, data+12+length, 4);
		crc2_recv = ntohl(crc2_recv);
		// On vérifie si les deux CRC sont les mêmes
		uint32_t crc2_check = crc32(0, (const Bytef *) data+12, length);
		if(crc2_recv != crc2_check){
			fprintf(stderr, "Erreur CRC2\n");
			free(payload);
			return E_CRC;
		}

	}

	// Encodage des valeurs dans la structure pkt
	pkt_status_code err_code;

	err_code = pkt_set_type(pkt, type);
	if(err_code != PKT_OK){
		free(payload);
		return E_TYPE;
	}

	err_code = pkt_set_tr(pkt, tr);
	if(err_code != PKT_OK){
		free(payload);
		return E_TR;
	}

	err_code = pkt_set_window(pkt, window);
	if(err_code != PKT_OK){
		free(payload);
		return E_WINDOW;
	}

	err_code = pkt_set_seqnum(pkt, seqnum);
	if(err_code != PKT_OK){
		free(payload);
		return E_SEQNUM;
	}

	err_code = pkt_set_length(pkt, length);
	if(err_code != PKT_OK){
		free(payload);
		return E_LENGTH;
	}

	err_code = pkt_set_timestamp(pkt, timestamp);

	err_code = pkt_set_crc1(pkt, crc1_recv);

	err_code = pkt_set_payload(pkt, payload, length);
	if(err_code != PKT_OK){
		free(payload);
		return E_LENGTH;
	}

	err_code = pkt_set_crc2(pkt, crc2_recv);


	return PKT_OK;

}


	/*
  //size_t indice_crc1 = *(data + 8);// Indice du data ou on doit commencer à copier le crc1
  //size_t indice_timestamp = *(data + 4); // Indice du data ou on doit commencer à copier le timestamp
  // size_t indice_payload = *(data + 12);
  size_t i;
  // On copie le header
  for (i = 0; i < 4; i++){
  	memcpy((void *)data, pkt, len);
  }
  // On copie le timestamp
  for (i = 4; i < 8; i++){
		memcpy(&pkt->timestamp, &timestamp, 4);
  }
  // On decode le crc1
  uLong new_crc1 = crc32(0L, Z_NULL, 0);
  new_crc1 = crc32(new_crc1,(Bytef*) data, 8);

  if (crc1 != new_crc1){ // Si le crc1 n'est pas verifie
  	return E_CRC;
  }

  for (i = 8; i < 12; i++){
    memcpy(&pkt->crc1, &crc1, sizeof(crc1));
  }
  pkt_set_crc1(pkt, crc1);

  // On decode le payload
  for (i = 12; i < 21; i++){
  	memcpy(&pkt->payload, &payload, length);
  }

  // On decode le crc2
  if (length <= 0){ // Si le paquet ne contient pas de payload
  	if(pkt_get_tr(pkt) != 0){ // Si le paquet est tronque
 			pkt_del(pkt);
      return E_TR;
    }
    return E_LENGTH;
	}
  uLong new_crc2 = crc32(0L, Z_NULL, 0);
  new_crc2 = crc32(new_crc2,(const Bytef *) data, htons(pkt_get_length(pkt)));
  if (crc2 != new_crc2){ // Si le crc2 n'est pas verifie
 		pkt_del(pkt);
    return E_CRC;
  }
  for (i = len-4; i < len; i++){
    memcpy(&pkt->crc2, &crc2, sizeof(crc2));
  }
  pkt_set_crc2(pkt, crc2);
	*/

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
pkt_status_code pkt_encode(const pkt_t* pkt, uint8_t *buf, size_t len)
 {

  // Gerer le header
  uint8_t window = pkt_get_window(pkt);
	if(window > 31 || window < 0){
		return E_WINDOW;
	}
  uint8_t type = pkt_get_type(pkt);
	if(type != PTYPE_DATA && type != PTYPE_ACK && type != PTYPE_NACK){
		return E_TYPE;
	}
  uint8_t tr = pkt_get_tr(pkt);
	if(tr != 0 && tr != 1){
		return E_TR;
	}
  uint8_t seqnum = pkt_get_seqnum(pkt);
	if(seqnum < 0 || seqnum > 255){
		return E_SEQNUM;
	}
  uint16_t length = pkt_get_length(pkt); // 2 bytes
	if(length < 0 || length > 512){
		return E_LENGTH;
	}
	uint32_t timestamp = htonl(pkt_get_timestamp(pkt)); // 4 bytes
	const char* payload = pkt_get_payload(pkt); // up to 512 bytes

	// Teste si le buffer est trop petit
	if(len < length+16){
		return E_NOMEM;
	}

	length = htons(length);

	// Premier byte
  uint8_t type_format = type<<6 & 0b00000011000000;
	uint8_t tr_format = tr<<5 & 0b00000100000;
	uint8_t window_format = window & 0b00011111;
	uint8_t first_byte = type_format | tr_format | window_format;
  memcpy(buf, &first_byte, 1);

	// Deuxième byte
	memcpy(buf+1, &seqnum, 1); // seqnum

	// Troisième et quatrième bytes
  memcpy(buf+2, &length, 2); // length

	// Quatrième au huitième byte
	memcpy(buf+4, &timestamp, 4); // timestamp

  // Gerer les CRC
  uint32_t crc1 = htonl(crc32(0, (const Bytef *) buf, 8));
	printf("Calcul de CRC1 : %u\n", ntohl(crc1));
  // Huitième au douzième byte : crc1
	memcpy(buf+8, &crc1, 4);

	// Si le paquet n'est pas tronqué
	if(tr == 0){
  	memcpy(buf+12, payload, ntohs(length)); // 12e -> 524e byte : payload

		uint32_t crc2 = htonl(crc32(0, (const Bytef *) buf+12, ntohs(length))); // Calcul du crc2
		printf("Calcul de CRC2 : %u\n", ntohl(crc2));
	 	memcpy(buf+12+ntohs(length), &crc2, 4);
	}

	return PKT_OK;
}


/* Resolve the resource name to an usable IPv6 address
 * @address: The name to resolve
 * @rval: Where the resulting IPv6 address descriptor should be stored
 * @return: NULL if it succeeded, or a pointer towards
 *          a string describing the error if any.
 *          (const char* means the caller cannot modify or free the return value,
 *           so do not use malloc!)
 */
const char *real_address(const char *address, struct sockaddr_in6 *rval){

    int err;

    struct addrinfo *hints = (struct addrinfo *) calloc(1, sizeof(struct addrinfo));
    if(hints == NULL){
      return "Erreur d'alloction de mémoire.";
    }
    struct addrinfo *results;

    //memset(hints, 0, sizeof(hints)); inutile si on fait calloc() au-dessus
    hints->ai_family = AF_INET6;
    hints->ai_protocol = IPPROTO_UDP;
		hints->ai_socktype = SOCK_DGRAM; // Car UDP packets
		// hints.ai_socktype = SOCK_STREAM; si TCP packets

    err = getaddrinfo(address, NULL, hints, &results);
    if(err != 0){
        return "Erreur d'exécution de la fonction addrinfo.";
    }

    struct sockaddr *temp = results->ai_addr;
    struct sockaddr_in6 *final = (struct sockaddr_in6 *) temp;
    *rval = *final;

    freeaddrinfo(results);
    return NULL;
}


/* Creates a socket and initialize it
 * @source_addr: if !NULL, the source address that should be bound to this socket
 * @src_port: if >0, the port on which the socket is listening
 * @dest_addr: if !NULL, the destination address to which the socket should send data
 * @dst_port: if >0, the destination port to which the socket should be connected
 * @return: a file descriptor number representing the socket,
 *         or -1 in case of error (explanation will be printed on stderr)
 */
int create_socket(struct sockaddr_in6 *source_addr, int src_port, struct sockaddr_in6 *dest_addr, int dst_port){

  int err; // Variable pour error check

  // Création du socket
  int socketfd = socket(AF_INET6, SOCK_DGRAM, 0);
  if(socketfd < 0){
    fprintf(stderr, "ERROR : fonction socket()\n");
    perror("socket");
    return -1;
  }

  // Liaison à la source
  if(source_addr != NULL && src_port > 0){

    struct addrinfo *hints = (struct addrinfo *) calloc(1, sizeof(struct addrinfo));
    if(hints == NULL){
      fprintf(stderr, "ERROR : fonction calloc()\n");
      return -1;
    }
    hints->ai_family = AF_INET6;
    hints->ai_socktype = SOCK_DGRAM;
    hints->ai_protocol = 0;

    struct addrinfo *res;

    err = getaddrinfo("::1", NULL, hints, &res);
    if(err != 0){
      perror("getaddrinfo");
      free(hints);
      return -1;
    }

    source_addr->sin6_port = htons(src_port); // Utilisation de htons pour convertir en Network byte order
    err = bind(socketfd, (struct sockaddr*) source_addr, res->ai_addrlen);
    // Error check
    if(err != 0){
      fprintf(stderr, "ERROR : fonction bind() source\n");
      perror("bind");
      return -1;
    }
  }

  // Liaison à la destination

  struct addrinfo *hints = (struct addrinfo *) calloc(1, sizeof(struct addrinfo));
  if(hints == NULL){
    fprintf(stderr, "ERROR : fonction calloc()\n");
    return -1;
  }
  hints->ai_family = AF_INET6;
  hints->ai_socktype = SOCK_DGRAM;
  hints->ai_protocol = 0;

  struct addrinfo *res;

  err = getaddrinfo("::1", NULL, hints, &res);
  if(err != 0){
    perror("getaddrinfo");
    free(hints);
    return -1;
  }

  if(dest_addr != NULL && dst_port > 0){
    dest_addr->sin6_port = htons(dst_port); // Utilisation de htons pour convertir en Network byte order
    err = connect(socketfd, (struct sockaddr*) dest_addr, res->ai_addrlen);
    // Error check
    if(err != 0){
      fprintf(stderr, "ERROR : fonction connect() destination\n");
      perror("connect");
      return -1;
    }
  }

  return socketfd;
}


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


/* Block the caller until a message is received on sfd,
 * and connect the socket to the source addresse of the received message.
 * @sfd: a file descriptor to a bound socket but not yet connected
 * @return: 0 in case of success, -1 otherwise
 * @POST: This call is idempotent, it does not 'consume' the data of the message,
 * and could be repeated several times blocking only at the first call.
 */
int wait_for_client(int sfd){

	struct sockaddr_in6 * address = (struct sockaddr_in6 *)calloc(1024,sizeof(struct sockaddr_in6));
	if(address == NULL){
    fprintf(stderr, "ERROR : fonction calloc()\n");
    return -1;
  }

	char * buf = (char *)malloc(1024*sizeof(char));
	if(buf == NULL){
    fprintf(stderr, "ERROR : fonction malloc()\n");
    return -1;
  }

	socklen_t len = sizeof(*address);
  int err;

  err = recvfrom(sfd, buf, sizeof(buf), MSG_PEEK, (struct sockaddr *) address, &len);
  if(err < 0){
    fprintf(stderr, "Erreur de la fonction recvfrom\n");
    return -1;
  }

  err = connect(sfd, (struct sockaddr *)address, len);
  if(err < 0){
    fprintf(stderr, "Erreur de connexion\n");
  }
  return 0;
}


/*
 * Incrémente le numéro de séquence de 1.
 * Si le numéro de séquence était 255, le remet à 0.
 *
 * @return : 0 si le numéro de séquence a été incrémenté
 *           1 si le numéro de séquence a été remis à 0
 *           -1 si le numéro de séquence n'était pas valide
 */
int seqnum_inc(int* seqnum){
	if(*seqnum >= 0 && *seqnum <= 254){
		*seqnum = *seqnum + 1;
		return 0;
	}
	else if(*seqnum == 255){
		*seqnum = 0;
		return 1;
	}
	else{
		return -1;
	}
}

/*
 * Vérifie si le numéro de séquence est dans la fenetre
 *
 * @return : 0 si il est dans la fenetre
 *					 -1 si il est hors de la fenetre
 *					 1 si une erreur d'argument
 *
 */
int in_window (uint8_t seqnum, uint8_t min_window, uint8_t len_window){

	if (seqnum > 255){
		printf("Le numero de sequence n'est pas valide\n");
		return 1;
	}

	if (len_window > MAX_WINDOW_SIZE){
		printf("La fenetre a une taille trop grande\n");
		return 1;
	}

	// Si le numero de sequence ne se trouve pas dans la fenetre
	if ((seqnum > (len_window + min_window -1)) || (seqnum < min_window)){
		printf("Le numero de sequence est hors de la fenetre\n");
		return -1;
	}

	// Si le numero de sequence se trouve dans la fenetre
	return 0;
}


/*
 * Ajoute un buffer dans le buffer d'envoi ou de reception
 *
 * @return : - le buffer d'envoi ou de reception modifié
 *
 */
uint8_t ** ajout_buffer (uint8_t * buffer, uint8_t ** buffer_recept){
  if (buffer_recept == NULL){
    *buffer_recept = buffer;
  }
	else {
		(*buffer_recept++) = buffer;
	}
	return buffer_recept;
}


/*
 * Retire un element dans le buffer d'envoi ou de reception
 *
 * @return : 0 si l'element a ete correctement retire du buffer
 * 					 1 si l'element n'a pas ete retire correctement
 */
 int retire_buffer(uint8_t ** buffer, uint8_t seqnum){
	 uint8_t compare;

	 for(size_t i = 0; i<sizeof(buffer); i++){
		 memcpy(&compare, (*(buffer+i))+1, 1);
		 if (compare == seqnum){
			 buffer[i] = NULL;
			 return 0;
		 }
	 }
	 return 1;
 }


 /*
  * Verifie si le buffer est plein ou pas
  *
  * @return : 1 si le buffer est plein
	*  					0 si il reste au moins une place dans le buffer
  */
 int buffer_plein(uint8_t ** buffer){
	 for(size_t i = 0; i<sizeof(buffer); i++){
		 if (buffer[i] == NULL){
			 return 0;
		 }
	 }
	 return 1;
 }


 /*
  * Decale la fenetre de reception ou d'envoi
  *
  * @return : 1 si la fenetre n'a pas ete decalee correctement
  *  					0 si la fenetre a ete deplacee correctement
  */
 int decale_window(uint8_t len_window, uint8_t min_window, uint8_t seqnum){
	 if (len_window > MAX_WINDOW_SIZE){
		 return 1;
	 }
	 if (seqnum > 255){
		 return 1;
	 }
	 min_window = seqnum;
	 return 0;
 }
