#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "my_timer.h"
#include "debug.h"
#include "common.h"
#include "normal.h"

void time_repeat_func(void)
{
    printf("%s\n", __func__);
}

void time_once_func(void)
{
    printf("%s\n", __func__);
}

int main(int argc, char *argv[])
{
    int ret;
    pthread_t thread_id;
    
    if (init_timer() != SDK_OK)
    {
        LOG_ERROR("init_timer fail\n");
        return -1;
    }
    
    alloc_timer(2000, TYPE_REPEAT, (TIMER_CALLBACK)time_repeat_func, 0, NULL);
    alloc_timer(1000, TYPE_ONCE, (TIMER_CALLBACK)time_once_func, 0, NULL);    
    while(1)
    {

        sleep(5);
        LOG_INFO("at main\n");
    }

    uninit_timer();
    return 0;
}
