#ifndef     __TASK_BASE_H__
#define     __TASK_BASE_H__

#include "normal.h"
#include "debug.h"

#include <pthread.h>

class CTaskBase
{
    public:
        CTaskBase(void);
        ~CTaskBase(void);
        int Create(void);
        void Destory(void);
		virtual void Process(void);
		virtual int Init(void) = 0;
		virtual void Uninit(void) = 0;		
		
	private:		
		static void *ThreadEnter(void *arg);

    private:
        pthread_t mThreadId;

    protected:      /* 本类和子类成员函数访问 */   
        char mTaskName[STR_32_LEN];

    public:
        int mRunning;
};

#endif
