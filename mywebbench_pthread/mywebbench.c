#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>

#include "mywebbench.h"
#include "bench.h"
// #include "connect2socket.h"

static void help(){
    printf("mywebbench url time clients\n");
    printf("eg:mywebbench http://www.baidu.com/ 10 10 \n");
    printf("defult:\n");
    printf("        url:http://www.baidu.com/\n");
    printf("        time:10\n");
    printf("        clients:10\n");
}
static int get_request(char *url, struct bench_info *request);
static void show_info(struct bench_info request);
static void show_res(struct bench_res arg, int benchtime);

int main(int argc, char *argv[]){
    struct bench_info request;
    char *url;

    if(argc>=2 && !strcmp(argv[1],"help")){
        help();
        return 1;
    }

    url = (argc>1)?argv[1]:"http://www.baidu.com/";
    request.benchtime = (argc>2)?atoi(argv[2]):10;
    request.clients = (argc>3)?atoi(argv[3]):10;

    if(get_request(url, &request)){
        errprintf("get_reuqest\n");
    }

    show_info(request);

    show_res(bench(request), request.benchtime);

    return 0;
}

static int get_request(char *url, struct bench_info *request){
    char *port = NULL;
    memset(request->head, 0, sizeof(request->head));

    strcpy(request->head, "GET");
    strcat(request->head, " ");
    strcat(request->head, "/");
    strcat(request->head, " ");
    strcat(request->head, "HTTP/1.0");
    strcat(request->head, "\r\n");

    strcat(request->head, "User-Agent: MyWebBench v0.1");
    strcat(request->head, "\r\n");

    if(strncmp(url, "http://", 7) != 0 || !strchr(url+7, '/')){
        errprintf("invaild http URL\n");
    }
    url += 7;

    if(index(url, ':') && index(url, ':') < index(url, '/')){
        strncpy(request->host, url, index(url, ':')-url);
        
        char *tmp = NULL;
        strncpy(tmp, index(url,':')+1, index(url, '/')-index(url, ':')-1);
        request->port = atoi(tmp);
    }else{
        strncpy(request->host, url, index(url, '/')-url);

        request->port = 80;
    }
    strcat(request->head, "Host:");
    strcat(request->head, request->host);
    strcat(request->head, "\r\n");

    strcat(request->head,"Connection: close");
    strcat(request->head, "\r\n");

    strcat(request->head, "\r\n");

    return 0;
}

static void show_info(struct bench_info request){
    printf("http-head:\n%s\n",request.head);
    printf("host:%s\n",request.host);
    printf("port:%d\n",request.port);
    printf("clients:%d\n",request.clients);
    printf("benchtime:%d\n",request.benchtime);
    printf("\r\n\r\n");
}

static void show_res(struct bench_res arg, int benchtime){
    printf("benchmark result:\r\n");
    printf("success:%d \r\n",arg.success);
    printf("failed:%d\r\n",arg.failed);
    printf("bytes:%d bytes/sec\r\n",arg.bytes/benchtime);
}
