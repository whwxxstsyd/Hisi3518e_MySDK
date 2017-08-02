#ifndef		__TASK_ONVIF_H__
#define		__TASK_ONVIF_H__

#include "task_base.h"

class CTaskOnvif: public CTaskBase
{
    public:
		static CTaskOnvif *getInstance()
		{
			static CTaskOnvif *obj = NULL;
			if (NULL == obj)
			{
				obj = new CTaskOnvif();		
			}
			return obj;
		}

        CTaskOnvif();
        ~CTaskOnvif();
		int Init(void);
		void Uninit(void);     
};

int onvif_task_create(void);
void onvif_task_destory(void);

#endif
