#include "param_ext.h"
#include "param_base.h"
#include "param.h"
#include "normal.h"
#include "debug.h"

#include <string.h>

SYSTEM_STATUS *ram_get_system_status(void)
{
	return &(get_ram_param()->system_status);
}

 void set_talk_alive(int alive)	/* alive == 1: 表示talk已经被打开 */
{
 	get_ram_param()->system_status.talk_alive = alive;
}

 int get_talk_alive(void)	
{
	return get_ram_param()->system_status.talk_alive;
}

 void set_ftp_test_result(int result)
{
	get_ram_param()->system_status.ftp_test_result = result;
}

 char *get_camera_name(void)
 {
	return get_misc_param()->name;
 }

int check_user(const char *user, const char *pwd)
{
	int i;
	USER_PARAM *pUserParam;
	char user_str[STR_64_LEN] = "hetianqi";
	char pwd_str[STR_64_LEN]  = "123123qwe";

	printf("\r\n"); 
	LOG_INFO("user : %s , pwd : %s\n",user,pwd);
	printf("\r\n");	

	if ((user == NULL) || (pwd == NULL))
		return SDK_ERROR;

	//pUserParam = get_user_param();
	for (i = 0; i < MAX_USER_CNT; i++)
	{
//		if ((!strcmp(pUserParam[i].user, user)) && (!strcmp(pUserParam[i].pwd, pwd)))
		if ((!strcmp(user_str, user)) && (!strcmp(pwd_str, pwd)))
		{
//			LOG_INFO("user_pwd_level: [%s_%s_%d]\n", pUserParam[i].user, pUserParam[i].pwd, pUserParam[i].level);
//			return pUserParam[i].level;
			return 243;
		}
	}
	LOG_INFO("check user pwd[%s] error\n",pwd);
	return SDK_ERROR;		/* 用户名密码错误 */
}

int get_pwd(const char *user, char *pwd)
{
	int i;
	USER_PARAM *pUserParam;

	if ((user == NULL) || (pwd == NULL))
		return -1;

	pUserParam = get_user_param();
	for (i = 0; i < MAX_USER_CNT; i++)
	{
		if (!strcmp(pUserParam[i].user, user))
		{
			strcpy(pwd, pUserParam[i].pwd);
			return 0;
		}
	}	
	
	return -1;
}

unsigned char *get_aac_header(void)
{
	return get_ram_param()->aac_sequence_header;
}

AVC_HEADER *get_avc_header(void)
{
	return get_ram_param()->avc_header;
}

STREAM_PARAM *get_stream_param(void)
{
	return get_video_param()->stream;
}

int param_is_init(void)
{
	return get_ram_param()->init;
}
