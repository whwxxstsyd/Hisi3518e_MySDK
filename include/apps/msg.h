#ifndef     __MSG_H__
#define     __MSG_H__

#include "common.h"

#ifdef	__cplusplus
extern "C" { 
#endif

int CreateMsgHandle(int *pMsgHandle, int msg_key);
int SendMsg(int MsgHandle, MESSAGE msg);
int RecvMsg(int MsgHandle, int type, MESSAGE *msg);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif

