#ifndef     __TASK_CHECK__
#define     __TASK_CHECK__

#include "task_base.h"

class CTaskCheck: public CTaskBase
{
    public:
		static CTaskCheck *getInstance()
		{
			static CTaskCheck *obj = NULL;
			if (NULL == obj)
			{
				obj = new CTaskCheck();		
			}
			return obj;
		}
        
        CTaskCheck();
        ~CTaskCheck();        
        int Init(void);
        void Uninit(void); 
        void Process(void);        

    private:
        char mMainPath[64];
	    char mPlatformPath[64];
};

int check_task_create(void);
void check_task_destory(void);

#endif
