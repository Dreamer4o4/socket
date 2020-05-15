#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>


#include "server.h"
#include "myhttp.h"
#include "pth_pool.h"
#include "log.h"

static struct client_info *listen_info = NULL;


int server_start(const char *port){
    int sock, optval = 1;
    struct addrinfo hint;
    struct addrinfo *res, *tmp;

    memset(&hint, 0, sizeof(struct addrinfo));
    hint.ai_flags = AI_PASSIVE | AI_NUMERICSERV;
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_protocol = 0;
    hint.ai_addrlen = 0;
    hint.ai_addr = NULL;
    hint.ai_canonname = NULL;
    hint.ai_next = NULL;
    if(getaddrinfo(NULL, port, &hint, &res) != 0){
        // perror("getaddrinfo:");
        print_with_log("getaddrinfo failed\r\n");
        return -1;
    }

    for(tmp = res; tmp != NULL; res = res->ai_next, tmp = res){
        sock = socket(tmp->ai_family, tmp->ai_socktype, tmp->ai_protocol);
        if(sock < 0){
            continue;
        }

        if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1){
            close(sock);
            continue;
        }

        if(bind(sock, tmp->ai_addr, tmp->ai_addrlen) != 0){
            close(sock);
            continue;
        }

        break;
    }
    if(tmp == NULL){
        return -1;
    }
    freeaddrinfo(res);

    if(listen(sock, 10) != 0){
        close(sock);
        return -1;
    }

    return sock;
}

void server_program(int server, void *(*program_core)(void *)){

#ifdef EPOLL

    int epfd = epoll_init(server);

    for(;;){
        struct epoll_event ev,events[EPOLL_SIZE];
        int num = epoll_wait((long)epfd, events, EPOLL_SIZE, -1);
        if(num == -1){
            continue;
        }

        for(int i=0; i < num; i++){
            if((events[i].events & EPOLLIN) && ((struct client_info *)events[i].data.ptr)->sock == server){
                
                struct client_info *info = get_accept_client_info(server);
                if(info == NULL){
                    continue;
                }

                add_info_into_epoll(epfd, info);
            
            }else if(events[i].events & EPOLLIN){

                epoll_ctl((long)epfd, EPOLL_CTL_DEL, ((struct client_info *)events[i].data.ptr)->sock, &ev);

                pth_work(events[i].data.ptr, program_core);

            }else if(events[i].events & EPOLLOUT){
                ;
            }else{
                ;
            }

        }
    }

    close(epfd);

    free(listen_info);
    listen_info = NULL;

#else

    for(;;){

        struct client_info *info = get_accept_client_info(server);

        pth_work(info, program_core);

    }

#endif

}

static struct client_info *get_accept_client_info(int listen_fd){
    struct sockaddr_storage client_addr;
    socklen_t client_addr_len = sizeof(struct sockaddr_storage);

    struct client_info *info = (struct client_info *)malloc(sizeof(struct client_info));
    if(info == NULL){
        // fprintf(stderr, "client info malloc failed\n");
        print_with_log("client info malloc failed\n");
        return NULL;
    }

    info->sock = accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len);
    if(info->sock < 0){
        free(info);
        info = NULL;
        return NULL;
    }

    if(getnameinfo((struct sockaddr*)&client_addr, client_addr_len, info->client_host, NI_MAXHOST, info->client_server, NI_MAXSERV, 0) != 0){
        strcpy(info->client_host, "unkonw");
        strcpy(info->client_server, "unkonw");
    }

    return info;
}


static int epoll_init(int listen_fd){
    int epfd = epoll_create(EPOLL_SIZE);

    if(epfd == -1){
        // perror("epoll create failed");
        print_with_log("epoll create failed\n");
        exit(-3);
    }
    
    struct client_info *info = (struct client_info *)malloc(sizeof(struct client_info));
    memset(info, 0, sizeof(struct client_info));
    info->sock = listen_fd;
    listen_info = info;

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.ptr = info;
    if(epoll_ctl(epfd, EPOLL_CTL_ADD, info->sock, &ev) == -1){
        // perror("epoll add listen fd failed");
        print_with_log("epoll add listen fd failed\n");
        exit(-4);
    }

    return epfd;
}

static void add_info_into_epoll(int epfd, struct client_info *info){
    if(!set_no_block(info->sock)){
        close(info->sock);
        free(info);
        info = NULL;
        return;
    }

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.ptr = info;
    epoll_ctl(epfd, EPOLL_CTL_ADD, info->sock, &ev);
}

static int set_no_block(int sock){
    int flags;
	
	flags = fcntl(sock, F_GETFL, NULL);
	if(flags == -1)
	{
		// fprintf(stderr,"fcntl F_GETFL failed.%s\n", strerror(errno));
        print_with_log("fcntl F_GETFL failed.%s\n", strerror(errno));
		return 0;
	}
 
	flags |= O_NONBLOCK;
 
	if(fcntl(sock, F_SETFL, flags) == -1)
	{
		// fprintf(stderr,"fcntl F_SETFL failed.%s\n", strerror(errno));
        print_with_log("fcntl F_SETFL failed.%s\n", strerror(errno));
		return 0;
	}

    return 1;
}

static void pth_work(void *data, void *(*program_core)(void *)){
#ifdef  PTH_POOL
    add_task(program_core, data);
#else
    pthread_t pid = 0;
    if(pthread_create(&pid, NULL, program_core, data) != 0){
        // perror("pthread_create:\n");
        print_with_log("pthread_create failed\r\n");
    }else{
        pthread_detach(pid);
    }
#endif
}