#ifndef		__TASK_PROXY_H__
#define		__TASK_PROXY_H__

#ifdef	__cplusplus
extern "C" { 
#endif

#include "task_base.h"
#include "common.h"
#include "param.h"

class CTaskProxy: public CTaskBase
{
    public:
		static CTaskProxy *getInstance()
		{
			static CTaskProxy *obj = NULL;
			if (NULL == obj)
			{
				obj = new CTaskProxy();	
			}
			return obj;
		}

        CTaskProxy();
        ~CTaskProxy();
		int Init(void);
		void Uninit(void);      
		void Process(void);	
	private:
          
};

int platform_proxy_task_create(void);
void platform_proxy_task_destory(void);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif
