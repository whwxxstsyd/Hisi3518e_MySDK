#include "msg.h"
#include "debug.h"

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

/* 超过三条消息没读就会发送覆盖 */
int CreateMsgHandle(int *pMsgHandle, int msg_key)
{
	int MsgId;
	
	MsgId = msgget(msg_key, IPC_CREAT | 0666);
	if (MsgId < 0)
	{
		LOG_ERROR("error at msgget\n");
		return SDK_ERROR;
	}
	*pMsgHandle = MsgId;
	
	return SDK_OK;
}

int SendMsg(int MsgHandle, MESSAGE msg)
{
	if (msgsnd(MsgHandle, &msg, sizeof(MESSAGE)-sizeof(long), 0) < 0)
	{
		LOG_ERROR("error at msgsnd msg: [%d_%d]\n", msg.mType, msg.mCommand);
		perror("msgsnd");
		return SDK_ERROR;
	}	
	return SDK_OK;
}
 
int RecvMsg(int MsgHandle, int type, MESSAGE *msg)
{
	if (msgrcv(MsgHandle, msg, sizeof(MESSAGE)-sizeof(long), type, 0) < 0)
	{
		LOG_ERROR("error at msgrcv msg: [%d]\n", type);
		perror("msgrcv");		
		return SDK_ERROR;
	}	
	return SDK_OK;
}
