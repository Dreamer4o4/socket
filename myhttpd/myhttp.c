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
#include <errno.h>

#include "myhttp.h"
#include "pth_pool.h"
#include "log.h"

/*  
**  select a work type.
**  DEFAULT : only pthread
**  PTH_POOL : prhread pool
**  EPOLL : IO Multiplexing -- event poll
*/
#define PTH_POOL
#define EPOLL

int main(int argc, char *argv[]){
    char *port = NULL;
    int sock;
    
    if(argc>1){
        port = argv[1];
    }else{
        port = DEFAULT_PORT;
    }

    log_start();

#ifdef  PTH_POOL
    if(pth_pool_init(PTH_POOL_SIZE) != 0){
        perror("init pth pool failed");
        #undef PTH_POOL
    }
#endif

    sock = server_start(port);
    if(sock < 0){
        perror("server start failed:");
        exit(-2);
    }

    server_program(sock);

#ifdef  PTH_POOL
    pth_pool_destory();
#endif

    return 0;
}

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
        perror("getaddrinfo:");
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

static void server_program(int server){

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

                pth_work(events[i].data.ptr);

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

        pth_work(info);

    }

#endif

}

static struct client_info *listen_info = NULL;
static int epoll_init(int listen_fd){
    int epfd = epoll_create(EPOLL_SIZE);

    if(epfd == -1){
        perror("epoll create failed");
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
        perror("epoll add listen fd failed");
        exit(-4);
    }

    return epfd;
}

static struct client_info *get_accept_client_info(int listen_fd){
    struct sockaddr_storage client_addr;
    socklen_t client_addr_len = sizeof(struct sockaddr_storage);

    struct client_info *info = (struct client_info *)malloc(sizeof(struct client_info));
    if(info == NULL){
        fprintf(stderr, "client info malloc failed\n");
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

static void pth_work(void *data){
#ifdef  PTH_POOL
    add_task(program_core, data);
#else
    pthread_t pid = 0;
    if(pthread_create(&pid, NULL, program_core, data) != 0){
        perror("pthread_create:\n");
    }else{
        pthread_detach(pid);
    }
#endif
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
		fprintf(stderr,"fcntl F_GETFL failed.%s\n", strerror(errno));
		return 0;
	}
 
	flags |= O_NONBLOCK;
 
	if(fcntl(sock, F_SETFL, flags) == -1)
	{
		fprintf(stderr,"fcntl F_SETFL failed.%s\n", strerror(errno));
		return 0;
	}

    return 1;
}

static void *program_core(void *arg){
    struct client_info client;
    memset(&client, 0, sizeof(struct client_info));
    memcpy(&client, arg, sizeof(struct client_info));

    free(arg);
    arg = NULL;

    char buff[BUFF_SIZE];
    char meth[10];
    int len = 0;
    int rev_len = 0;
    memset(buff, 0, BUFF_SIZE);
    
#ifdef EPOLL
    while((rev_len = recv(client.sock, &buff[len], BUFF_SIZE-len, 0)) > 0){
        len += rev_len;
    }
#else
    len = recv(client.sock, &buff[len], BUFF_SIZE-len, 0);
#endif
    
    fprintf(stderr,"len:%d REC:\n%s\n",len,buff);

    if(len == 0 || (len == BUFF_SIZE && buff[BUFF_SIZE-1] != '\n')){
        bad_request(client.sock);
        return NULL;
    }

    for(int i=0; i<10; i++){
        if(buff[i] != ' '){
            meth[i] = buff[i];
        }else{
            meth[i] = '\0';
            break;
        }
    }
    
    if(strcmp(meth, "GET") == 0){
        response(client, GET);
    }else{
        response(client, OTHERS);
    }

    close(client.sock);


    return NULL;
}

static void response(struct client_info info, int type){
    char response[RESP_SIZE];
    memset(response, 0, RESP_SIZE);
    time_t t = time(NULL);

    if(type == GET){
        strcat(response,"HTTP/1.0 200 OK\r\n");
    }else if(type == OTHERS){
        strcat(response,"HTTP/1.0 501 Method Not Implemented\r\n");
    }

    strcat(response,"Server: httpd/0.1\r\n");
    strcat(response,"Content-Type: text/html\r\n");
    strcat(response,"\r\n");
    strcat(response,"<HTML>");
    strcat(response,"<HEAD><TITLE>zh\r\n</TITLE></HEAD>\r\n");

    if(type == GET){
        strcat(response,"<BODY><h1>HELLO</h1><p>oops!</p>\r\n</BODY>\r\n");
    }else if(type == OTHERS){
        strcat(response,"<BODY><h1>HAVE NOT BUILDED YET</h1><p>oops!</p>\r\n</BODY>\r\n");
    }

    strcat(response,"<BODY><h2>Client Info</h2>");
    strcat(response,"<p>");
    strcat(response,info.client_host);
    strcat(response,"</p>\r\n");
    strcat(response,"<p>");
    strcat(response,info.client_server);
    strcat(response,"</p>\r\n");
    strcat(response,"<p>");
    strcat(response,ctime(&t));
    strcat(response,"</p>\r\n");
    strcat(response,"</BODY>\r\n");

    strcat(response,"</HTML>\r\n");

    write(info.sock, response, strlen(response));
    
    return ;
}

static void bad_request(int sock){
    char response[RESP_SIZE];
    memset(response, 0, RESP_SIZE);

    strcat(response,"HTTP/1.0 400 Bad Request\r\n");
    strcat(response,"Server: httpd/0.1\r\n");
    strcat(response,"Content-Type: text/html\r\n");
    strcat(response,"\r\n");

    write(sock, response, strlen(response));

    return ;
}