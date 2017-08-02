#ifndef		__TASK_SNAP_H__
#define		__TASK_SNAP_H__

#include "task_base.h"
#include "common.h"
#include "my_timer.h"

class CTaskSnap: public CTaskBase
{
    public:
		static CTaskSnap *getInstance()
		{
			static CTaskSnap *obj = NULL;
			if (NULL == obj)
			{
				obj = new CTaskSnap();		
			}
			return obj;
		}	
		
        CTaskSnap();
        ~CTaskSnap();
		int Init(void);
		void Uninit(void);        
        void Process(void);    
};

int snap_task_create(void);
void snap_task_destory(void);

#endif
