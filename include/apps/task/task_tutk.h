#ifndef     __TASK_TUTK_H__
#define     __TASK_TUTK_H__

#include "task_base.h"
#include "tutk.h"
#include "param.h"

class CTaskTutk: public CTaskBase
{
    public:
		static CTaskTutk *getInstance()
		{
			static CTaskTutk *obj = NULL;
			if (NULL == obj)
			{
				obj = new CTaskTutk();		
			}
			return obj;
		}  
        
        CTaskTutk(void);
        ~CTaskTutk(void);
        int Init(void);
        void Uninit(void);
};

int tutk_task_create(void);
void tutk_task_destory(void);

#endif
