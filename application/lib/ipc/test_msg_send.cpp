#include "msg.h"
#include "debug.h"

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	int MsgHandle = -1;
	MESSAGE send_msg;

	CreateMsgHandle(&MsgHandle, MSG_EVENT_KEY);

	LOG_INFO("MsgHandle [%d]\n", MsgHandle);

	while (1)
	{
		memset(&send_msg, 0, sizeof(send_msg));
		send_msg.mCommand = 10;
		send_msg.mType = MSG_TYPE_EVENT_ALARM;
        if (SendMsg(MsgHandle, send_msg) < 0)
		{
			LOG_ERROR("error at SendMsg\n"); 
			return -1;
		}
        LOG_INFO("----------------------SEND MSG\n");
		sleep(1);
	}

	return 0;
}
