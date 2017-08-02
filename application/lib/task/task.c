#include "common.h"
#include "debug.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#undef		TARGET_LEVEL
#define		TARGET_LEVEL		LEVEL_INFO

int CreateTask(pthread_t *thread_id, THREAD_ENTER routine, void *arg) /* 必需调用pthread_join销毁,否则线程资源无法释放,造成内存泄露 */
{
    pthread_create(thread_id, NULL, routine, arg);

    return SDK_OK;
}

int CreateDetachedTask(pthread_t *thread_id, THREAD_ENTER routine, void *arg)    
{
    pthread_attr_t attr;
	
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(thread_id, &attr, routine, arg);
    pthread_attr_destroy(&attr);

    return SDK_OK;
}

int CreateDetachedTaskExt(pthread_t *thread_id, THREAD_ENTER routine, void *arg, int high)    
{
    pthread_attr_t attr;
    
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (high)
    {
        struct sched_param param;
        int max_priority, min_priority;

        max_priority = sched_get_priority_max(SCHED_RR);
        min_priority = sched_get_priority_min(SCHED_RR);        
        pthread_attr_setschedpolicy(&attr, SCHED_RR);
        LOG_DEBUG("priority [%d_%d]\n", min_priority, max_priority);
        param.sched_priority = min_priority;
        pthread_attr_setschedparam(&attr, &param);        
    }
    pthread_create(thread_id, &attr, routine, arg);
    pthread_attr_destroy(&attr);

    return SDK_OK;
}

/* 注意线程中锁释放 */
void DestroyTask(pthread_t thread_id)
{
    pthread_cancel(thread_id);
    pthread_join(thread_id, NULL);      /* detached线程可以不用调用jion */
}

