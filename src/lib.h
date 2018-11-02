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

typedef struct ack ack_t;

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

/*
* pkt_new : Fonction qui crée un nouveau paquet de type PTYPE_DATA
*
* @return : un nouveau paquet de type PTYPE_DATA ou NULL en cas d'erreur
*/
pkt_t* pkt_new();

/*
* pkt_ack_new : Fonction qui crée un nouveau paquet de type PTYPE_ACK
*
* @return : un nouveau paquet de type PTYPE_ACK ou NULL en cas d'erreur
*/
ack_t* ack_new();

/*
* pkt_del : Libere le pointeur vers la struct pkt, ainsi que toutes les
* ressources associees
*
* @pkt : pointeur vers un paquet
* @return : /
*/
void pkt_del(pkt_t* pkt);

/*
* pkt_get_type : Fonction qui va chercher le type du paquet place en argument
*
* @pkt : pointeur vers un paquet
* @return : le type du paquet
*/
ptypes_t pkt_get_type     (const pkt_t* pkt);

/*
* pkt_get_tr : Fonction qui va chercher le bit qui correspond
* au troncage du paquet placé en argument
*
* @pkt : pointeur vers un paquet
* @return : le bit qui correspond au troncage du paquet
*/
uint8_t  pkt_get_tr       (const pkt_t* pkt);

/*
* pkt_get_window : Fonction qui va chercher la longueur de la fenetre de
* l'emetteur du paquet place en argument
*
* @pkt : pointeur vers un paquet
* @return : la longueur de la fenetre glissante
*/
uint8_t  pkt_get_window   (const pkt_t* pkt);

/*
* pkt_get_seqnum : Fonction qui va chercher le numero de sequence
* du paquet place en argument
*
* @pkt : pointeur vers un paquet
* @return : le numero de sequence du paquet
*/
uint8_t  pkt_get_seqnum   (const pkt_t* pkt);

/*
* pkt_get_length: Fonction qui va chercher la longueur du payload
* du paquet place en argument
*
* @pkt : pointeur vers un paquet
* @return : la longueur du payload
*/
uint16_t pkt_get_length   (const pkt_t* pkt);

/*
* pkt_get_timestamp: Fonction qui va chercher le timestamp
* du paquet place en argument
*
* @pkt : pointeur vers un paquet
* @return : le timestamp
*/
uint32_t pkt_get_timestamp(const pkt_t* pkt);

/*
* pkt_get_crc1: Fonction qui va chercher la valeur du CRC1
* du paquet place en argument
*
* @pkt : pointeur vers un paquet
* @return : la valeur du CRC1
*/
uint32_t pkt_get_crc1     (const pkt_t* pkt);

/*
* pkt_get_crc2: Fonction qui va chercher la valeur du CRC2
* du paquet place en argument
*
* @pkt : pointeur vers un paquet
* @return : la valeur du CRC2 ou 0 si il n'y a pas de CRC2
*/
uint32_t pkt_get_crc2(const pkt_t* pkt);

/*
* pkt_get_payload: Fonction qui va chercher l'information du payload
* du paquet place en argument
*
* @pkt : pointeur vers un paquet
* @return : un pointeur vers les données contenues dans le payload
*/
const char* pkt_get_payload(const pkt_t* pkt);

/*
* pkt_set_type : Fonction qui va initialiser le type du paquet en arguments
* a une certaine valeur
*
* @type : valeur a laquelle le type du paquet doit etre initialisee
* @pkt : pointeur vers un paquet
* @return : Un code indiquant si l'operation a reussi ou representant
* l'erreur rencontree
*/
pkt_status_code pkt_set_type     (pkt_t* pkt, const ptypes_t type);

/*
* pkt_set_tr : Fonction qui va initialiser le bit de troncage du paquet en arguments
* a une certaine valeur
*
* @tr : valeur a laquelle le troncage du paquet doit etre initialisee
* @pkt : pointeur vers un paquet
* @return : Un code indiquant si l'operation a reussi ou representant
* l'erreur rencontree
*/
pkt_status_code pkt_set_tr       (pkt_t* pkt, const uint8_t tr);

/*
* pkt_set_window : Fonction qui va initialiser la taille de la fenetre du
* paquet en arguments a une certaine valeur
*
* @window : valeur a laquelle la fenetre du paquet doit etre initialisee
* @pkt : pointeur vers un paquet
* @return : Un code indiquant si l'operation a reussi ou representant
* l'erreur rencontree
*/
pkt_status_code pkt_set_window   (pkt_t* pkt, const uint8_t window);

/*
* pkt_set_seqnum : Fonction qui va initialiser le numero de sequence du
* paquet en arguments a une certaine valeur
*
* @seqnum : valeur a laquelle le numero de sequence du paquet doit etre initialisee
* @pkt : pointeur vers un paquet
* @return : Un code indiquant si l'operation a reussi ou representant
* l'erreur rencontree
*/
pkt_status_code pkt_set_seqnum   (pkt_t* pkt, const uint8_t seqnum);

/*
* pkt_set_length : Fonction qui va initialiser la longueur du payload du
* paquet en arguments a une certaine valeur
*
* @length : valeur a laquelle la longueur du payload du paquet doit etre initialisee
* @pkt : pointeur vers un paquet
* @return : Un code indiquant si l'operation a reussi ou representant
* l'erreur rencontree
*/
pkt_status_code pkt_set_length   (pkt_t* pkt, const uint16_t length);

/*
* pkt_set_timestamp : Fonction qui va initialiser le timestamp du
* paquet en arguments a une certaine valeur
*
* @timestamp : valeur a laquelle le timestamp du paquet doit etre initialise
* @pkt : pointeur vers un paquet
* @return : Un code indiquant si l'operation a reussi ou representant
* l'erreur rencontree
*/
pkt_status_code pkt_set_timestamp(pkt_t* pkt);

/*
* pkt_set_crc1 : Fonction qui va initialiser la valeur du CRC1 du
* paquet en arguments a une certaine valeur
*
* @crc1 : valeur a laquelle le CRC1 du paquet doit etre initialisee
* @pkt : pointeur vers un paquet
* @return : Un code indiquant si l'operation a reussi ou representant
* l'erreur rencontree
*/
pkt_status_code pkt_set_crc1     (pkt_t* pkt, const uint32_t crc1);

/*
* pkt_set_crc2 : Fonction qui va initialiser la valeur du CRC2 du
* paquet en arguments a une certaine valeur
*
* @crc2 : valeur a laquelle le CRC2 du paquet doit etre initialisee
* @pkt : pointeur vers un paquet
* @return : Un code indiquant si l'operation a reussi ou representant
* l'erreur rencontree
*/
pkt_status_code pkt_set_crc2(pkt_t* pkt, const uint32_t crc2);

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
pkt_status_code pkt_set_payload(pkt_t* pkt, const char *data, const uint16_t length);


/*
* pkt_encode : Encode une struct pkt dans un buffer, pret a etre envoye sur le reseau
* (c-a-d en network byte-order), incluant le CRC32 du header et
* eventuellement le CRC32 du payload si celui-ci est non nul.
*
* @pkt: La structure a encoder
* @buf: Le buffer dans lequel la structure sera encodee
* @len: La taille disponible dans le buffer
* @return: Un code indiquant si l'operation a reussi ou E_NOMEM si
* le buffer est trop petit.
*/
pkt_status_code pkt_encode(const pkt_t* pkt, uint8_t *buf, size_t len);


/*
* pkt_encode : Encode une struct ack dans un buffer, pret a etre envoye sur le reseau
* (c-a-d en network byte-order), incluant le CRC32 du header et
*
* @ack: La structure a encoder
* @buf: Le buffer dans lequel la structure sera encodee
* @len: La taille disponible dans le buffer
* @return: Un code indiquant si l'operation a reussi ou E_NOMEM si
* le buffer est trop petit.
*/
pkt_status_code ack_encode(const ack_t* ack, uint8_t *buf, size_t len);

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
pkt_status_code pkt_decode(uint8_t *data, const size_t len, pkt_t *pkt);


/*
* ack_decode : Decode des donnees recues et cree une nouvelle structure ack.
* Le paquet recu est en network byte-order.
* La fonction verifie que:
* - Le CRC32 du header recu est le même que celui decode a la fin
*   du header (en considerant le champ TR a 0)
* - Le type du paquet est valide
* - La longueur du paquet et le champ TR sont valides et coherents
*   avec le nombre d'octets recus.
*
* @data: L'ensemble d'octets constituant le paquet recu
* @len: Le nombre de bytes recus
* @pkt: Une struct ack valide
* @post: ack est la representation du paquet recu
* @return: Un code indiquant si l'operation a reussi ou representant
* l'erreur rencontree
*/
pkt_status_code ack_decode(uint8_t *data, const size_t len, ack_t *ack);

/*
* real_address : Trouve le nom de la ressource correspondant à une adresse IPv6
*
* @address: L'adresse a trouver
* @rval: Où le resultat de la description de l'adresse IPv6 sera stockee
*
* @return: - NULL si tout s'est bien deroule
*          - un pointeur vers un string decrivant l'erreur sinon
*/
const char * real_address(const char *address, struct sockaddr_in6 *rval);

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
int create_socket(struct sockaddr_in6 *source_addr,
	int src_port,
	struct sockaddr_in6 *dest_addr,
	int dst_port);


	/*
	* wait_for_client : Bloque le receveur jusqu'a ce qu'un message soit recu sur sfd
	* et connecte le socket a l'adresse source du message recu
	*
	* @sfd: un file descriptor vers un socket lie mais pas connecte
	*
	* @return: - 0 en cas de succes
	*				  -1 otherwise
	*/
	int wait_for_client(int sfd);


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
	int seqnum_inc(uint8_t* seqnum);


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
	int in_window (uint8_t seqnum, uint8_t min_window, uint8_t max_window);


	/*
	* decale_window : Decale la fenetre de reception ou d'envoi
	*
	* @min_window : un pointeur vers le plus petit numero de sequence present dans la fenetre
	* @max_window : un pointeur vers le plus grand numero de sequence present dans la fenetre
	*
	* @return : /
	*/
	void decale_window(uint8_t *min_window, uint8_t *max_window);


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
	void ajout_buffer (pkt_t* pkt, pkt_t** buffer_recept, uint8_t min_window);


	/*
	* buffer_plein : Verifie si le buffer est plein ou pas
	*
	* @buffer : buffer de paquets a parcourir
	*
	* @return : - 1 si le buffer est plein
	*  					- 0 si il reste au moins une place dans le buffer
	*/
	int buffer_plein(pkt_t ** buffer);


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
	pkt_t* get_from_buffer(pkt_t ** buffer, uint8_t seqnum);

	/*
	* retire_buffer : Retire un paquet dans le buffer d'envoi ou de reception
	*
	* @buffer : buffer de paquets
	* @seqnum : numero de sequence du paquet a retirer du buffer
	*
	* @return : - 0 si l'element a ete correctement retire du buffer
	* 					 - 1 si l'element n'a pas ete retire correctement
	*/
	int retire_buffer(pkt_t ** buffer, uint8_t seqnum);

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
	int write_buffer(int fd, pkt_t **buffer, uint8_t *min_window, uint8_t *max_window);


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
	int arg_check(int argc, int n_min, int n_max);



	#endif
