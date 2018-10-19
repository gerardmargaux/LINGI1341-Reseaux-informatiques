#ifndef _LIBRARY_H
#define _LIBRARY_H

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

/* Raccourci pour struct pkt */
typedef struct pkt pkt_t;

/* Types de paquets */
typedef enum {
	PTYPE_DATA = 1,
	PTYPE_ACK = 2,
	PTYPE_NACK = 3,
} ptypes_t;



/* Taille maximale permise pour le payload */
#define MAX_PAYLOAD_SIZE 512
/* Taille maximale de Window */
#define MAX_WINDOW_SIZE 31

#define LENGTH_BUF_REC 31

#define STDIN 0
#define STDOUT 1
#define STDERR 2

/* Valeur de retours des fonctions */
typedef enum {
	PKT_OK = 0,     /* Le paquet a ete traite avec succes */
	E_TYPE,         /* Erreur liee au champs Type */
	E_TR,           /* Erreur liee au champ TR */
	E_LENGTH,       /* Erreur liee au champs Length  */
	E_CRC,          /* CRC invalide */
	E_WINDOW,       /* Erreur liee au champs Window */
	E_SEQNUM,       /* Numero de sequence invalide */
	E_NOMEM,        /* Pas assez de memoire */
	E_NOHEADER,     /* Le paquet n'a pas de header (trop court) */
	E_UNCONSISTENT, /* Le paquet est incoherent */
} pkt_status_code;


// Fonctions utiles pour le projet

/* Alloue et initialise une struct pkt
 * @return: NULL en cas d'erreur */
pkt_t* pkt_new();

pkt_t* pkt_ack_new();

/* Libere le pointeur vers la struct pkt, ainsi que toutes les
 * ressources associees
 */
void pkt_del(pkt_t* pkt);

/* Accesseurs pour les champs toujours presents du paquet.
 * Les valeurs renvoyees sont toutes dans l'endianness native
 * de la machine!
 */
ptypes_t pkt_get_type     (const pkt_t* pkt);
uint8_t  pkt_get_tr       (const pkt_t* pkt);
uint8_t  pkt_get_window   (const pkt_t* pkt);
uint8_t  pkt_get_seqnum   (const pkt_t* pkt);
uint16_t pkt_get_length   (const pkt_t* pkt);
uint32_t pkt_get_timestamp(const pkt_t* pkt);
uint32_t pkt_get_crc1     (const pkt_t* pkt);

/* Renvoie un pointeur vers le payload du paquet, ou NULL s'il n'y
 * en a pas.
 */
const char* pkt_get_payload(const pkt_t* pkt);


/* Renvoie le CRC2 dans l'endianness native de la machine. Si
 * ce field n'est pas present, retourne 0.
 */
uint32_t pkt_get_crc2(const pkt_t* pkt);

/* Setters pour les champs obligatoires du paquet. Si les valeurs
 * fournies ne sont pas dans les limites acceptables, les fonctions
 * doivent renvoyer un code d'erreur adapte.
 * Les valeurs fournies sont dans l'endianness native de la machine!
 */
pkt_status_code pkt_set_type     (pkt_t* pkt, const ptypes_t type);
pkt_status_code pkt_set_tr       (pkt_t* pkt, const uint8_t tr);
pkt_status_code pkt_set_window   (pkt_t* pkt, const uint8_t window);
pkt_status_code pkt_set_seqnum   (pkt_t* pkt, const uint8_t seqnum);
pkt_status_code pkt_set_length   (pkt_t* pkt, const uint16_t length);
pkt_status_code pkt_set_timestamp(pkt_t* pkt, const uint32_t timestamp);
pkt_status_code pkt_set_crc1     (pkt_t* pkt, const uint32_t crc1);

/* Defini la valeur du champs payload du paquet.
 * @data: Une succession d'octets representants le payload
 * @length: Le nombre d'octets composant le payload
 * @POST: pkt_get_length(pkt) == length */
pkt_status_code pkt_set_payload(pkt_t* pkt,
                                const char *data,
                                const uint16_t length);

/* Setter pour CRC2. Les valeurs fournies sont dans l'endianness
 * native de la machine!
 */
pkt_status_code pkt_set_crc2(pkt_t* pkt, const uint32_t crc2);

/*
 * Crée un paquet et initialise tous ses champs avec les arguments de la fonction
 */
pkt_t* pkt_init(ptypes_t type, uint8_t tr, uint8_t window, uint8_t seqnum,
								uint16_t length, uint32_t timestamp, uint32_t crc1, uint32_t crc2,
								const char* payload);

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
pkt_status_code pkt_encode(const pkt_t* pkt, uint8_t *buf, size_t len);

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
pkt_status_code pkt_decode(uint8_t *data, const size_t len, pkt_t *pkt);

/* Resolve the resource name to an usable IPv6 address
 * @address: The name to resolve
 * @rval: Where the resulting IPv6 address descriptor should be stored
 * @return: NULL if it succeeded, or a pointer towards
 *          a string describing the error if any.
 *          (const char* means the caller cannot modify or free the return value,
 *           so do not use malloc!)
 */
const char * real_address(const char *address, struct sockaddr_in6 *rval);

/* Creates a socket and initialize it
 * @source_addr: if !NULL, the source address that should be bound to this socket
 * @src_port: if >0, the port on which the socket is listening
 * @dest_addr: if !NULL, the destination address to which the socket should send data
 * @dst_port: if >0, the destination port to which the socket should be connected
 * @return: a file descriptor number representing the socket,
 *         or -1 in case of error (explanation will be printed on stderr)
 */
int create_socket(struct sockaddr_in6 *source_addr,
                  int src_port,
                  struct sockaddr_in6 *dest_addr,
                  int dst_port);

/* Loop reading a socket and printing to stdout,
 * while reading stdin and writing to the socket
 * @sfd: The socket file descriptor. It is both bound and connected.
 * @return: as soon as stdin signals EOF
 */
void read_write_loop(const int sfd);

/* Block the caller until a message is received on sfd,
 * and connect the socket to the source addresse of the received message.
 * @sfd: a file descriptor to a bound socket but not yet connected
 * @return: 0 in case of success, -1 otherwise
 * @POST: This call is idempotent, it does not 'consume' the data of the message,
 * and could be repeated several times blocking only at the first call.
 */
int wait_for_client(int sfd);


/*
 * Incrémente le numéro de séquence de 1.
 * Si le numéro de séquence était 255, le remet à 0.
 *
 * @return : 0 si le numéro de séquence a été incrémenté
 *           1 si le numéro de séquence a été remis à 0
 *           -1 si le numéro de séquence n'était pas valide
 */
int seqnum_inc(int* seqnum);


/*
 * Vérifie si le numéro de séquence est dans la fenetre
 *
 * @return : 0 si il est dans la fenetre et est ajouté au buffer de reception
 *					 -1 si il est hors de la fenetre. Le paquet recu est discardé
 *
 */
int in_window (uint8_t seqnum, uint8_t min_window, uint8_t len_window);


/*
 * Ajoute un buffer dans le buffer d'envoi ou de reception
 *
 * @return : - le buffer d'envoi ou de reception modifié
 *
 */
int ajout_buffer (uint8_t * buffer, uint8_t ** buffer_recept);

/*
 * Decale la fenetre de reception ou d'envoi
 *
 * @return : 1 si la fenetre n'a pas ete decalee correctement
 *  					0 si la fenetre a ete deplacee correctement
 */
int decale_window(uint8_t len_window, uint8_t * min_window, uint8_t seqnum);

/*
 * Verifie si le buffer est plein ou pas
 *
 * @return : 1 si le buffer est plein
 *  					0 si il reste au moins une place dans le buffer
 */
int buffer_plein(uint8_t ** buffer);

/*
 * Retire un element dans le buffer d'envoi ou de reception
 *
 * @return : 0 si l'element a ete correctement retire du buffer
 * 					 1 si l'element n'a pas ete retire correctement
 */
 int retire_buffer(uint8_t ** buffer, uint8_t seqnum);


/*
 * Vérifie le nombre d'arguments
 */
int arg_check(int argc, int n_min, int n_max);



#endif
