#include "create_socket.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int count_digits(int n){
    int digits = 0;
    while(n >= 1){
        digits++;
        n = n/10;
    }
    return digits;
}

int create_socket(struct sockaddr_in6 *source_addr, int src_port, struct sockaddr_in6 *dest_addr, int dst_port){

    int err;
    struct protoent *protocol = IPPROTO_UDP;
    int udp = protocol->p_proto;
    int sd;
    int digits;

    if(source_addr != NULL){
        sd = socket(PF_INET6, SOCK_DGRAM, udp);
        if(sd < 0){
            fprintf(stderr, "ERROR : fonction socket()\n");
            return -1;
        }
        err = connect(sd, (struct sockaddr*) source_addr, sizeof((source_addr->sin6_addr).s6_addr));
        if(err < 0){
            fprintf(stderr, "ERROR : source_addr\n");
            return -1;
        }
    }

    else if(src_port > 0){
        struct addrinfo hints, *res;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET6;
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_protocol = udp;
        hints.ai_flags = AI_PASSIVE;

        digits = count_digits(src_port);
        char* string = (char*) malloc(sizeof(char)*digits);
        if(string == NULL){
            fprintf(stderr, "ERROR : fonction malloc()\n");
            return -1;
        }
        sprintf(string, "%d", htons(src_port));

        err = getaddrinfo(NULL, string, &hints, &res);
        if(err != 0){
            fprintf(stderr, "ERROR = fonction getaddrinfo()\n");
            return -1;
        }

        sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if(sd < 0){
            fprintf(stderr, "ERROR : fonction socket()\n");
            return -1;
        }

        err = bind(sd, res->ai_addr, res->ai_addrlen);
        if(err != 0){
            fprintf(stderr, "ERROR : fonction bind()\n");
            return -1;
        }
    }

    else if(dest_addr != NULL){
        sd = socket(PF_INET6, SOCK_DGRAM, udp);
        if(sd < 0){
            fprintf(stderr, "ERROR : fonction socket()\n");
            return -1;
        }
        err = connect(sd, (struct sockaddr*) dest_addr, sizeof((source_addr->sin6_addr).s6_addr));
        if(err < 0){
            fprintf(stderr, "ERROR : dest_addr\n");
            return -1;
        }
    }

    else if(dst_port > 0){
        struct addrinfo hints, *res;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET6;
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_protocol = udp;
        hints.ai_flags = AI_PASSIVE;

        digits = count_digits(dst_port);
        char* string = (char*) malloc(sizeof(char)*digits);
        if(string == NULL){
            fprintf(stderr, "ERROR : fonction malloc()\n");
            return -1;
        }
        sprintf(string, "%d", htons(dst_port));

        err = getaddrinfo(NULL, string, &hints, &res);
        if(err != 0){
            fprintf(stderr, "ERROR = fonction getaddrinfo()\n");
            return -1;
        }

        sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if(sd < 0){
            fprintf(stderr, "ERROR : fonction socket()\n");
            return -1;
        }

        err = bind(sd, res->ai_addr, res->ai_addrlen);
        if(err != 0){
            fprintf(stderr, "ERROR : fonction bind()\n");
            return -1;
        }
    }

    else{
        fprintf(stderr, "ERROR : format des arguments\n");
        return -1;
    }

    return sd;
}
