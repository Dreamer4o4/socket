#ifndef _HTTP_
#define _HTTP_
#include <netdb.h>

#define DEFAULT_PORT "4000"
#define INFO_SIZE 150
#define BUFF_SIZE 1000

#define GET 0
#define OTHERS 1

struct client_info
{
    int sock;
    char client_host[NI_MAXHOST];
    char client_server[NI_MAXSERV];
};



#endif