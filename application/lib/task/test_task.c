#include "task.h"
#include "common.h"
#include "debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

void *Proc1(void *pArg)
{
    while(1)
    {
        printf("int %s..\n", __func__);
        sleep(1);
    }

    return NULL;    
}

void *Proc2(void *pArg)
{
    while(1)
    {
        printf("int %s..\n", __func__);
        sleep(1);
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t thread_id1, thread_id2;

    CreateTask(&thread_id1, Proc1, NULL);
    CreateDetachedTask(&thread_id2, Proc2, NULL);
    
    sleep(3);
   
    pthread_join(thread_id1, NULL);

    return 0;
}

