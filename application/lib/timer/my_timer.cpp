#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include "my_timer.h"
#include "linux_list.h"
#include "debug.h"
#include "common.h"
#include "task.h"
#include "normal.h"

CTimer::CTimer(void)
{
    mRunning = -1;
	mInit = 0;
}

CTimer::~CTimer(void)
{

}

int check_timeout(TIMER *timer)
{
	long now;

    now = get_time_ms();
    if (now >= (timer->start_time + timer->timeout))
		return TIME_OUT;
	else
		return NO_TIME_OUT;
}

void CTimer::Handler(CTimer *pCTimer)  
{
	TIMER *timer, *n_timer;

    list_for_each_entry_safe(timer, n_timer, &pCTimer->mTimerList, list)
    {
        if (check_timeout(timer) == TIME_OUT)
        {
            timer->timer_callback(timer->arg, timer);        /* 暂未定义传入参数 */
            if (timer->type == TYPE_REPEAT)
                timer->start_time = get_time_ms();
            else
            {
			    list_del(&timer->list);
			    free(timer);
				timer = NULL;            
//                free_timer(timer);
            }
        }
    }
}

void *CTimer::TimerProc(void *pArg)
{  
    CTimer *pCTimer = (CTimer *)pArg;
    
    pCTimer->mRunning = 1;
    while (pCTimer->mRunning)
    {
        Handler(pCTimer);
        usleep(100 * 1000);
    }

    return NULL;
}

int CTimer::Init(void)
{
	if (mInit)
		return 0;
	
    INIT_LIST_HEAD(&mTimerList);
    CreateTask(&mTimerProcId, TimerProc, this);

	mInit = 1;

    return SDK_OK;
}

void CTimer::Uninit(void)
{
    TIMER *timer, *n_timer;

	if (!mInit)
		return;

    mRunning = 0;
    pthread_join(mTimerProcId, NULL);
    list_for_each_entry_safe(timer, n_timer, &mTimerList, list)
    {
        list_del(&timer->list);
        free(timer);
    }
	mInit = 0;
}

/* run_once: 立即执行一次 */
TIMER *CTimer::AllocTimer(int timeout, int type, TIMER_CALLBACK callback, int run_once, void *arg)  /* run_once:[1]立刻运行callback  unit: ms */
{
	int i;
	TIMER *timer;

	if ((NULL == callback) || (timeout == 0))
	{
		LOG_ERROR("param error\n");
		return NULL;
	}
	
	timer = (TIMER *)malloc(sizeof(TIMER));
    timer->timeout = timeout;
	timer->type = type;
    timer->arg = arg;
	timer->timer_callback = callback;
    timer->start_time = get_time_ms();
    if (run_once == 1)
        callback(arg, timer);
    list_add_tail(&timer->list, &mTimerList);

	return timer;
}

void CTimer::FreeTimer(TIMER *timer)
{
	timer->type = TYPE_ONCE;
	timer->start_time = get_time_ms() - 1000;
	usleep(150 * 1000);			/* 等待callback执行完 */
}

int init_timer(void)
{
    return CTimer::getInstance().Init();
}
void uninit_timer(void)
{
    return CTimer::getInstance().Uninit();
}
TIMER *alloc_timer(int timeout, int type, TIMER_CALLBACK callback, int run_once, void *arg)  /* unit: s */
{
    return CTimer::getInstance().AllocTimer(timeout, type, callback, run_once, arg);
}
void free_timer(TIMER *timer)
{
    return CTimer::getInstance().FreeTimer(timer);
}

