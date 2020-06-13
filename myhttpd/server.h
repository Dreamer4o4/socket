#ifndef _SERVER_
#define _SERVER_

#include "myhttp.h"

/*
**  socket bind listen  
*/
int server_start(const char *port);

/*
**  reactor
*/
void server_program(int server, void *(*program_core)(void *));


/***************   local function    ******************/

/*
**  get peer address info
*/
static struct client_info *get_accept_client_info(int listen_fd);

static int epoll_init(int listen_fd);

static int set_no_block(int sock);

/*
**  put accept sock into epoll
*/
static void add_info_into_epoll(int epfd, struct client_info *info);

/*
**  different ways to handle EPOLLIN accept sock
*/
static void pth_work(void *data, void *(*program_core)(void *));

#endif