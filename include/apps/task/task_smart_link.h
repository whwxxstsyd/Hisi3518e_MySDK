#ifndef		__TASK_SMART_LINK_H__
#define		__TASK_SMART_LINK_H__

#include "task_base.h"
#include "common.h"
#include "param.h"

class CTasSmartLink: public CTaskBase
{
    public:
		static CTasSmartLink *getInstance()
		{
			static CTasSmartLink *obj = NULL;
			if (NULL == obj)
			{
				obj = new CTasSmartLink();		
			}
			return obj;
		}  
        
        CTasSmartLink();
        ~CTasSmartLink();
		int Init(void);
		void Uninit(void);        
        void Process(void);
};

int smart_link_task_create(void);
void smart_link_task_destory(void);

#endif

