#include "pth_pool.h"
#include "myhttp.h"

static struct pool_info *info;

int pth_pool_init(int num){
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

        for(int i=0; i<DEFULT_SIZE; i++){
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
    for(;;){
        struct task *tmp;

        pthread_mutex_lock(&(info->task_mtx));
        while(info->task_size == 0){
            pthread_cond_wait(&(info->ready), &(info->task_mtx));

            if(info->del_num == 0 && !info->shutdown){
                continue;
            }
            if(info->del_num != 0){
                pthread_mutex_lock(&(info->pth_mtx));
                info->del_num--;
                pthread_mutex_unlock(&(info->pth_mtx));
            }
            if(info->pth_cur_size >= DEFULT_SIZE || info->shutdown){
                pthread_mutex_lock(&(info->pth_mtx));
                info->pth_cur_size--;
                info->pth_no[(long)arg] = 0;
                pthread_mutex_unlock(&(info->pth_mtx));
                pthread_mutex_unlock(&(info->task_mtx));
                pthread_exit(NULL);
            }
        }

        tmp = info->task_front;
        info->task_front = info->task_front->next;
        info->task_size--;

        pthread_mutex_unlock(&(info->task_mtx));

        (*(tmp->fun))(tmp->arg);

        free(tmp->arg);
        tmp->arg = NULL;
        free(tmp);
        tmp = NULL;

    }

    pthread_exit(NULL);
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
        // pthread_mutex_lock(&(info->pth_mtx));
        // pthread_mutex_lock(&(info->task_mtx));
        fprintf(stderr,"cur pth:%d,cur task:%d\n",info->pth_cur_size,info->task_size);

        if(2 * info->pth_cur_size < info->task_size || info->pth_cur_size < DEFULT_SIZE){
            int top = (info->task_size/2 > info->pth_max_size)?(info->pth_max_size):(info->task_size/2);
            num = ((top > DEFULT_SIZE)?top:DEFULT_SIZE) - info->pth_cur_size;
        }else if(info->pth_cur_size > info->task_size || info->pth_cur_size > info->pth_max_size){
            int bottom = (info->task_size > DEFULT_SIZE)?(info->task_size):(DEFULT_SIZE);
            num = ((bottom > info->pth_max_size)?info->pth_max_size:bottom) - info->pth_cur_size;
        }
        // pthread_mutex_unlock(&(info->task_mtx));
        // pthread_mutex_unlock(&(info->pth_mtx));


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

    }
    pthread_exit(NULL);
}

void add_task(void *(*fun)(void *), void *arg){
    if(info->shutdown){
        return;
    }
    struct task *tmp = (struct task *)malloc(sizeof(struct task));
    tmp->fun = fun;
    tmp->arg = NULL;
    add_task_arg(&tmp->arg, arg);
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

    pthread_cond_broadcast(&(info->ready));
}
  
static void add_task_arg(void **taskarg,void *arg){
    *taskarg = (struct client_info *)malloc(sizeof(struct client_info));
    memcpy(*taskarg, arg, sizeof(struct client_info));
}
