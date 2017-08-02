#include "debug.h"
#include "task/task_check.h"
#include "param_base.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/reboot.h>
#include <fcntl.h>
#include <dirent.h>

int find_pid_by_name(const char *ProcName)
{
    DIR *dir;
	int pid = -1, get_pid = 0;
    struct dirent *d;

    /* Open the /proc directory. */
    dir = opendir("/proc");
    if (!dir)
    {
        LOG_INFO("cannot open /proc");
        return -1;
    }

    /* Walk through the directory. */
    while ((d = readdir(dir)) != NULL) 
	{
		int fd;
        char path[64] = {0};
		char data[1024] = {0};

        /* See if this is a process */
        if ((pid = atoi(d->d_name)) == 0)       
			continue;

        snprintf(path, sizeof(path), "/proc/%s/cmdline", d->d_name);		
		fd = open(path, O_RDONLY);
		read(fd, data, sizeof(data));
		if (strstr(data, ProcName))
		{
			close(fd);
			closedir(dir);
			return pid;
		}
		close(fd);
    }
    closedir(dir);

    return 0;
}


CTaskCheck::CTaskCheck(void)
{
	strcpy(mTaskName, "CHECK");
	memset(mMainPath, 0, sizeof(mMainPath));
	memset(mPlatformPath, 0, sizeof(mPlatformPath));	
}

CTaskCheck::~CTaskCheck(void)
{

}


void CTaskCheck::Process(void)
{	
	int MainEnterExist = 0, PlatformHisiExist = 0;
	RAM_PARAM *pRamParam = get_ram_param();
	
	while (mRunning)
	{
		MainEnterExist = access(mMainPath, F_OK);
		PlatformHisiExist = access(mPlatformPath, F_OK);		
		if (((MainEnterExist != 0) || (PlatformHisiExist != 0)) && 
			(pRamParam->system_status.upgrade == 0))
		{
			LOG_ERROR("[%s] is killed, restart system\n", MainEnterExist != 0 ? "main_enter" : "platform_hisi");
			sleep(5);
			reboot(RB_AUTOBOOT);	
		}	
		sleep(5);
	}
}

int CTaskCheck::Init(void)
{
	int MainPid = -1, PlatformPid = -1;

	MainPid = find_pid_by_name("main_enter");
	PlatformPid = find_pid_by_name("platform_hisi");

	if (!MainPid || !PlatformPid)
	{
		LOG_ERROR("[main_enter/platform_hisi] not start\n");
		return -1;
	}
	sprintf(mMainPath, "/proc/%d", MainPid);
	sprintf(mPlatformPath, "/proc/%d", PlatformPid);	

	return SDK_OK;
}

void CTaskCheck::Uninit(void)
{

}

int check_task_create(void)
{	
	return CTaskCheck::getInstance()->Create();
}

void check_task_destory(void)
{
	return CTaskCheck::getInstance()->Destory();
}

