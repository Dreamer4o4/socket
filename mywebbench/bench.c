#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "bench.h"
#include "connect2socket.h"

#define MAX_BUFF 1000

static int success = 0;
static int failed = 0;
static int bytes = 0;

#define SUCCESS 1
#define FAILED  2
#define BYTES   3

pthread_mutex_t bench_mtx = PTHREAD_MUTEX_INITIALIZER;

static void *bench_core(void *arg);
static int getbenchres(int type);
static void addbenchres(int type, int num);


struct bench_res bench(struct bench_info request){
    int sock;
    pthread_t *pth_pid;
    struct bench_res res;

    sock = connect2socket(request.host, request.port);
    if(sock<0){
        errprintf("failed to connect2socket\n");
    }
    close(sock);

    pth_pid = (pthread_t *)malloc(request.clients * sizeof(pthread_t));
    memset(pth_pid, 0, request.clients * sizeof(pthread_t));
    for(int i=0; i<request.clients; i++){
        if(pthread_create(&pth_pid[i], NULL, bench_core, &request) != 0){
            continue;
        }
        pthread_detach(pth_pid[i]);
    }
    
    sleep(request.benchtime);

    res.success = getbenchres(SUCCESS);
    res.failed = getbenchres(FAILED);
    res.bytes = getbenchres(BYTES);

    return res;
}

static void *bench_core(void *arg){
    struct bench_info *request = (struct bench_info *)arg;
    int writelen = strlen(request->head),readlen = 0;
    int sock;
    char buffer[MAX_BUFF];

    while(1){
        sock = connect2socket(request->host,request->port);
        if(sock<0){
            continue;
        }

        if(write(sock, request->head, writelen) != writelen){
            addbenchres(FAILED,1);
            close(sock);
            continue;
        }

        while((readlen = read(sock, buffer, MAX_BUFF)) > 0){
            addbenchres(BYTES,readlen);
            continue;
        }
        if(readlen < 0){
            addbenchres(FAILED,1);
        }else{
            addbenchres(SUCCESS,1);
        }

        // printf("buffer:%s\r\n",buffer);
        close(sock);
    }
}


static int getbenchres(int type){
    switch (type){
    case SUCCESS:
        return success;
    case FAILED:
        return failed;
    case BYTES:
        return bytes;
    default:
        break;
    }
    return 0;
}

static void addbenchres(int type, int num){
    pthread_mutex_lock(&bench_mtx);
    switch (type){
    case SUCCESS:
        success+=num;
        break;
    case FAILED:
        failed+=num;
        break;
    case BYTES:
        bytes+=num;
        break;
    default:
        break;
    }
    pthread_mutex_unlock(&bench_mtx);
    return;
}