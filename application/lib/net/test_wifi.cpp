#include "wifi.h"
#include "debug.h"
#include "param.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#include <termios.h>
#include <unistd.h>



static int StdinDevInit(void)
{
    struct termios tTTYState;
 
    //get the terminal state
    tcgetattr(STDIN_FILENO, &tTTYState);
 
    //turn off canonical mode
    tTTYState.c_lflag &= ~ICANON;
    //minimum of number input read.
    tTTYState.c_cc[VMIN] = 1;   /* 有一个数据时就立刻返回 */

    //set the terminal attributes.
    tcsetattr(STDIN_FILENO, TCSANOW, &tTTYState);

	return 0;
}


static int StdinDevExit(void)
{

    struct termios tTTYState;
 
    //get the terminal state
    tcgetattr(STDIN_FILENO, &tTTYState);
 
    //turn on canonical mode
    tTTYState.c_lflag |= ICANON;
	
    //set the terminal attributes.
    tcsetattr(STDIN_FILENO, TCSANOW, &tTTYState);	
	return 0;
}




int main(int argc, char *argv[])
{	
	int AP_Num = 0;
	char *str = NULL;//里面存储了SSID
	int net_id = 0;
	int net_return = 0;

	char *wifi_value[5];//在这个数组中存放了bssid, freq, signal, flags, ssid


	if (wifi_init() < 0)
	{
		LOG_ERROR("wifi_init fail\n");
		goto  OUT;
	}
	
	StdinDevInit();
	while(1)
	{
		//这里主要是扫描无线信号
		//对已经连接的信号进行账号和密码的保存
		if(wifi_check_link_Y_N() < 0)
		{
			LOG_ERROR("check_link_Y_N fail\n");
			continue;
		}
		if (wifi_scan() < 0)
		{
			LOG_ERROR("wifi_scan fail\n");
			continue;
		}
		/* 打印出来 */
		if (wifi_scan_result() < 0)
		{
			LOG_ERROR("wifi_scan_result fail\n");
			continue;
		}
        /* 让用户选择某个AP */
		if(wifi_Enter_Ssid(wifi_value,&AP_Num)<0)
		{
			LOG_ERROR("wifi_Enter_Ssid\n");
			continue;
		}
		/* 创建一个network */
		net_id = wifi_add_network();
		LOG_INFO("net_id [%d]\n", net_id);
		wifi_set_network(net_id, "ssid" , wifi_value[4]);

		/* 让用户输入密码 */	
		LOG_INFO("User is going to input the password\n");
		if(wifi_Enter_Password(wifi_value,net_id)<0)
		{
			LOG_ERROR("wifi_Enter_Password\n");
			continue;
		}
	}
	
OUT:
	wifi_uninit();
	StdinDevExit();

	return 0;
}


