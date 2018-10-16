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
