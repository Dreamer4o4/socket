#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "pth_pool.h"
#include "log.h"

static struct pool_info *info;


/*
**  VAR_SIZE : the pthread pool size is variable
*/
// #define VAR_SIZE

int pth_pool_init(int num){

    num = (num>MAX_SIZE)?MAX_SIZE:num;

    do{
        info = (struct pool_info *)malloc(sizeof(struct pool_info));
        if(info == NULL){
            fprintf(stderr, "pthread pool info malloc failed\n");
            break;
        }

        info->pth_no = (pthread_t *)malloc(sizeof(pthread_t) * num);
        if(info->pth_no == NULL){
            fprintf(stderr, "pthread pool pth_no malloc failed\n");
            break;
        }
        memset(info->pth_no, 0, sizeof(pthread_t) * num);
        info->pth_admin = 0;

        if(pthread_mutex_init(&(info->task_mtx), NULL) != 0 || 
            pthread_mutex_init(&(info->pth_mtx), NULL) != 0 || 
            pthread_cond_init(&(info->ready), NULL) != 0){
            fprintf(stderr, "pthread pool mtx or cond init failed\n");
            break;
        }

        info->pth_max_size = num;
        info->pth_cur_size = 0;
        info->shutdown = 0;
        info->del_num = 0;
        
        info->task_size = 0;
        info->task_front = NULL;
        info->task_back = NULL;

#ifdef  VAR_SIZE
        int start_size = (info->pth_max_size/2);
#else
        int start_size = info->pth_max_size;
#endif
        for(int i=0; i<start_size; i++){
            pthread_create(&(info->pth_no[i]), NULL, work_program, (void *)(long)i);
        }

        if(pthread_create(&(info->pth_admin), NULL, admin_program, NULL) != 0){
            fprintf(stderr, "admin pth create failed\n");
            break;
        }

        return 0;
    }while(0);
    
    pth_pool_destory();
    return -1;
}

void pth_pool_destory(){
    if(info == NULL){
        return;
    }

    info->shutdown = 1;

    pthread_cond_broadcast(&(info->ready));

    while(info->pth_cur_size != 0){
        fprintf(stderr,"%d pths is exitting\n",info->pth_cur_size);
        sleep(1);
    }
    free(info->pth_no);
    info->pth_no = NULL;

    pthread_mutex_destroy(&(info->task_mtx));
    pthread_mutex_destroy(&(info->pth_mtx));
    pthread_cond_destroy(&(info->ready));

    while(info->task_front != NULL){
        struct task *tmp = info->task_front;
        info->task_front = info->task_front->next;
        free(tmp->arg);
        tmp->arg = NULL;
        free(tmp);
        tmp = NULL;
    }

    free(info);
    info = NULL;

    return;
}

static void *work_program(void *arg){
    pthread_detach(pthread_self());

    pthread_mutex_lock(&(info->pth_mtx));
    info->pth_cur_size++;
    pthread_mutex_unlock(&(info->pth_mtx));

    int exit_flag = 0;

    for(;;){
        struct task *tmp;

        pthread_mutex_lock(&(info->task_mtx));
        while(info->task_size == 0){
            pthread_cond_wait(&(info->ready), &(info->task_mtx));

            pthread_mutex_lock(&(info->pth_mtx));
            if(info->del_num == 0 && !info->shutdown){
                pthread_mutex_unlock(&(info->pth_mtx));
                continue;
            }
            if(info->del_num != 0){
                info->del_num--;
            }
            if(info->pth_cur_size >= (info->pth_max_size/2) || info->shutdown){
                info->pth_cur_size--;
                info->pth_no[(long)arg] = 0;
                exit_flag = 1;
            }
            pthread_mutex_unlock(&(info->pth_mtx));
            if(exit_flag){
                break;
            }
        }
        if(exit_flag){
            pthread_mutex_unlock(&(info->task_mtx));
            break;
        }

        tmp = info->task_front;
        info->task_front = info->task_front->next;
        info->task_size--;

        fprintf(stderr,"%ld do work,cur size %d\n",pthread_self(),info->task_size);

        pthread_mutex_unlock(&(info->task_mtx));

        (*(tmp->fun))(tmp->arg);

        fprintf(stderr,"%ld work finished\n",pthread_self());

        free(tmp);
        tmp = NULL;

    }

}

static void *admin_program(void *arg){
    pthread_detach(pthread_self());
    
    time_t t;

    while(!info->shutdown){
        int num = 0;
        sleep(UPDATE_TIME);
        if(info->shutdown){
            break;
        }

        t = time(NULL);
        fprintf(stderr,"time:%s",ctime(&t));
        pthread_mutex_lock(&(info->pth_mtx));
        pthread_mutex_lock(&(info->task_mtx));
        fprintf(stderr,"cur pth:%d,cur task:%d\n",info->pth_cur_size,info->task_size);

#ifdef  VAR_SIZE
        if(2 * info->pth_cur_size < info->task_size || info->pth_cur_size < (info->pth_max_size/2)){
            int top = (info->task_size/2 > info->pth_max_size)?(info->pth_max_size):(info->task_size/2);
            num = ((top > (info->pth_max_size/2))?top:(info->pth_max_size/2)) - info->pth_cur_size;
        }else if(info->pth_cur_size > info->task_size || info->pth_cur_size > info->pth_max_size){
            int bottom = (info->task_size > (info->pth_max_size/2))?(info->task_size):((info->pth_max_size/2));
            num = ((bottom > info->pth_max_size)?info->pth_max_size:bottom) - info->pth_cur_size;
        }
#endif
        pthread_mutex_unlock(&(info->task_mtx));
        pthread_mutex_unlock(&(info->pth_mtx));

#ifdef  VAR_SIZE
        if(num > 0){
            for(int i=0,j=0; i<info->pth_max_size && j<num; i++){
                if(info->pth_no[i] == 0){
                    if(!pthread_create(&(info->pth_no[i]), NULL, work_program, (void *)(long)i)){
                        j++;
                    }
                }
            }
        }else if(num<0){
            pthread_mutex_lock(&(info->pth_mtx));
            info->del_num = -num;
            pthread_mutex_unlock(&(info->pth_mtx));
            pthread_cond_broadcast(&(info->ready));
        }
#endif

    }
}

void add_task(void *(*fun)(void *), void *arg){
    do{
        if(info->shutdown){
            fprintf(stderr,"no pth pool exist\n");
        }else if(info->task_size > LIMITED_TASK_SIZE){
            fprintf(stderr,"too many task to do\n");
        }else{
            break;
        }
        return ;
    }while(0);
    
    struct task *tmp = (struct task *)malloc(sizeof(struct task));
    if(info == NULL){
        fprintf(stderr, "add task malloc failed\n");
        return;
    }
    tmp->fun = fun;
    tmp->arg = arg;
    tmp->next = NULL;


    pthread_mutex_lock(&(info->task_mtx));
    if(info->task_size == 0){
        info->task_front = tmp;
        info->task_back = tmp;
    }else{
        info->task_back->next = tmp;
        info->task_back = info->task_back->next;
    }
    info->task_size++;
    pthread_mutex_unlock(&(info->task_mtx));

    pthread_cond_signal(&(info->ready));
}
