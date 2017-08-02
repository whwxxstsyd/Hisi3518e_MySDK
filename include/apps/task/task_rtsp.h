#ifndef		__TASK_RTSP_H__
#define		__TASK_RTSP_H__

#include "task_base.h"

class CTaskRtsp: public CTaskBase
{
    public:
		static CTaskRtsp *getInstance()
		{
			static CTaskRtsp *obj = NULL;
			if (NULL == obj)
			{
				obj = new CTaskRtsp();		
			}
			return obj;
		}
		
        CTaskRtsp();
        ~CTaskRtsp();
		int Init(void);
		void Uninit(void);	  
};

int rtsp_task_create(void);
void rtsp_task_destory(void);

#endif
