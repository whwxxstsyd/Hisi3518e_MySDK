#ifndef		__TASK_DDNS_H__
#define		__TASK_DDNS_H__

#include "task_base.h"
#include "common.h"

class CTaskDdns: public CTaskBase
{
    public:
		static CTaskDdns *getInstance()
		{
			static CTaskDdns *obj = NULL;
			if (NULL == obj)
			{
				obj = new CTaskDdns();		
			}
			return obj;
		}
		
        CTaskDdns();
        ~CTaskDdns();
		int Init(void);
		void Uninit(void);		
        int StartThrDdnsServer(void);
        int StopThrDdnsServer(void);
        int ThrDdnsRestart(void);  
        int StartInnerDdnsServer(void);
        int StopInnerDdnsServer(void);
        int InnerDdnsRestart(void);          

		static void *GetExternProc(void *arg);
		static void *InnerDdnsProc(void *arg);
		static void *ThrDdnsProc(void *arg);        

	public:
		pthread_t mInnerDdnsId;
		pthread_t mThrDdnsId;		
		pthread_t mRealIpId;	
		MISC_PARAM *mMiscParam;
		INNER_DDNS_PARAM *mInnerDdnsParam;
		DDNS_PARAM *mThrDdnsParam;
		RAM_PARAM *mRamParam;
		int get_wlan_ip;
};

int ddns_task_create(void);
void ddns_task_destory(void);
int inner_ddns_start(void);
int inner_ddns_stop(void);
int thr_ddns_start(void);
int thr_ddns_stop(void);


#endif
