#ifndef     __TASK_NTP_H__
#define     __TASK_NTP_H__

#include "task_base.h"
#include "param.h"

class CTaskNtp: public CTaskBase
{
    public:
		static CTaskNtp *getInstance()
		{
			static CTaskNtp *obj = NULL;
			if (NULL == obj)
			{
				obj = new CTaskNtp();		
			}
			return obj;
		}
        CTaskNtp();
        ~CTaskNtp();		
		int Init(void);
		void Uninit(void);	
		void Process(void);
};

int ntp_task_create(void);
void ntp_task_destory(void);

#endif
