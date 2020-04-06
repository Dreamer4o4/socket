#ifndef _BENCH_
#define _BENCH_

#include "mywebbench.h"

struct bench_res{
    int success;
    int failed;
    int bytes;
};

struct bench_res bench(struct bench_info request);

#endif