#ifndef		__TASK_CGI_H__
#define		__TASK_CGI_H__

#include "task_base.h"
#include "common.h"

class CTaskCgi: public CTaskBase
{
    public:
		static CTaskCgi *getInstance()
		{
			static CTaskCgi *obj = NULL;
			if (NULL == obj)
			{
				obj = new CTaskCgi();		
			}
			return obj;
		}   
        
        CTaskCgi();
        ~CTaskCgi();
        int Init(void);      
        void Uninit(void);           
		void Process(void);	

	private:
};

int cgi_server_task_create(void);
void cgi_server_task_destory(void);


#endif
