#ifndef		__TASK_WEB_H__
#define		__TASK_WEB_H__

#include "task_base.h"
#include "common.h"
#include "param.h"

class CTaskWeb: public CTaskBase
{
    public:
		static CTaskWeb *getInstance()
		{
			static CTaskWeb *obj = NULL;
			if (NULL == obj)
			{
				obj = new CTaskWeb();		
			}
			return obj;
		}  
        
        CTaskWeb();
        ~CTaskWeb();
		int Init(void);
		void Uninit(void);        
        void Process(void);
        
	private:
        int mPort;
};

int web_task_create(void);
void web_task_destory(void);
int task_http_port(int index, char *value);

#endif

