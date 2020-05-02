#ifndef _POOL_
#define _POOL_

#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <signal.h>


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

#define MAX_SIZE 10
#define UPDATE_TIME 5
#define LIMITED_TASK_SIZE 10240

int pth_pool_init(int num);
void pth_pool_destory();
void add_task(void*(*fun)(void *), void *arg);
static void *work_program(void *arg);
static void *admin_program(void *arg);
static void set_signal_exit();
static void sigint_hander(int signo);

#endif