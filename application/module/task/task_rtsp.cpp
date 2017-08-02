#include <stdio.h>
#include <stdlib.h>

#include "param_base.h"
#include "param.h"
#include "debug.h"
#include "rtsp_global.h"
#include "task/task_rtsp.h"

CTaskRtsp::CTaskRtsp(void)
{
	strcpy(mTaskName, "RTSP");
}

CTaskRtsp::~CTaskRtsp(void)
{

}
/* stream index define:
10 main 
11 sub1 
12 sub2 
13 main + 711
14 sub1 + 711
15 sub2 + 711
*/
int CTaskRtsp::Init(void)
{
//	MISC_PARAM *pMiscParam = get_misc_param();
	
	rtsp_set_port(554);
	rtsp_start();

	return SDK_OK;
}

void CTaskRtsp::Uninit(void)
{
	rtsp_stop();
}

int rtsp_task_create(void)
{
	return CTaskRtsp::getInstance()->Create();
}

void rtsp_task_destory(void)
{
	return CTaskRtsp::getInstance()->Destory();
}

