#ifndef		__TASK_UPNP_H__
#define		__TASK_UPNP_H__

#include "task_base.h"
#include "common.h"
#include "param.h"
#include "my_timer.h"

class CTaskUpnp: public CTaskBase
{
    public:
		static CTaskUpnp *getInstance()
		{
			static CTaskUpnp *obj = NULL;
			if (NULL == obj)
			{
				obj = new CTaskUpnp();		
			}
			return obj;
		}	
		
        CTaskUpnp();
        ~CTaskUpnp();
        int Init(void);
        void Uninit(void);
        int restart(void);
        void Process(void); 

    public:
        int mOldHttpPort;
        int mOldRtspPort;        
        unsigned int mUpnpHandle;
};

int upnp_task_create(void);
void upnp_task_destory(void);  
int upnp_restart(int index, char *value);


#endif
