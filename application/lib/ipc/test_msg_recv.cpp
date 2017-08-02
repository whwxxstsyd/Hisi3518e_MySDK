#include "msg.h"
#include "debug.h"

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

int main(int argc, char *argv[])
{
	int MsgHandle = -1;
	MESSAGE recv_msg;

	CreateMsgHandle(&MsgHandle, MSG_EVENT_KEY);

	LOG_INFO("MsgHandle [%d]\n", MsgHandle);

	while (1)
	{
		memset(&recv_msg, 0, sizeof(MESSAGE));	
        if (RecvMsg(MsgHandle, MSG_TYPE_EVENT_ALARM, &recv_msg) < 0)
		{
			LOG_ERROR("error at RecvMsg\n"); 
			return -1;
		}
        LOG_INFO("----------------------GET MSG\n");
	}

	return 0;
}
