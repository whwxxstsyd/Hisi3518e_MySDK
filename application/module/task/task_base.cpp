#include "debug.h"
#include "task/task_base.h"
#include "common.h"
#include "param.h"
#include "task.h"
#include "normal.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>

#undef		TARGET_LEVEL
#define		TARGET_LEVEL		LEVEL_INFO

CTaskBase::CTaskBase(void)
{
	mThreadId = -1;
	mRunning = 0;
	memset(mTaskName, 0, sizeof(mTaskName));
}

CTaskBase::~CTaskBase(void)
{

}

void CTaskBase::Process(void)
{
	LOG_DEBUG("in base process\n");
};

void *CTaskBase::ThreadEnter(void *arg)
{
	CTaskBase *pTaskBase = (CTaskBase *)arg;  

//	print_pid(pTaskBase->mTaskName);
	pTaskBase->Process();

	return NULL;
}


int CTaskBase::Create(void)
{	
	mRunning = 1;
	
	if (Init() != SDK_OK)
		return SDK_ERROR;

	if (!strcmp(mTaskName, "WEB"))		/* 将web线程设置高优先级 */
	{
		CreateDetachedTaskExt(&mThreadId, ThreadEnter, this, 1);		
		LOG_DEBUG("create realtime task[%s]\n", mTaskName);
	}	
	else	
		CreateDetachedTask(&mThreadId, ThreadEnter, this);		

	LOG_DEBUG("task %s create ok\n", mTaskName);
		
	return SDK_OK;
}

void CTaskBase::Destory(void)
{
	mRunning = 0;
	pthread_join(mThreadId, NULL);
	Uninit();
}

