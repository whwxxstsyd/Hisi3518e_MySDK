#ifndef     __TASK_TOPWAY_H__
#define     __TASK_TOPWAY_H__

#include "task_base.h"
#include "param.h"
#include <netinet/in.h>

#define 	TOPWAY_INTERFACE_DEF
#define 	TOPWAY_UDP_BOARDCAST_PORT	6785
#define 	TOPWAY_TCP_RESPOND_PORT	6786

typedef struct topway_udp_message
{
	char id[4];
	char cmd[4];
	char time[4];
	char reserved[32];
	char rand[32];		
	char tag[32];		
	
}topway_udp_msg_t;

typedef struct topway_tcp_message
{
	char id[4];
	char code[4];
	char rand[32];
	char tag[32];
	
}topway_tcp_msg_t;

typedef enum
{
	TOPWAY_IDLE_MODE,
	TOPWAY_RECORD_MODE,
	TOPWAY_SNAP_MODE,
	
}topway_mode;

typedef struct topway_var
{
	unsigned char rec_sta;
	unsigned char cap_sta;
	unsigned char new_sta;		//UDP广播新接收到的状态
	unsigned short cap_num;		//需要拍照的数量
	unsigned short cap_count;	//计数
	unsigned short cap_interval;
	unsigned int rec_duration;
	struct sockaddr_in c_addr;
	long cmd_dur;
	
}topway_var_t;

class CTaskTopway: public CTaskBase
{
	public:
		static CTaskTopway *getInstance()
		{
			static CTaskTopway *obj = NULL;
			if (NULL == obj)
			{
				obj = new CTaskTopway();		
			}
			return obj;
		}  

		CTaskTopway(void);
		~CTaskTopway(void);
		int Init(void);
		void Uninit(void);
		void Process(void);
		int TopwayInitSocket(int port);
		//void * topway_record_process(void *arg);
		//void * topway_capture_process(void *arg);
		
};

extern topway_var_t TwVar;
int topway_task_create(void);
void topway_task_destory(void);

#endif

