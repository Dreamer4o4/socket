#include "log.h"

static int flag = 0;

int log_init(){
    FILE *fd = NULL;
    
    if((fd = fopen("log.txt", "w+")) == NULL){
        perror("log.txt fopen failed");
        return -1;
    }

    if(fclose(fd) != 0){
        perror("log.txt fclose failed");
        return -2;
    }

    flag = 1;

    return 0;
}

int log_write(const char *formate, ...){
    if(flag != 1){
        return -1;
    }

    FILE *fd = NULL;
    va_list args;

    if((fd = fopen("log.txt", "a+")) == NULL){
        perror("log.txt fopen failed");
        return -1;
    }

    va_start(args, formate);
    fprintf(fd, formate, args);
    va_end(args);

    if(fclose(fd) != 0){
        perror("log.txt fclose failed");
        return -2;
    }

    return 0;
}