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
