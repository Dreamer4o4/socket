#ifndef _POOL_
#define _POOL_

#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <time.h>


struct task{
    void *(*fun)(void *);
    void *arg;
    struct task *next;
};

struct pool_info{

    struct task *task_front;
    struct task *task_back;

    pthread_mutex_t pth_mtx;
    pthread_mutex_t task_mtx;
    pthread_cond_t ready;

    int task_size;
    int pth_cur_size;
    int pth_max_size;

    pthread_t *pth_no;
    pthread_t pth_admin;
    
    int shutdown;
    int del_num;
};

#define DEFULT_SIZE 10
#define UPDATE_TIME 5

int pth_pool_init(int num);
void pth_pool_destory();
void add_task(void*(*fun)(void *), void *arg);
static void *work_program(void *arg);
static void *admin_program(void *arg);
static void add_task_arg(void **taskarg,void *arg);
static int is_pth_alive(pthread_t pid);


#endif