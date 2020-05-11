#include <sys/types.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "log.h"
#include "myhttp.h"
#include "connect2socket.h"

static pid_t log_pid = 0;
static int sock_ = 0;

int log_start(){
    FILE *fd = NULL;
    pid_t pid = fork();

    if(pid == 0){
        int sock = 0;
        prctl(PR_SET_NAME, "myhttp-log", NULL, NULL, NULL);

        sock = log_init();
        if(sock <= 0){
            return -1;
        }

        log_program(sock);

    }else{
        sleep(1);

        log_pid = pid;

        char host[] = "localhost";
        int port = atoi(LOG_PORT);
        sock_ = connect2socket(host, port);

        if(sock_ <= 0){
            return -1;
        }

    }
    

    return 0;
}

static int log_init(){
    int sock = 0;

    if(log_test() != 0){
        return -1;
    }

    sock = server_start(LOG_PORT);
    if(sock < 0){
        return -2;
    }

    return sock;
}

static int log_test(){
    FILE *fd = NULL;

    if((fd = fopen(LOG_FILE, "w+")) == NULL){
        perror("LOG_FILE fopen failed");
        return -1;
    }

    if(fclose(fd) != 0){
        perror("LOG_FILE fclose failed");
        return -2;
    }

    return 0;
}

static int log_program(int sock){
    int fd = accept(sock, NULL, 0);
    char buff[LOG_BUFF_SIZE];

    for(;;){
        memset(buff, 0, sizeof(buff));
        read(fd, buff, sizeof(buff));

        log_write(buff);
    }
}

static int log_write(const char *formate){
    FILE *fd = NULL;

    if((fd = fopen(LOG_FILE, "a+")) == NULL){
        perror("LOG_FILE fopen failed");
        return -1;
    }

    fprintf(fd, "%s", formate);

    if(fclose(fd) != 0){
        perror("LOG_FILE fclose failed");
        return -2;
    }

    return 0;
}

static int write_to_log(char *s, int len){
    write(sock_, s, len);

    return 0;
}

int print_with_log(const char *formate, ...){
    va_list args;
    char s[LOG_BUFF_SIZE];
    memset(s, 0, sizeof(s));

    va_start(args, formate);
    vsprintf(s, formate, args);
    va_end(args);

    fprintf(stderr, "%s", s);
    if(sock_ != 0){
        write_to_log(s, sizeof(s));
    }

}
