/* come from wpa_cli */
#include "common.h"
#include "wifi.h"
#include "wpa_cli_bak.h"
#include "debug.h"
#include "param.h"
#include "wireless_tool/iwlib.h"
#include "normal.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#undef		TARGET_LEVEL
#define		TARGET_LEVEL		LEVEL_INFO

/***************************START*********************************/
/************定义了一些函数中使用的工具*************/
int CWifi::get_line_from_buf(int index, char *line)
{
    int i = 0;
    int j = 0;
    int endcnt = -1;
    char *linestart = mRecvBuf;
    int len;

    while (1)
    {
        if (mRecvBuf[i] == '\n' || mRecvBuf[i] == '\r' || mRecvBuf[i] == '\0')
        {
            endcnt++;
            if (index == endcnt)//获取到了数据
            {
                len = &mRecvBuf[i] - linestart;
                strncpy(line, linestart, len);
                line[len] = '\0';
                return 0;
            }
            else
            {
                /* 更新linestart */
                for (j = i + 1; mRecvBuf[j]; )
                {
                    if (mRecvBuf[j] == '\n' || mRecvBuf[j] == '\r')
                        j++;
                    else
                        break;
                }
                if (!mRecvBuf[j])
                    return -1;
				
                linestart = &mRecvBuf[j];
                i = j;
            }
        }

        if (!mRecvBuf[i])
            return -1;
        i++;
    }
}


int CWifi::StdinGetChar(int Timeout)
{
    /* 
	 * 如果有数据就读取、处理、返回
	 * 如果没有数据, 立刻返回, 不等待
	 */

	/* select, poll 可以参数 UNIX环境高级编程 */

    struct timeval tTV;
    fd_set tFDs;
	char c;
	
    tTV.tv_sec = Timeout;
    tTV.tv_usec = 0;
    FD_ZERO(&tFDs);
	
    FD_SET(STDIN_FILENO, &tFDs); //STDIN_FILENO is 0
    if (Timeout == -1)
    {
        select(STDIN_FILENO+1, &tFDs, NULL, NULL, NULL);
	}
    else
    {
        select(STDIN_FILENO+1, &tFDs, NULL, NULL, &tTV);
	}

    if (FD_ISSET(STDIN_FILENO, &tFDs))
    {
		/* 处理数据 */		
		c = fgetc(stdin);
		return c;
    }
	else
	{
		return 0;
	}
}


int CWifi::StdinGetString(char *buf,int Timeout)
{
    /* 如果获得了第1个字符,以后的字符没有时间限制 */
    int c;
    int i = 0;

    c = StdinGetChar(Timeout);
    if (!c)
        return -1;
    if (c == '\n' || c == '\r')
        return -1;
	
    buf[i++] = c;
    while (c = StdinGetChar(-1))
    {
        if (c == '\n' || c == '\r')
        {
            buf[i] = '\0';
            return 0;
        }
        else
        {
            buf[i++] = c;
        }        
    }
    return 0;
}
/*****************************END*********************************/
/************定义了一些函数中使用的工具*************/
CWifi::CWifi(void)
{
	mWpaCtrl = NULL;
	mInit = 0;
	memset(mRecvBuf, 0, sizeof(mRecvBuf));
}

CWifi::~CWifi()
{

}

int CWifi::init(void)
{
	int i, max_net_id;	
	
	if (mInit)
		return 0;
	
	mWpaCtrl = wpa_init();
	if (mWpaCtrl == NULL)
	{
		LOG_ERROR("wpa_init fail\n");
		return SDK_ERROR;
	}

	max_net_id = wifi_add_network();
	for (i = 0; i <= max_net_id; i++)
		wifi_del_network(i);	 
	
	mInit = 1;
	
	return SDK_OK;
}

void CWifi::uninit(void)
{
	int i, max_net_id;
	
	if (!mInit)
		return;

	max_net_id = add_network();
	for (i = 0; i <= max_net_id; i++)
		wifi_del_network(i);	
	
	wpa_uninit();

	mInit = 0;
}

/*检测当前是否有无线连接*/
int CWifi::check_link_Y_N(void) 
{
	int reply_len = sizeof(mRecvBuf);
	int ret = 0;
	
	ret	= my_wpa_ctrl_command(mWpaCtrl, "STATUS", mRecvBuf, &reply_len);
	if (ret)		
	{
		return -1;		  
	}

	if (strstr(mRecvBuf, "wpa_state=COMPLETED"))
	{
		printf("\n");
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		printf("!!!!!!!!!The WIFI IS CONNECTED!!!!!!!!!!\n");
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		printf("\n");
		while(1);
		//对连接上的无线进行账号和密码保存
	}
	else if(strstr(mRecvBuf, "wpa_state=SCANNING"))
	{
		printf("\n");
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		printf("!!!!!!!!The WIFI IS SCANNINGING!!!!!!!!!\n");
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		printf("\n");

	}
	else
	{
		printf("\n");
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		printf("!!!!!!!!!The WIFI IS DISCONNECTED!!!!!!!\n");
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		printf("\n");
	}
}

int CWifi::check_link(const char *ssid)		/* 输入参数 */
{
	int reply_len = sizeof(mRecvBuf);

    my_wpa_ctrl_command(mWpaCtrl, "STATUS", mRecvBuf, &reply_len);	
	if (strstr(mRecvBuf, "wpa_state=COMPLETED"))
	{
		char delim[] = "\n";
		char *p = NULL;
		
		p = strtok(mRecvBuf, delim);
		while (p != NULL)
		{
			if (!strncmp(p, "ssid=", strlen("ssid=")))
			{
				sscanf(p, "ssid=%[^\n]", ssid);
				break;
			}
			p = strtok(NULL, delim);
		}
		return WIFI_LINKED;
	}
	else
		return WIFI_UNLINKED;

	return WIFI_UNLINKED; 
}

int CWifi::Ready_Linking(char **pStr)
{
	char cmd[1024];
	char input[1024];
	int  ap  = 0;

	char *SSID_Tmp = pStr[4];//取出SSID的数据
	int reply_len = sizeof(mRecvBuf);	
	
	if (my_wpa_ctrl_command(mWpaCtrl, "ADD_NETWORK", mRecvBuf, &reply_len) < 0)
		return -1;
	
	sscanf(mRecvBuf, "%d", &ap);
	if(set_network(ap,"ssid",pStr[4]) == SDK_ERROR)
	{
		return -1;
	}

	return 0;
}


/*
	int CWifi::Iput_Password(char **pStr)
	{
		char cmd[1024];
		char input[1024];
		int ap  = 0;

		char *SSID_Tmp = pStr[4];//取出SSID的数据
		int reply_len = sizeof(mRecvBuf);	
		
		sscanf(mRecvBuf, "%d", &ap);
		if(set_network(ap,"psk",SSID_Tmp) == SDK_ERROR)
		{
			return -1;
		}

		return 0;
	}
*/


int CWifi::add_network(void)
{
	int reply_len = sizeof(mRecvBuf);	
	
	if (my_wpa_ctrl_command(mWpaCtrl, "ADD_NETWORK", mRecvBuf, &reply_len) < 0)
		return SDK_ERROR;

	return atoi(mRecvBuf);
}

int CWifi::del_network(int index)
{
    char cmd[STR_64_LEN] = {0};
	int reply_len = sizeof(mRecvBuf);		

    sprintf(cmd, "REMOVE_NETWORK %d", index);
    return my_wpa_ctrl_command(mWpaCtrl, cmd, mRecvBuf, &reply_len);
}

int CWifi::set_network(int index,const char *name,const char *value)	/* SET SSID */
{
    char cmd[STR_128_LEN] = {0};	
	int reply_len = sizeof(mRecvBuf); 			

    sprintf(cmd, "SET_NETWORK %d %s \"%s\"", index, name, value);
	my_wpa_ctrl_command(mWpaCtrl, cmd, mRecvBuf, &reply_len);
	
	if (strstr(mRecvBuf, "OK"))
	{
		printf("The NETWORK_SET OK\n");
		return SDK_OK;
	}
	else if(strstr(mRecvBuf, "FAIL"))
	{
		printf("The NETWORK_SET FAIL\n");
		return SDK_ERROR;
	}
	else
	{
		printf("The NETWORK_SET Nothing\n");
	}

	return SDK_ERROR;
}



int CWifi::set_network2wep(int index, const char *name, const char *value)	/* SET SSID */
{
    char cmd[STR_128_LEN] = {0};	
	int reply_len = sizeof(mRecvBuf);			

    sprintf(cmd, "SET_NETWORK %d %s %s", index, name, value);
	my_wpa_ctrl_command(mWpaCtrl, cmd, mRecvBuf, &reply_len);

	if (strstr(mRecvBuf, "OK"))
	{
		printf("The NETWORK2_SET OK\n");
		return SDK_OK;
	}

	else if(strstr(mRecvBuf, "FAIL"))
	{
		printf("The NETWORK2_SET FAIL\n");
		return SDK_ERROR;
	}
	else
	{
		printf("The NETWORK2_SET Nothing\n");
	}

	return SDK_ERROR;
}

int CWifi::set_network3wep(int index, const char *name, int value)	/* SET SSID */
{
    char cmd[STR_128_LEN] = {0};	
	int reply_len = sizeof(mRecvBuf);			

    sprintf(cmd, "SET_NETWORK %d %s %d", index, name, value);
	my_wpa_ctrl_command(mWpaCtrl, cmd, mRecvBuf, &reply_len);

	if (strstr(mRecvBuf, "OK"))
		return SDK_OK;
	else if(strstr(mRecvBuf, "FAIL"))
		return SDK_ERROR;

	return SDK_ERROR;
}

int CWifi::popen_cmd(char *cmd, char *type, char *ret, int retSize)
{
	FILE *fp = NULL;
	
	if (NULL == (fp = popen(cmd, type)))
	{		
		LOG_ERROR("popen %s error", cmd);
		return SDK_ERROR;	
	}	
	memset(ret, 0, retSize);
	fread(ret, retSize-1, 1, fp);	
	pclose(fp);
	
	return SDK_OK;
}

int CWifi::select_network(int index)
{
	char cmd[STR_64_LEN];
	int  reply_len = sizeof(mRecvBuf);			
	
	sprintf(cmd, "SELECT_NETWORK %d", index);
	return my_wpa_ctrl_command(mWpaCtrl, cmd, mRecvBuf, &reply_len);
}

int CWifi::enable_network(int index)
{
    char cmd[STR_64_LEN];
	int reply_len = sizeof(mRecvBuf);			
	
	sprintf(cmd, "ENABLE_NETWORK %d", index);
	
    return my_wpa_ctrl_command(mWpaCtrl, cmd, mRecvBuf, &reply_len);
}

int CWifi::disable_network(int index)
{
    char cmd[STR_64_LEN];
	int reply_len = sizeof(mRecvBuf);		
	
	sprintf(cmd, "DISABLE_NETWORK %d", index);
	
    return my_wpa_ctrl_command(mWpaCtrl, cmd, mRecvBuf, &reply_len);
}

int CWifi::scan(void)
{
	int reply_len = sizeof(mRecvBuf);		

	return my_wpa_ctrl_command(mWpaCtrl, "SCAN", mRecvBuf, &reply_len);
}	

int CWifi::scan_result(void)
{
	int reply_len = sizeof(mRecvBuf);		
	int i;
	char line[128];
		
    char bssid[128];
    char freq[128];
    char signal[128];
    char flags[128];
    char ssid[128];


	//wpa扫描失败
	if (my_wpa_ctrl_command(mWpaCtrl, "SCAN_RESULTS", mRecvBuf, &reply_len) < 0)
	{
		LOG_INFO("SCAN_RESULTS\n");
		return SDK_ERROR;
	}
	//wpa扫描成功
	else
	{
        for (i = 1; !get_line_from_buf(i, line); i++)
        {
            sscanf(line, "%s %s %s %s %s", bssid, freq, signal, flags, ssid);
            printf("%02d %-32s %s\n", i, ssid, flags);
        }
	}

	return SDK_OK;
}




int wifi_init(void)
{
	return CWifi::getInstance().init();
}

void wifi_uninit(void)
{
	return CWifi::getInstance().uninit();
}

int wifi_add_network(void)
{
	return CWifi::getInstance().add_network();
}

int wifi_Ready_Linking(char **pStr,int *ap)
{
	return CWifi::getInstance().Ready_Linking(pStr);
}


int wifi_del_network(int index)
{
	return CWifi::getInstance().del_network(index);
}

int wifi_set_network(int index,const char *name,const char *value)
{
	return CWifi::getInstance().set_network(index, name, value);
}
/*add by zhang*/
int wifi_set_network2wep(int index, const char *name, const char *value)
{
	return CWifi::getInstance().set_network2wep(index, name, value);
}
int wifi_set_network3wep(int index, const char *name, int value)
{
	return CWifi::getInstance().set_network3wep(index, name, value);
}

int wifi_select_network(int index)
{
	return CWifi::getInstance().select_network(index);
}
int wifi_is_wep(char *ssid)
{
	int ret = -1;
	char buf[32] = {0};
	char wpa_cmd[96] = {0};
	
	sprintf(wpa_cmd, "wpa_cli scan_results | grep %s | grep -v grep | awk '{print $4}'", ssid);
	ret = CWifi::getInstance().popen_cmd(wpa_cmd, "r", buf, sizeof(buf));
	if( 0 != ret )
	{
		LOG_ERROR("popen_cmd fail!!!\n");
		return -1;
	}/*popen get success*/
	if (memcmp_ext(buf, "[WEP]", 5) == 0)
	{
		return AUTH_WEP;
	}
	else if (memcmp_ext(buf, "[ESS]", 5) == 0)
	{
		return AUTH_OPEN;
	}
	else
	{
		return AUTH_WPA;
	}
}

int wifi_enable_network(int index)		
{
	return CWifi::getInstance().enable_network(index);
}

int wifi_disable_network(int index)
{
	return CWifi::getInstance().disable_network(index);
}

int wifi_scan(void)
{
	return CWifi::getInstance().scan();
}

int wifi_scan_result(void)
{
	return CWifi::getInstance().scan_result();
}


int wifi_check_link(const char *ssid)
{
	return CWifi::getInstance().check_link(ssid);
}

int wifi_check_link_Y_N(void)
{
	return CWifi::getInstance().check_link_Y_N();
}

int wifi_Enter_Ssid(char **pStr,int *ap)
{
	int  ret = 0;
	char line[128];
	char input[64];	

	//使用static防止在函数退出后地址空间被其他的数据填充
    static char bssid[64];
    static char freq[64];
    static char signal[64];
    static char flags[64];
    static char ssid[64];

	printf("\n");
	printf("Q: Quit\n");
	printf("Please Iput the index of AP : ");
	printf("\n");
	fflush(stdout);

	ret = CWifi::getInstance().StdinGetString(input, 5);
	if (ret)
	{
		return -1;
	}
	if (input[0] == 'q' || input[0] == 'Q')
	{
		printf("\n");
		printf("Quit Successfully\n");
		printf("\n");
		return -1;
	}

	sscanf(input, "%d", ap);
	printf("\n");
	printf("You have selected The No.%d AP\n",*ap);
	printf("\n");
	CWifi::getInstance().get_line_from_buf(*ap,line);
	//line中保存了所选择的
	sscanf(line, "%s %s %s %s %s", bssid, freq, signal, flags, ssid);

	pStr[0] = bssid;
	pStr[1] = freq;
	pStr[2] = signal;
	pStr[3] = flags;
	pStr[4] = ssid;

	printf("\n");
	printf("the bssid : %s\n",pStr[0]);
	printf("the freq  : %s\n",pStr[1]);
	printf("the signal: %s\n",pStr[2]);
	printf("the flags : %s\n",pStr[3]);
	printf("the SSID  : %s\n",pStr[4]);
	printf("\n");
	
	return 0;	
}
int wifi_Enter_Password(char **pStr,int net_id)
{
	char input[64];
	int ret = 0;
	char *Flag_tmp = pStr[3];
	
	if(strstr(Flag_tmp, "WPA"))//WPA加密的无线
	{
		printf("\n");
		printf("Please Input The Password : ");
		printf("\n");
		fflush(stdout);
		
		ret = CWifi::getInstance().StdinGetString(input, -1);
		if (ret)
		{
			return -1;
		}
		
		printf("Please Input The Password : ");
		/*不同的加密方式使用不同的连接方式*/
		printf("\n");
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		printf("The Wifi Type is %s\n",pStr[3]);
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		printf("WPA Model\n");
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		wifi_set_network(net_id, "psk", input);
		wifi_select_network(net_id);
		wifi_enable_network(net_id);

	}
	else//开放密码的无线
	{
		printf("\n");
		printf("The SSID is open,Connecting!\n");
		printf("\n");
		wifi_set_network2wep(net_id, "key_mgmt", "NONE");
		wifi_select_network(net_id);
		wifi_enable_network(net_id);
	}

	return 0;
}

