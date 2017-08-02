#ifndef     __TASK_H__
#define     __TASK_H__

#ifdef	__cplusplus
extern "C" { 
#endif

#include "common.h"

#include <pthread.h>

int CreateTask(pthread_t *thread_id, THREAD_ENTER routine, void *arg);
int CreateDetachedTask(pthread_t *thread_id, THREAD_ENTER routine, void *arg); 
int CreateDetachedTaskExt(pthread_t *thread_id, THREAD_ENTER routine, void *arg, int high_priority);
void DestroyTask(pthread_t thread_id);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif
