#include "param_base.h"
#include "param.h"
#include "normal.h"
#include "common.h"

#include <string.h>

 FTP_PARAM *get_ftp_param(void)
{
	int offset;
	
	offset = offsetof(SYSTEM_PARAM, flash_param) +  offsetof(FLASH_PARAM, ftp_param);

	return (FTP_PARAM *)system_param_map(offset);		
}

 ALARM_PARAM *get_alarm_param(void)
{
	int offset;
	
	offset = offsetof(SYSTEM_PARAM, flash_param) +  offsetof(FLASH_PARAM, alarm_param);

	return (ALARM_PARAM *)system_param_map(offset);		
}

 EMAIL_PARAM *get_email_param(void)
{
	int offset;
	
	offset = offsetof(SYSTEM_PARAM, flash_param) +  offsetof(FLASH_PARAM, email_param);

	return (EMAIL_PARAM *)system_param_map(offset);		
}

 VIDEO_PARAM *get_video_param(void)
{
	int offset;
	
	offset = offsetof(SYSTEM_PARAM, flash_param) +  offsetof(FLASH_PARAM, video_param);

	return (VIDEO_PARAM *)system_param_map(offset);		
}

 AUDIO_PARAM *get_audio_param(void)
{
	int offset;
	
	offset = offsetof(SYSTEM_PARAM, flash_param) +  offsetof(FLASH_PARAM, audio_param);

	return (AUDIO_PARAM *)system_param_map(offset);		
}

 PTZ_PARAM *get_ptz_param(void)
{
	int offset;
	
	offset = offsetof(SYSTEM_PARAM, flash_param) +  offsetof(FLASH_PARAM, ptz_param);

	return (PTZ_PARAM *)system_param_map(offset);		
}

 RECORD_PARAM *get_record_param(void)
{
	int offset;
	
	offset = offsetof(SYSTEM_PARAM, flash_param) +  offsetof(FLASH_PARAM, record_param);

	return (RECORD_PARAM *)system_param_map(offset);		
}

 SNAP_PARAM *get_snap_param(void)
{
	int offset;
	
	offset = offsetof(SYSTEM_PARAM, flash_param) +  offsetof(FLASH_PARAM, snap_param);

	return (SNAP_PARAM *)system_param_map(offset);		
}

 WIFI_PARAM *get_wifi_param(void)
{
	int offset;
	
	offset = offsetof(SYSTEM_PARAM, flash_param) +  offsetof(FLASH_PARAM, wifi_param);

	return (WIFI_PARAM *)system_param_map(offset);		
}

 NET_PARAM *get_net_param(void)
{
	int offset;
	
	offset = offsetof(SYSTEM_PARAM, flash_param) +  offsetof(FLASH_PARAM, net_param);

	return (NET_PARAM *)system_param_map(offset);		
}

 NET_PARAM *get_real_net_param(void)
{
	int offset;
	
	offset = offsetof(SYSTEM_PARAM, ram_param) +  offsetof(RAM_PARAM, net_param);

	return (NET_PARAM *)system_param_map(offset);		
}

 INNER_DDNS_PARAM *get_ddns_param(void)
{
	int offset;
	
	offset = offsetof(SYSTEM_PARAM, factory_param) +  offsetof(FACTORY_PARAM, ddns_param);

	return (INNER_DDNS_PARAM *)system_param_map(offset);		
}

DDNS_PARAM *get_thr_ddns_param(void)
{
	int offset;
	
	offset = offsetof(SYSTEM_PARAM, flash_param) +  offsetof(FLASH_PARAM, ddns_param);

	return (DDNS_PARAM *)system_param_map(offset);		
}

 TIME_PARAM *get_time_param(void)
{
	int offset;
	
	offset = offsetof(SYSTEM_PARAM, flash_param) +  offsetof(FLASH_PARAM, time_param);

	return (TIME_PARAM *)system_param_map(offset);		
}

 USER_PARAM *get_user_param(void)
{
	int offset;
	
	offset = offsetof(SYSTEM_PARAM, flash_param) +  offsetof(FLASH_PARAM, user_param);

	return (USER_PARAM *)system_param_map(offset);		
}

 MULTI_PARAM *get_multi_param(void)
{
	int offset;
	
	offset = offsetof(SYSTEM_PARAM, flash_param) +  offsetof(FLASH_PARAM, multi_param);

	return (MULTI_PARAM *)system_param_map(offset);		
}

 MISC_PARAM *get_misc_param(void)
{
	int offset;
	
	offset = offsetof(SYSTEM_PARAM, flash_param) +  offsetof(FLASH_PARAM, misc_param);

	return (MISC_PARAM *)system_param_map(offset);		
}

  RTMP_PARAM *get_rtmp_param(void)
{
	int offset;
	
	offset = offsetof(SYSTEM_PARAM, flash_param) +  offsetof(FLASH_PARAM, rtmp_param);

	return (RTMP_PARAM *)system_param_map(offset);		
}

LED_STATUS_ONOFF *get_led_status_onoff_param(void)
{
	int offset;
	
	offset = offsetof(SYSTEM_PARAM, flash_param) +  offsetof(FLASH_PARAM, led_status_onoff);

	return (LED_STATUS_ONOFF *)system_param_map(offset);		
}

FLASH_PARAM *get_flash_param(void)
{
	int offset;
	
	offset = offsetof(SYSTEM_PARAM, flash_param);

	return (FLASH_PARAM *)system_param_map(offset);		
}

//获取内存当中共享内存的地址
 RAM_PARAM *get_ram_param(void)
{
	int offset;

	//求结构体中一个成员在该结构体中的偏移量	
	offset = offsetof(SYSTEM_PARAM, ram_param);

	return (RAM_PARAM *)system_param_map(offset);		
}

 FACTORY_PARAM *get_factory_param(void)
{
	int offset;
	
	offset = offsetof(SYSTEM_PARAM, factory_param);

	return (FACTORY_PARAM *)system_param_map(offset);		
}
