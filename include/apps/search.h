#ifndef		__SEARCH_H__
#define		__SEARCH_H__

#define		BROADCAST_PORT		(10020) 

typedef enum SEARCH_CMD_T
{
	SEARCH_REQ_CMD = 0,
	SEARCH_RESP_CMD,
	SET_PARAM_REQ_CMD,
	SET_PARAM_RESP_CMD,
}SEARCH_CMD;

typedef struct CMD_HEAD_T
{
	char mark[4];
	int type;
	int context_len;
}CMD_HEAD;

typedef struct RESP_MSG_T
{
    CMD_HEAD *pCmdHead;
    char *pContext;
}RESP_MSG;

#endif
