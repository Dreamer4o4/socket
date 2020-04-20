#include "myhttp.h"
#include "pth_pool.h"

/*  select a work type.
**  PTH : only pthread
**  PTH_POOL : prhread pool
*/
#define PTH_POOL

int main(int argc, char *argv[]){
    char *port = NULL;
    int sock;
    
    if(argc>1){
        port = argv[1];
    }else{
        port = DEFAULT_PORT;
    }

#ifdef  PTH_POOL
    pth_pool_init(100);
#endif

    sock = server_start(port);
    if(sock < 0){
        perror("server start failed:");
        exit(1);
    }

    server_program(sock);

#ifdef  PTH_POOL
    pth_pool_destory();
#endif

    return 0;
}

static int server_start(const char *port){
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
    struct client_info info;
    struct sockaddr_storage client_addr;
    socklen_t client_addr_len = sizeof(struct sockaddr_storage);
    pthread_t pid = 0;

    for(;;){
        // memset(&client_addr, 0, sizeof(struct sockaddr));
        info.sock = accept(server, (struct sockaddr*)&client_addr, &client_addr_len);
        if(info.sock < 0){
            continue;
        }

        if(getnameinfo((struct sockaddr*)&client_addr, client_addr_len, info.client_host, NI_MAXHOST, info.client_server, NI_MAXSERV, 0) != 0){
            strcpy(info.client_host, "unkonw");
            strcpy(info.client_server, "unkonw");
        }

#ifdef  PTH
        if(pthread_create(&pid, NULL, program_core, &info) != 0){
            perror("pthread_create:\n");
        }else{
            pthread_detach(pid);
        }
#endif

#ifdef PTH_POOL
        add_task(program_core, &info);
#endif

    }

    return ; 
}

static void *program_core(void *arg){
    // struct client_info client = *(struct client_info *)arg;
    struct client_info client;
    memset(&client, 0, sizeof(struct client_info));
    memcpy(&client, arg, sizeof(struct client_info));

    char buff[BUFF_SIZE];
    char meth[10];
    int len = 0;

    memset(buff, 0, BUFF_SIZE);
    len = recv(client.sock, buff, BUFF_SIZE, 0);
    if(buff[BUFF_SIZE-1] != 0 || (len == BUFF_SIZE && buff[BUFF_SIZE-1] != '\n')){
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
    fprintf(stderr,"client:%s %s\nrcv:\n%s\n", client.client_host, 
                            client.client_server, buff);

    return NULL;
}

static void response(struct client_info info, int type){
    char response[1024];
    memset(response, 0, 1024);
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
    char response[1024];

    strcat(response,"HTTP/1.0 400 Bad Request\r\n");
    strcat(response,"Server: httpd/0.1\r\n");
    strcat(response,"Content-Type: text/html\r\n");
    strcat(response,"\r\n");

    write(sock, response, strlen(response));

    return ;
}