#ifndef		__TASK_DEFAULT_H__
#define		__TASK_DEFAULT_H__

#include "task_base.h"

class CTaskDefault: public CTaskBase
{
    public:
		static CTaskDefault *getInstance()
		{
			static CTaskDefault *obj = NULL;
			if (NULL == obj)
			{
				obj = new CTaskDefault();		
			}
			return obj;
		}	
		
        CTaskDefault();
        ~CTaskDefault();
		int Init(void);
		void Uninit(void);
        void Process(void);

	private:	
};

int ToPlatformInt(const char *key_word, int value);
int default_task_create(void);
void default_task_destory(void);

#endif
