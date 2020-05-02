#ifndef _HTTP_
#define _HTTP_
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <error.h>


#define DEFAULT_PORT "4000"
#define PTH_POOL_SIZE 10
#define BUFF_SIZE 1024
#define RESP_SIZE 1024
#define EPOLL_SIZE 1024

#define GET 0
#define OTHERS 1

struct client_info
{
    int sock;
    char client_host[NI_MAXHOST];
    char client_server[NI_MAXSERV];
};

static int server_start(const char *port);
static void server_program(int server);
static void *program_core(void *arg);
static void response(struct client_info info, int type);
static void bad_request(int sock);
static void *event_happend(void *arg);
static int set_no_block(int sock);
#endif