#ifndef _POOL_
#define _POOL_

#include <pthread.h>

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
    
    volatile int shutdown;
    int del_num;
};

#define MAX_SIZE 10
#define UPDATE_TIME 5
#define LIMITED_TASK_SIZE 10000

int pth_pool_init(int num);
void pth_pool_destory();

/*
**  add task into task queue
*/
int add_task(void*(*fun)(void *), void *arg);

/*
**  two types pthread function  
*/
static void *work_program(void *arg);
static void *admin_program(void *arg);

#endif