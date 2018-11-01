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

// Definition de la structure d'un paquet
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

/*
 * pkt_new : Fonction qui crée un nouveau paquet de type PTYPE_DATA
 *
 * @return : un nouveau paquet de type PTYPE_DATA ou NULL en cas d'erreur
 */
pkt_t* pkt_new()
{
	pkt_t * new = (pkt_t *) malloc(sizeof(pkt_t));
  if (new == NULL){
    fprintf(stderr, "Erreur du malloc");
    return NULL;
  }
  new->window = 0; // Par definition, on fait commencer la fenetre à 1
  new->tr = 0;
  new->type = PTYPE_DATA;
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

/*
 * pkt_ack_new : Fonction qui crée un nouveau paquet de type PTYPE_ACK
 *
 * @return : un nouveau paquet de type PTYPE_ACK ou NULL en cas d'erreur
 */
pkt_t* pkt_ack_new(){
	pkt_t * new = (pkt_t *) malloc(sizeof(pkt_t));
  if (new == NULL){
    fprintf(stderr, "Erreur du malloc");
    return NULL;
  }
  new->window = 0; // Par definition, on fait commencer la fenetre à 1
  new->tr = 0;
  new->type = PTYPE_ACK;
  new->seqnum = 0;
  new->length = 0;
  new->timestamp = 0;
  new->crc1 = 0;
	return new;
}


/*
 * pkt_del : Libere le pointeur vers la struct pkt, ainsi que toutes les
 * ressources associees
 *
 * @pkt : pointeur vers un paquet
 * @return : /
 */
void pkt_del(pkt_t *pkt)
{
	if(pkt_get_type(pkt) == PTYPE_DATA){
    	free(pkt->payload);
		}
    free(pkt);
}

/*
 * pkt_get_type : Fonction qui va chercher le type du paquet place en argument
 *
 * @pkt : pointeur vers un paquet
 * @return : le type du paquet
 */
ptypes_t pkt_get_type  (const pkt_t * pkt)
{
	return pkt->type;
}

/*
 * pkt_get_tr : Fonction qui va chercher le bit qui correspond
 * au troncage du paquet placé en argument
 *
 * @pkt : pointeur vers un paquet
 * @return : le bit qui correspond au troncage du paquet
 */
uint8_t  pkt_get_tr(const pkt_t * pkt)
{
	return pkt->tr;
}

/*
 * pkt_get_window : Fonction qui va chercher la longueur de la fenetre de
 * l'emetteur du paquet place en argument
 *
 * @pkt : pointeur vers un paquet
 * @return : la longueur de la fenetre glissante
 */
uint8_t  pkt_get_window(const pkt_t * pkt)
{
	return pkt->window;
}

/*
 * pkt_get_seqnum : Fonction qui va chercher le numero de sequence
 * du paquet place en argument
 *
 * @pkt : pointeur vers un paquet
 * @return : le numero de sequence du paquet
 */
uint8_t  pkt_get_seqnum(const pkt_t * pkt)
{
	return pkt->seqnum;
}

/*
 * pkt_get_length: Fonction qui va chercher la longueur du payload
 * du paquet place en argument
 *
 * @pkt : pointeur vers un paquet
 * @return : la longueur du payload
 */
uint16_t pkt_get_length(const pkt_t * pkt)
{
	return pkt->length;
}

/*
 * pkt_get_timestamp: Fonction qui va chercher le timestamp
 * du paquet place en argument
 *
 * @pkt : pointeur vers un paquet
 * @return : le timestamp
 */
uint32_t pkt_get_timestamp(const pkt_t * pkt)
{
	return pkt->timestamp;
}

/*
 * pkt_get_crc1: Fonction qui va chercher la valeur du CRC1
 * du paquet place en argument
 *
 * @pkt : pointeur vers un paquet
 * @return : la valeur du CRC1
 */
uint32_t pkt_get_crc1(const pkt_t * pkt)
{
	return pkt->crc1;
}

/*
 * pkt_get_crc2: Fonction qui va chercher la valeur du CRC2
 * du paquet place en argument
 *
 * @pkt : pointeur vers un paquet
 * @return : la valeur du CRC2 ou 0 si il n'y a pas de CRC2
 */
uint32_t pkt_get_crc2(const pkt_t * pkt)
{
	return pkt->crc2;
}

/*
 * pkt_get_payload: Fonction qui va chercher l'information du payload
 * du paquet place en argument
 *
 * @pkt : pointeur vers un paquet
 * @return : un pointeur vers les données contenues dans le payload
 */
const char* pkt_get_payload(const pkt_t * pkt)
{
	if ((pkt->length) <= 0){
    return NULL;
  }
  return pkt->payload;
}

/*
 * pkt_set_type : Fonction qui va initialiser le type du paquet en arguments
 * a une certaine valeur
 *
 * @type : valeur a laquelle le type du paquet doit etre initialisee
 * @pkt : pointeur vers un paquet
 * @return : Un code indiquant si l'operation a reussi ou representant
 * l'erreur rencontree
 */
pkt_status_code pkt_set_type(pkt_t *pkt, const ptypes_t type)
{
	if(type == 1 || type == 2 || type == 3){
    pkt->type = type;
    return PKT_OK;
  }
  return E_TYPE;
}

/*
 * pkt_set_tr : Fonction qui va initialiser le bit de troncage du paquet en arguments
 * a une certaine valeur
 *
 * @tr : valeur a laquelle le troncage du paquet doit etre initialisee
 * @pkt : pointeur vers un paquet
 * @return : Un code indiquant si l'operation a reussi ou representant
 * l'erreur rencontree
 */
pkt_status_code pkt_set_tr(pkt_t *pkt, const uint8_t tr)
{
	if (tr != 0 && tr != 1){
    return E_TR;
  }
  pkt->tr = tr;
  return PKT_OK;
}

/*
 * pkt_set_window : Fonction qui va initialiser la taille de la fenetre du
 * paquet en arguments a une certaine valeur
 *
 * @window : valeur a laquelle la fenetre du paquet doit etre initialisee
 * @pkt : pointeur vers un paquet
 * @return : Un code indiquant si l'operation a reussi ou representant
 * l'erreur rencontree
 */
pkt_status_code pkt_set_window(pkt_t *pkt, const uint8_t window)
{
	if (window > MAX_WINDOW_SIZE){
    return E_WINDOW;
  }
  pkt->window = window;
  return PKT_OK;
}

/*
 * pkt_set_seqnum : Fonction qui va initialiser le numero de sequence du
 * paquet en arguments a une certaine valeur
 *
 * @seqnum : valeur a laquelle le numero de sequence du paquet doit etre initialisee
 * @pkt : pointeur vers un paquet
 * @return : Un code indiquant si l'operation a reussi ou representant
 * l'erreur rencontree
 */
pkt_status_code pkt_set_seqnum(pkt_t *pkt, const uint8_t seqnum)
{
	if(seqnum > 255 || seqnum < 0){
		return E_SEQNUM;
	}
	pkt->seqnum = seqnum;
  return PKT_OK;
}

/*
 * pkt_set_length : Fonction qui va initialiser la longueur du payload du
 * paquet en arguments a une certaine valeur
 *
 * @length : valeur a laquelle la longueur du payload du paquet doit etre initialisee
 * @pkt : pointeur vers un paquet
 * @return : Un code indiquant si l'operation a reussi ou representant
 * l'erreur rencontree
 */
pkt_status_code pkt_set_length(pkt_t *pkt, const uint16_t length)
{
	if (length > MAX_PAYLOAD_SIZE){
    return E_LENGTH;
  }
  pkt->length = length;
  return PKT_OK;
}

/*
 * pkt_set_timestamp : Fonction qui va initialiser le timestamp du
 * paquet en arguments a une certaine valeur
 *
 * @timestamp : valeur a laquelle le timestamp du paquet doit etre initialise
 * @pkt : pointeur vers un paquet
 * @return : Un code indiquant si l'operation a reussi ou representant
 * l'erreur rencontree
 */
pkt_status_code pkt_set_timestamp(pkt_t *pkt, const uint32_t timestamp)
{
	pkt->timestamp = timestamp;
  return PKT_OK;
}

/*
 * pkt_set_crc1 : Fonction qui va initialiser la valeur du CRC1 du
 * paquet en arguments a une certaine valeur
 *
 * @crc1 : valeur a laquelle le CRC1 du paquet doit etre initialisee
 * @pkt : pointeur vers un paquet
 * @return : Un code indiquant si l'operation a reussi ou representant
 * l'erreur rencontree
 */
pkt_status_code pkt_set_crc1(pkt_t *pkt, const uint32_t crc1)
{
	pkt->crc1 = crc1;
  return PKT_OK;
}

/*
 * pkt_set_crc2 : Fonction qui va initialiser la valeur du CRC2 du
 * paquet en arguments a une certaine valeur
 *
 * @crc2 : valeur a laquelle le CRC2 du paquet doit etre initialisee
 * @pkt : pointeur vers un paquet
 * @return : Un code indiquant si l'operation a reussi ou representant
 * l'erreur rencontree
 */
pkt_status_code pkt_set_crc2(pkt_t *pkt, const uint32_t crc2)
{
	pkt->crc2 = crc2;
  return PKT_OK;
}

/*
 * pkt_set_payload : Fonction qui va copier une certaine information dans
 * le payload du paquet en arguments
 *
 * @pkt : pointeur vers un paquet
 * @data : l'information à mettre dans le payload
 * @length : la longueur du payload
 * @return : Un code indiquant si l'operation a reussi ou representant
 * l'erreur rencontree
 */
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
 * pkt_decode : Decode des donnees recues et cree une nouvelle structure pkt.
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
 * @return: Un code indiquant si l'operation a reussi ou representant
 * l'erreur rencontree
 */
pkt_status_code pkt_decode(uint8_t *data, const size_t len, pkt_t *pkt){

	pkt_status_code err_code;

  if (len < 12){ // Il n'y a pas de header car il est encode sur 12 bytes
    return E_NOHEADER;
  }
	else if(len > 528){ // Le paquet est trop long
		return E_UNCONSISTENT;
	}

	// Initialisation des variables
	ptypes_t type;
	uint8_t tr;
	uint8_t window;
	uint8_t seqnum;
	uint16_t length;
	uint32_t timestamp;
	uint32_t crc1_recv;

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

	// Encodage des valeurs dans la structure pkt

	err_code = pkt_set_type(pkt, type);
	if(err_code != PKT_OK){
		return E_TYPE;
	}

	err_code = pkt_set_tr(pkt, tr);
	if(err_code != PKT_OK){
		return E_TR;
	}

	err_code = pkt_set_window(pkt, window);
	if(err_code != PKT_OK){
		return E_WINDOW;
	}

	err_code = pkt_set_seqnum(pkt, seqnum);
	if(err_code != PKT_OK){
		return E_SEQNUM;
	}

	err_code = pkt_set_length(pkt, length);
	if(err_code != PKT_OK){
		return E_LENGTH;
	}

	err_code = pkt_set_timestamp(pkt, timestamp);

	err_code = pkt_set_crc1(pkt, crc1_recv);

	if(type == PTYPE_DATA){

		uint32_t crc2_recv;
		char * payload = (char *) malloc(512*sizeof(char));
		if(payload == NULL){
			fprintf(stderr, "Erreur payload\n");
			return E_NOMEM;
		}

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

		err_code = pkt_set_payload(pkt, payload, length);
		if(err_code != PKT_OK){
			free(payload);
			return E_LENGTH;
		}

		err_code = pkt_set_crc2(pkt, crc2_recv);

	}


	return PKT_OK;

}

/*
 * pkt_encode : Encode une struct pkt dans un buffer, pret a etre envoye sur le reseau
 * (c-a-d en network byte-order), incluant le CRC32 du header et
 * eventuellement le CRC32 du payload si celui-ci est non nul.
 *
 * @pkt: La structure a encoder
 * @buf: Le buffer dans lequel la structure sera encodee
 * @len: La taille disponible dans le buffer
 * @len-POST: Le nombre de d'octets ecrit dans le buffer
 * @return: Un code indiquant si l'operation a reussi ou E_NOMEM si
 * le buffer est trop petit.
 */
pkt_status_code pkt_encode(const pkt_t* pkt, uint8_t *buf, size_t len)
 {

  // Gerer le header
  uint8_t window = pkt_get_window(pkt);
	if(window > 31 || window < 0){
		fprintf(stderr, "Erreur window\n");
		return E_WINDOW;
	}
  uint8_t type = pkt_get_type(pkt);
	if(type != PTYPE_DATA && type != PTYPE_ACK && type != PTYPE_NACK){
		fprintf(stderr, "Erreur type\n");
		return E_TYPE;
	}
  uint8_t tr = pkt_get_tr(pkt);
	if(tr != 0 && tr != 1){
		fprintf(stderr, "Erreur tr\n");
		return E_TR;
	}
  uint8_t seqnum = pkt_get_seqnum(pkt);
	if(seqnum < 0 || seqnum > 255){
		fprintf(stderr, "Erreur seqnum\n");
		return E_SEQNUM;
	}
  uint16_t length = pkt_get_length(pkt); // 2 bytes
	if(length < 0 || length > 512){
		fprintf(stderr, "Erreur length\n");
		return E_LENGTH;
	}
	uint32_t timestamp = htonl(pkt_get_timestamp(pkt)); // 4 bytes


	// Teste si le buffer est trop petit
	if(len < length+16){
		fprintf(stderr, "Erreur nomem\n");
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


	printf("Type : %u\n", type);
	// Si le paquet n'est pas tronqué
	if(tr == 0 && type == PTYPE_DATA){

		const char* payload = pkt_get_payload(pkt); // up to 512 bytes
		printf("Payload avant encodage : %s\n", payload);

  	memcpy(buf+12, payload, ntohs(length)); // 12e -> 524e byte : payload

		uint32_t crc2 = htonl(crc32(0, (const Bytef *) buf+12, ntohs(length))); // Calcul du crc2
		printf("Calcul de CRC2 : %u\n", ntohl(crc2));
	 	memcpy(buf+12+ntohs(length), &crc2, 4);
	}

	return PKT_OK;
}


/*
 * real_address : Trouve le nom de la ressource correspondant à une adresse IPv6
 *
 * @address: L'adresse a trouver
 * @rval: Où le resultat de la description de l'adresse IPv6 sera stockee
 *
 * @return: - NULL si tout s'est bien deroule
 *          - un pointeur vers un string decrivant l'erreur sinon
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


/*
 * create_socket : Cree un socket et l'initialise
 *
 * @source_addr: si !NULL, l'adresse source devrait etre liee a ce socket
 * @src_port: si >0, le port sur lequel le socket ecoute
 * @dest_addr: si !NULL, l'adresse de destination auquelle le socket doit
 * envoyer de l'information
 * @dst_port: si >0, le port de destination auquel le socket devrait etre connecte
 *
 * @return: - un numero de file descriptor representant le socket
 *          - -1 en cas d'erreur
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



/*
 * wait_for_client : Bloque le receveur jusqu'a ce qu'un message soit recu sur sfd
 * et connecte le socket a l'adresse source du message recu
 *
 * @sfd: un file descriptor vers un socket lie mais pas connecte
 *
 * @return: - 0 en cas de succes
 *				  -1 otherwise
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
 * seqnum_inc : Incremente le numéro de sequence de 1.
 * Si le numero de sequence etait 255, le remet à 0.
 *
 * @seqnum : numero de sequence à incrementer
 *
 * @return : 0 si le numéro de séquence a été incrémenté
 *           1 si le numéro de séquence a été remis à 0
 *           -1 si le numéro de séquence n'était pas valide
 */
int seqnum_inc(uint8_t* seqnum){
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
 * in_window : Vérifie si le numéro de séquence est dans la fenetre
 *
 * @seqnum : numero de sequence à verifier
 * @min_window : le plus petit numero de sequence present dans la fenetre
 * @max_window : le plus grand numero de sequence present dans la fenetre
 *
 * @return : 0 si il est dans la fenetre
 *					 -1 si il est hors de la fenetre
 *					 1 si une erreur d'argument
 *
 */
 int in_window (uint8_t seqnum, uint8_t min_window, uint8_t max_window){

 	if (seqnum < 0 || seqnum > 255){
 		printf("Le numero de sequence n'est pas valide\n");
 		return 1;
 	}

	if(min_window < max_window){
		if(seqnum >= min_window && seqnum <= max_window){
			return 0;
		}
		else{
			return -1;
		}
	}
	else{
		if(seqnum >= min_window || seqnum <= max_window){
			return 0;
		}
		else{
			return -1;
		}
	}
 }


/*
 * decale_window : Decale la fenetre de reception ou d'envoi
 *
 * @min_window : un pointeur vers le plus petit numero de sequence present dans la fenetre
 * @max_window : un pointeur vers le plus grand numero de sequence present dans la fenetre
 *
 * @return : /
 */
void decale_window(uint8_t *min_window, uint8_t *max_window){
	if(*max_window == 255){
		*min_window = *min_window + 1;
		*max_window = 0;
	}
	else if(*min_window == 255){
		*min_window = 0;
		*max_window = *max_window + 1;
	}
	else{
		*min_window = *min_window + 1;
		*max_window = *max_window + 1;
	}
}

/*
 * ajout_buffer : Ajoute un buffer dans le buffer d'envoi ou de reception
 *
 * @pkt : un pointeur vers un paquet
 * @buffer_recept : buffer de reception qui contient lui-meme des buffer
 * @min_window : le plus petit numero de sequence present dans la fenetre
 *
 * @return : 0 si le paquet a bien été ajouté au buffer
 *           1 si le paquet n'a pas été ajouté au buffer
 */
 void ajout_buffer (pkt_t* pkt, pkt_t** buffer_recept, uint8_t min_window){
 	int i;
 	printf("Min window : %u\n", min_window);
 	printf("Seqnum : %u\n", pkt_get_seqnum(pkt));
 	if(min_window <= pkt_get_seqnum(pkt)){
 		i = pkt_get_seqnum(pkt) - min_window;
 		printf("Place à laquelle le paquet est mis dans le buffer : %d\n", i);
 		*(buffer_recept + i) = pkt;
 	}
 	else{
 		i = (255-min_window+1+pkt_get_seqnum(pkt));
 		printf("Place à laquelle le paquet est mis dans le buffer : %d\n", i);
 		*(buffer_recept + i) = pkt;
 	}
 }

/*
 * get_from_buffer : Retrouve le paquet qui correspond à un numero de sequence
 * particulier
 *
 * @pkt : un pointeur vers un buffer de paquet
 * @seqnum : numero de sequence du paquet a retrouver
 *
 * @return : - le paquet qui a pour numero de sequence seqnum
 *           - NULL en cas d'erreur
 */
pkt_t* get_from_buffer(pkt_t ** buffer, uint8_t seqnum){
	int i;
	for(i = 0; i < MAX_WINDOW_SIZE; i++){
		if(*(buffer+i) != NULL){
			if(seqnum == pkt_get_seqnum(*(buffer+i))){
				return *(buffer+i);
			}
		}
	}
	return NULL;
}

/*
 * retire_buffer : Retire un paquet dans le buffer d'envoi ou de reception
 *
 * @buffer : buffer de paquets
 * @seqnum : numero de sequence du paquet a retirer du buffer
 *
 * @return : - 0 si l'element a ete correctement retire du buffer
 * 					 - 1 si l'element n'a pas ete retire correctement
 */
 int retire_buffer(pkt_t ** buffer, uint8_t seqnum){
	 int i;
	 for(i = 0; i < MAX_WINDOW_SIZE; i++){
 		if(*(buffer+i) != NULL){
 			if(seqnum == pkt_get_seqnum(*(buffer+i))){
				pkt_del(*(buffer+i));
				*(buffer+i) = NULL;
				return 0;
 			}
 		}
 	}
 	return 1;
 }


 /*
  * buffer_plein : Verifie si le buffer est plein ou pas
  *
	* @buffer : buffer de paquets a parcourir
	*
  * @return : - 1 si le buffer est plein
	*  					- 0 si il reste au moins une place dans le buffer
  */
 int buffer_plein(pkt_t ** buffer){
	 int i;
	 for(i = 0; i < LENGTH_BUF_REC; i++){
		 if (buffer[i] == NULL){
			 return 0;
		 }
	 }
	 return 1;
 }

/*
 * write_buffer : Ecrit tous les éléments du buffer qui sont disponible et dans l'ordre
 *
 * @fd : un numero de file descriptor representant le socket
 * @buffer : un buffer de paquets
 * @min_window : un pointeur vers le plus petit numero de sequence present dans la fenetre
 * @max_window : un pointeur vers le plus grand numero de sequence present dans la fenetre
 *
 * @return : le nombre d'elements ecrits
 *
 */
int write_buffer(int fd, pkt_t **buffer, uint8_t *min_window, uint8_t *max_window){
	int i = 0;
	for(; i < LENGTH_BUF_REC; i++){
	 if(buffer[i] == NULL){
		 return i;
	 }
	 else{
		 if(fd == STDOUT){
			 printf("%s\n", pkt_get_payload(buffer[i]));
			 pkt_del(buffer[i]);
			 buffer[i] = NULL;
			 decale_window(min_window, max_window);
		 }
		 else{
			 write(fd, pkt_get_payload(buffer[i]), pkt_get_length(buffer[i]));
			 pkt_del(buffer[i]);
			 buffer[i] = NULL;
			 decale_window(min_window, max_window);
		 }
	 }
 }
 return i;
}



/*
 * arg_check : Vérification du nombre d'arguments passes en ligne de commande
 *
 * @argc : le nombre d'arguments passes en ligne de commande
 * @n_min : le nombre min d'arguments requis
 * @n_max : le nombre maximal d'arguments que l'on peut passer en ligne de commande
 *
 * @return : - 0 si le nombre d'arguments a ete verifie correctement
 *   				 - -1 en cas d'erreur
 */
int arg_check(int argc, int n_min, int n_max){
	if(argc < n_min){
		fprintf(stderr, "Pas assez d'arguments.\n");
		return -1;
	}
	else if(argc > n_max){
		fprintf(stderr, "Trop d'arguments.\n");
		return -1;
	}
	return 0;
}
