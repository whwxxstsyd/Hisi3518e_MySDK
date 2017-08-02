#ifndef		__MY_TIMER_H__
#define		__MY_TIMER_H__

#ifdef		__cplusplus
extern "C"	{
#endif

#include <sys/time.h>
#include "linux_list.h"
#include "common.h"

typedef     void (*TIMER_CALLBACK)(void *param, void *pTimer);

typedef struct TIMER_T
{
    struct list_head list;
	int type; 
    int timeout;        /* ms */
    long start_time;
    void *arg;
	TIMER_CALLBACK timer_callback;     
}TIMER;

#ifdef	__cplusplus
class CTimer
{
    public:
		static CTimer& getInstance()
		{
            static CTimer mInstance;

            return mInstance;
        }        
        CTimer(void);
        ~CTimer(void);        
        int Init(void);
        void Uninit(void);
        TIMER *AllocTimer(int timeout, int type, TIMER_CALLBACK callback, int run_once, void *arg);
        void FreeTimer(TIMER *timer);

        static void *TimerProc(void *pArg);
        static void Handler(CTimer *pTimer);

    private:
        struct list_head mTimerList;
        pthread_t mTimerProcId;
        int mRunning;
        int mInit;
};
#endif

int init_timer(void);
void uninit_timer(void);
TIMER *alloc_timer(int timeout, int type, TIMER_CALLBACK callback, int run_once, void *arg);  /* unit: s */
void free_timer(TIMER *timer);

#ifdef		__cplusplus
}
#endif

#endif
