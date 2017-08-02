#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "param_base.h"
#include "param.h"
#include "debug.h"
#include "task/task_web.h"
#include "web_main.h"
#include "task.h"
#include "cgi_server.h"

CTaskWeb::CTaskWeb(void)
{
	strcpy(mTaskName, "WEB");
}

CTaskWeb::~CTaskWeb(void)
{

}

int CTaskWeb::Init(void)
{
	MISC_PARAM *pMiscParam = get_misc_param();
	
	set_web_port(pMiscParam->http_port);
	web_set_interface(set_param_str, set_param_int, get_param, cmd_proc);

	return SDK_OK;
}

void CTaskWeb::Uninit(void)
{

}

void CTaskWeb::Process(void)
{
	start_web_server();		/* block func */	
}

int web_task_create(void)
{	
	return CTaskWeb::getInstance()->Create();
}

void web_task_destory(void)
{
	return CTaskWeb::getInstance()->Destory();
}

