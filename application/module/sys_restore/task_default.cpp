#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/reboot.h>

#include "param.h"
#include "debug.h"
#include "msg.h"
#include "param_base.h"
#include "param.h"
#include "boot_save_default.h"
#include "hi_gpio.h"
#include "av_buffer.h"
#include "task/task_default.h"
#include "cgi_server.h"
#include "param_str.h"

#define		GPIO_DEBUG		0	/* 检查gpio是否正常 */

CTaskDefault::CTaskDefault(void)
{
	strcpy(mTaskName, "DEFAULT");
}

CTaskDefault::~CTaskDefault(void)
{

}

void CTaskDefault::Process(void)
{
	RAM_PARAM *pRamParam = get_ram_param();
	FACTORY_PARAM *pFactoryParam = get_factory_param();
#if !GPIO_DEBUG
	while (mRunning)
	{
		if (get_gpio_value(DEFAULT_GPIO))
		{
			usleep(100 * 1000);
			continue;
		}
		/* 防止抖动误操作 */
		usleep(10 * 1000);
		if (get_gpio_value(DEFAULT_GPIO))
		{
			usleep(100 * 1000);
			continue;
		}		
		sleep(1);
		if (get_gpio_value(DEFAULT_GPIO))
			goto ENTER_AP_MODE;
		sleep(1);
		if (get_gpio_value(DEFAULT_GPIO))
			goto ENTER_AP_MODE;
		sleep(1);	
		if (get_gpio_value(DEFAULT_GPIO))
		{
			continue;
		}	
		LOG_INFO("========  press default key to default camera  ========\n");		
		
		if(0 == strncmp(pFactoryParam->region, "china", sizeof(pFactoryParam->region)))
		play_audio_file(AUDIO_DEFAULT_CH);
		else
		play_audio_file(AUDIO_DEFAULT_EN);
		sleep(4);			/* wait for play audio */
		sys_default_config();	
		reboot(RB_AUTOBOOT);

ENTER_AP_MODE:
		LOG_INFO("PLAY AUDIO_ENTER_SMARK_LINK\n");
		pRamParam->net_status = NET_AP;	
		if(0 == strncmp(pFactoryParam->region, "china", sizeof(pFactoryParam->region)))
			play_audio_file(AUDIO_OUT_SMARK_LINK_CH);		
		else			
			play_audio_file(AUDIO_OUT_SMARK_LINK_EN);	
		//ToPlatformInt(RED_LED_STR, LED_FLASH_SLOW);
		system("echo \"stop\" > /proc/elian");
		sleep(1);
		if (!pRamParam->wifi_mode)
		{
			system("/etc/sta_to_ap.sh");
			pRamParam->wifi_mode = 1;
			pRamParam->smart_link_flag = 0;
		}
	}
#else	
	while (1)
	{
		int ret;

		ret = get_gpio_value(DEFAULT_GPIO);
		LOG_INFO("gpio value: %d\n", ret);
		usleep(200 * 1000);
	}
#endif
}

int CTaskDefault::Init(void)
{

	return SDK_OK;
}

void CTaskDefault::Uninit(void)
{

}

int default_task_create(void)
{	
	return CTaskDefault::getInstance()->Create();
}

void default_task_destory(void)
{
	return CTaskDefault::getInstance()->Destory();
}

