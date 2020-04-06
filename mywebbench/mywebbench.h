#ifndef _MYWEBBENCH_
#define _MYWEBBENCH_

#define errprintf(msg, arg...) {fprintf(stderr, msg, ##arg);exit(1);}

struct bench_info{
    char head[200];
    char host[100];
    int port;
    int benchtime;
    int clients;
};

#endif