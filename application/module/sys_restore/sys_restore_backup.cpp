#include "task/task_upgrade_backup.h"
#include "param.h"
#include "normal.h"
#include "common.h"
#include "debug.h"
#include "param_ext.h"
#include "my_timer.h"
#include "string_parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>

const char *short_options = "D";		/* D: deamon mode, N: with Network task */
struct option long_options[] = 
{     
	{ "daemon",   	0,   	NULL,	'D' },   
	{  0,     		0,     	0,     	0 },
};

int main(int argc, char *argv[])
{
	int ch;
	int isDaemon = 1, WithCheck = 1;	

	while ((ch = getopt_long(argc, argv, short_options, long_options, NULL)) != -1)
	{
		switch (ch)
		{
			case 'D':		/* 在前台运行 */
				isDaemon = 0;
				break;
			default:
				break;
		}
	}

	if (isDaemon)
        daemon(0, 0);	

#ifdef MEMORY_WATCH
	mem_check_init();
	LOG_INFO("init memory watch\n");
#endif	

	if (string_parser_init() < 0)
		LOG_WARN("string_parser_init fail\n");	
	if (init_timer() < 0)
		LOG_WARN("init_timer fail\n");

	while (param_is_init() == 0)	/* 等待参数加载完成 */
		usleep(100 * 1000);
		
	upgrade_backup_task_create();	

	while (1) sleep(1);

	upgrade_backup_task_destory();

#ifdef MEMORY_WATCH
	mem_check_uninit();
#endif		
 
	return SDK_OK;
}
