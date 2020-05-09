#ifndef _HTTP_
#define _HTTP_

#include <netdb.h>

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

static struct client_info *listen_info;

int server_start(const char *port);
static void server_program(int server);
static void *program_core(void *arg);
static void response(struct client_info info, int type);
static void bad_request(int sock);
static int set_no_block(int sock);
static int epoll_init(int listen_fd);
static struct client_info *get_accept_client_info(int listen_fd);
static void pth_work(void *data);
static void add_info_into_epoll(int epfd, struct client_info *info);
#endif