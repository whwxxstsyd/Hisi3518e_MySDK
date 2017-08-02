#ifndef     __TASK_DANALE_H__
#define     __TASK_DANALE_H__

#include "task_base.h"
#include "danale.h"
#include "param.h"

class CTaskDanale: public CTaskBase
{
    public:
		static CTaskDanale *getInstance()
		{
			static CTaskDanale *obj = NULL;
			if (NULL == obj)
			{
				obj = new CTaskDanale();		
			}
			return obj;
		}  
        
        CTaskDanale(void);
        ~CTaskDanale(void);
        int Init(void);
        void Uninit(void);
};

int danale_task_create(void);
void danale_task_destory(void);

#endif
