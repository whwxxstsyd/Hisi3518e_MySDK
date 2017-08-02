#ifndef		__TASK_OTA_H__
#define		__TASK_OTA_H__

#include "task_base.h"
#include "common.h"

#define     USE_NAND_FLASH          0

class CTaskOta public CTaskBase
{
    public:
		static CTaskOta *getInstance()
		{
			static CTaskOta *obj = NULL;
			if (NULL == obj)
			{
				obj = new CTaskOta();		
			}
			return obj;
		}	
		
        CTaskOta();
        ~CTaskOta();
		int Init(void);
		void Uninit(void);	
        int LocalUpgrade(int type, const char *path); 
        int GetFw(const char *local_path);
        int OnlineUpgrade(int type);
        void Process(void);	

	private:	
        int mMsgHandle;
};

int upgrade_task_create(void);
void upgrade_task_destory(void);


#endif
