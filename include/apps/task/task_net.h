#ifndef		__TASK_NET_H__
#define		__TASK_NET_H__

#include "task_base.h"
#include "common.h"
#include "my_timer.h"
#include "param.h"


class CTaskNet: public CTaskBase
{
    public:
		static CTaskNet *getInstance()
		{
			static CTaskNet *obj = NULL;
			if (NULL == obj)
			{
				obj = new CTaskNet();		
			}
			return obj;
		}	
		
        CTaskNet(void);
        ~CTaskNet(void);	
        int Init(void);	
        void Uninit(void);	        
        void Process(void);
};

int net_task_create(void);
void net_task_destory(void);  
int net_set_ip(int index, char *value);
int net_set_netmask(int index, char *value);
int net_set_gateway(int index, char *value);
int net_set_dns(int index, char *value);
int net_set_mac(int index, char *value);        
int net_dhcp_ip(int index, char *value);
int net_dhcp_all(int index, char *value); 
//int config_net(const char *inf, NET_PARAM *pNetParam);

#endif
