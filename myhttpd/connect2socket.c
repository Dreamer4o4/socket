#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "connect2socket.h"

int connect2socket(const char *host, int port){
    int sock;
    char cport[20];
    struct addrinfo hints;
    struct addrinfo *result,*tmp;
    int err;

    sprintf(cport,"%d",port);

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_flags = AI_NUMERICSERV;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_addrlen = 0;
    hints.ai_addr = NULL;
    hints.ai_canonname = NULL;
    hints.ai_next = NULL;
    if((err = getaddrinfo(host, cport, &hints, &result)) != 0){
        fprintf(stderr, "getaddrinfo failed\n");
        return -1;
    }

    for(tmp = result; tmp != NULL; tmp = tmp->ai_next){
        sock = socket(tmp->ai_family, tmp->ai_socktype, tmp->ai_protocol);
        if(sock < 0){
            continue;
        }

        if(connect(sock, tmp->ai_addr, tmp->ai_addrlen) == 0){
            break;
        }

        close(sock);
    }
    if(tmp == NULL){
        fprintf(stderr, "connect failed\n");
        return -1;
    }

    freeaddrinfo(result);

    return sock;
}