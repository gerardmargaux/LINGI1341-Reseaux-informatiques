#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>


/* Block the caller until a message is received on sfd,
 * and connect the socket to the source addresse of the received message.
 * @sfd: a file descriptor to a bound socket but not yet connected
 * @return: 0 in case of success, -1 otherwise
 * @POST: This call is idempotent, it does not 'consume' the data of the message,
 * and could be repeated several times blocking only at the first call.
 */
int wait_for_client(int sfd){
	int err;
  struct sockaddr_in6 address;
	struct sockaddr * addr_cast = (struct sockaddr *)&address;
  memset(&address, 0, sizeof(struct sockaddr_in6)); // On remplit avec des 0 la structure vers laquelle pointe address
  char * buffer = (char *)malloc(1024*sizeof(char));

	// Reception du message du client par le serveur
  err = recvfrom(sfd, buffer, strlen(buffer), MSG_PEEK, addr_cast, (sizeof(struct sockaddr_in6)));
  if (err < 0){
    fprintf(stderr,"erreur de reception du message");
    return -1;
  }

	// Connexion entre le serveur et le client
  err = connect(sfd, addr_cast, sizeof(struct sockaddr_in6));
  if (err < 0){
    fprintf(stderr,"erreur de connextion ");
  }

  return err;
}