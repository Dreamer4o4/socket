#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>

#include "myhttp.h"
#include "pth_pool.h"
#include "log.h"
#include "server.h"


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

    server_program(sock, program_core);

#ifdef  PTH_POOL
    pth_pool_destory();
#endif

    return 0;
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
    // print_with_log("len:%d REC:\n%s\n",len,buff);

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
    strcat(response,"<HTML>\r\n");
    strcat(response,"<HEAD><TITLE>zh</TITLE></HEAD>\r\n");

    strcat(response,"<BODY>\r\n");
    if(type == GET){
        strcat(response,"<h1>HELLO</h1>\r\n");
        // strcat(response,"<h2>select reservation or query</h2>\r\n");
        // strcat(response,"<a href=\"http://localhost/reservation.html\">\r\n");
        // strcat(response,"<button>reservation</button>\r\n");
        // strcat(response,"</a>\r\n");
        // strcat(response,"<br>\r\n");
        // strcat(response,"<a href=\"http://localhost/query.html\">\r\n");
        // strcat(response,"<button>query</button>\r\n");
        // strcat(response,"</a>\r\n");
    }else if(type == OTHERS){
        strcat(response,"<h1>HAVE NOT BUILDED YET</h1>\r\n");
        strcat(response,"<p>oops!</p>\r\n");
    }
    strcat(response,"</BODY>\r\n");

    strcat(response,"<BODY>\r\n");
    strcat(response,"<h2>Client Info</h2>\r\n");
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