#ifndef     __TASK_PTZ_H__
#define     __TASK_PTZ_H__

#include "task_base.h"
#include "param.h"

class CTaskPtz: public CTaskBase
{
    public:
		static CTaskPtz *getInstance()
		{
			static CTaskPtz *obj = NULL;
			if (NULL == obj)
			{
				obj = new CTaskPtz();		
			}
			return obj;
		}
        CTaskPtz();
        ~CTaskPtz();		
		int Init(void);
		void Uninit(void);	
		void Process(void);

    private:
        PTZ_PARAM *mpPtzParam;
};

int ptz_task_create(void);
void ptz_task_destory(void);

#endif
