#ifndef		__TASK_RECORD_H__
#define		__TASK_RECORD_H__

#include "task_base.h"
#include "common.h"
#include "avilib.h"

class CTaskRecord: public CTaskBase
{
    public:
		static CTaskRecord *getInstance()
		{
			static CTaskRecord *obj = NULL;
			if (NULL == obj)
			{
				obj = new CTaskRecord();		
			}
			return obj;
		}	
		
        CTaskRecord();
        ~CTaskRecord();	
		int Init(void);	
		void Uninit(void);	        
		void Process(void);

	private:

};


int record_task_create(void);
void record_task_destory(void);

#endif
