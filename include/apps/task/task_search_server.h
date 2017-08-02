#ifndef		__TASK_SEARCH_SERVER_H__
#define		__TASK_SEARCH_SERVER_H__

#include "task_base.h"
#include "common.h"
#include "search.h"

class CTaskSearchServer: public CTaskBase
{
    public:
		static CTaskSearchServer *getInstance()
		{
			static CTaskSearchServer *obj = NULL;
			if (NULL == obj)
			{
				obj = new CTaskSearchServer();		
			}
			return obj;
		}
		
        CTaskSearchServer();
        ~CTaskSearchServer();
		int Init(void);
		void Uninit(void);
        int InitSocket(int port);
        char *SearchProc(char *RespStr);
        int SetParamProc(char *ReqStr, char *RespStr);
        void Process(void);

    private:        
        SYSTEM_PARAM *mSystemParam;
};

int search_server_task_create(void);
void search_server_task_destory(void);


#endif

