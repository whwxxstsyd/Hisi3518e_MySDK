#include "param.h"
#include "debug.h"
#include "task/task_net.h"
#include "param_base.h"
#include "net.h"
#include "wifi.h"
#include "param_str.h"
#include "cgi_server.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "av_buffer.h"
#include "task.h"
#include "mp4v2/mp4v2.h"


#undef		TARGET_LEVEL
#define		TARGET_LEVEL		LEVEL_INFO

CTaskNet::CTaskNet(void)
{
	strcpy(mTaskName, "NET");
}

CTaskNet::~CTaskNet(void)
{

}	

int config_wifi(WIFI_PARAM *pWifiParam)
{
	int wifi_id;
	int len = 0;

	wifi_id = wifi_add_network();
	if (wifi_id < 0)
	{
		return SDK_ERROR;
	}
	
	if((0 == pWifiParam->auth) || (AUTH_WEP == pWifiParam->auth))/*第二次连接时候也要根据路由wep加密类型连接*/
	{
		len = strlen(pWifiParam->key);
		if(wifi_set_network(wifi_id, "ssid", pWifiParam->ssid) < 0)
			goto ERROR_OUT;
		if(wifi_set_network2wep(wifi_id, "key_mgmt", "NONE") < 0)
			goto ERROR_OUT;
		if((10 == len) || (26 == len))
		{
			if(wifi_set_network2wep(wifi_id, "wep_key0", pWifiParam->key) < 0)
				goto ERROR_OUT;
		}
		else if((5 == len) || (13 == len))
		{
			if(wifi_set_network(wifi_id, "wep_key0", pWifiParam->key) < 0)
				goto ERROR_OUT;
		}
		
		if(wifi_set_network3wep(wifi_id, "wep_tx_keyidx", 0) < 0)
			goto ERROR_OUT;	
		if(wifi_select_network(wifi_id) < 0)
			goto ERROR_OUT;
		if(wifi_enable_network(wifi_id) < 0)
			goto ERROR_OUT;
		
	}
	else
	{
		if (wifi_set_network(wifi_id, "ssid", pWifiParam->ssid) < 0)
			goto ERROR_OUT;
		if (wifi_set_network(wifi_id, "psk", pWifiParam->key) < 0)
			goto ERROR_OUT;
		if (wifi_enable_network(wifi_id) < 0)
			goto ERROR_OUT;

		LOG_INFO("id: %d, ssid: %s key: %s\n", wifi_id, pWifiParam->ssid, pWifiParam->key);	
	}
	return SDK_OK;

ERROR_OUT:
	wifi_del_network(wifi_id);
	return SDK_ERROR;
}

#if 0
static int config_net(const char *inf, NET_PARAM *pNetParam)
{
	int net_type = 0;
	char cmd[STR_128_LEN] = {0};

	if ((NULL == pNetParam) || (inf == NULL))
	{
		LOG_ERROR("param error\n");
		return -1;
	}

	netcfg_set_ip_addr(inf, pNetParam->ip);
	netcfg_set_mask_addr(inf, pNetParam->netmask);	
	netcfg_set_gw_addr(inf, pNetParam->gateway);
	netcfg_set_dns(pNetParam->dns1, pNetParam->dns2);
	
	/* 再考虑DHCP */
	if (pNetParam->dhcp == DHCP_ALL)		/* 优先dhcp_all */
	{
		LOG_DEBUG("dhcp all\n");
		system("killall udhcpc");
		sprintf(cmd, "udhcpc -s /etc/udhcpc_all.default -i %s &", inf);
		system(cmd);
	}
	else if (pNetParam->dhcp == DHCP_IP)
	{
		LOG_DEBUG("dhcp ip\n");	
		system("killall udhcpc");
		sprintf(cmd, "udhcpc -s /etc/udhcpc_ip.default -i %s", inf);
		system(cmd);
	}

	return net_type;
}
#endif
void *wifi_status_check(void *arg)
{
	int old_netstatus = 0 ,count = 0;
	int shakecnt=0,shakestatus = 0;
	char ssid[32] = {0};
	RAM_PARAM *pRamParam = get_ram_param();
	FACTORY_PARAM *pFactoryParam = get_factory_param();
	while(1)
	{
		sleep(2);
		if (old_netstatus != pRamParam->net_status)
		{
			LOG_INFO("WIFI Status:[%d] to [%d]\n",old_netstatus,pRamParam->net_status);
			//don't change position of the code,the net_status is changing possible
			old_netstatus = pRamParam->net_status; 
			switch(pRamParam->net_status)
			{
				case NET_IDLE:
					set_param_int(RED_LED_STR, LED_ON);
					break;
				case NET_LINKING:
					set_param_int(RED_LED_STR, LED_BLUE_FLASH);
					break;
				case NET_AP:
					set_param_int(RED_LED_STR, LED_RED_FLASH);
					break;
				case NET_SMART_LINK:
					set_param_int(RED_LED_STR, LED_FLASH_QUICK);
					break;
				case NET_LINKED:
					set_param_int(RED_LED_STR, LED_OFF);
					if(0 == strncmp(pFactoryParam->region, "china", sizeof(pFactoryParam->region)))
					play_audio_file(AUDIO_WIFI_OK_CH);
					else
					play_audio_file(AUDIO_WIFI_OK_EN);
					break;
				case NET_UNLINKED:
					set_param_int(RED_LED_STR, LED_BLUE_FLASH);
					if(0 == strncmp(pFactoryParam->region, "china", sizeof(pFactoryParam->region)))
					play_audio_file(AUDIO_WIFI_FAIL_CH);
					else
					play_audio_file(AUDIO_WIFI_FAIL_EN);
					break;
				default:
					LOG_ERROR("error net status!\n");
					break;
			}

		}
		else if (pRamParam->net_status == NET_LINKING)
		{
			if (wifi_check_link(ssid) == WIFI_LINKED)
				pRamParam->net_status = NET_LINKED;
		}
		else if ((pRamParam->net_status == NET_LINKED) || (pRamParam->net_status == NET_UNLINKED))
		{			
			if (wifi_check_link(ssid) == WIFI_LINKED)
				shakestatus = NET_LINKED;
			else
				shakestatus = NET_UNLINKED;
			if (pRamParam->net_status != shakestatus)
			{
				//网络防抖动
				shakecnt++;
				if (shakecnt >= 2)
				{
					shakecnt = 0;
					pRamParam->net_status = shakestatus;
				}
			}
		}		
	}
	return NULL;
}

void CTaskNet::Process(void)
{
	int eth0_exist, wlan0_exist, len, ret,i,keylen;
	char eth_mac[6] = {0}, wlan_mac[6] = {0};
	pthread_t wifista_thrID;
	NET_PARAM *pNetParam = get_net_param();
	WIFI_PARAM *pWifiParam = get_wifi_param();
	RAM_PARAM *pRamParam = get_ram_param();	
	char data[2048] = {0}, ssid[32] = {0}, ap_ssid[STR_64_LEN] = {0}, *pStr = NULL;

#if 0		/* mac地址不同,否则在连接wifi smartlink的时候有线网络会掉线 */
	netcfg_get_mac_addr("eth0", eth_mac);
	netcfg_get_mac_addr("ra0", wlan_mac);
	if (memcmp(eth_mac, wlan_mac, sizeof(eth_mac)))		/* 同步mac地址 */
	{
		char mac[STR_64_LEN] = {0};

		sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x", wlan_mac[0], wlan_mac[1], wlan_mac[2], 
													 wlan_mac[3], wlan_mac[4], wlan_mac[5]);
		LOG_INFO("set mac addr[%s]\n", mac);
		if (strcmp(mac, "00:00:00:00:00:00"))
		{
			if (netcfg_set_mac_addr("eth0", mac) != 0)	/* 在NFS下无法修改mac地址 */
				LOG_WARN("netcfg_set_mac_addr FAIL\n");
		}
	}
	netcfg_get_mac_addr("eth0", eth_mac);
	sprintf(mRamParam->net_param.mac, "%02x:%02x:%02x:%02x:%02x:%02x", eth_mac[0], eth_mac[1], eth_mac[2], 
												 eth_mac[3], eth_mac[4], eth_mac[5]);
	strcpy(mNetParam->mac ,mRamParam->net_param.mac);
#else
	netcfg_get_mac_addr("waln0", wlan_mac);
	wlan_mac[5] += 1;	/* eth0和wifi mac地址相邻 */
	sprintf(pNetParam->mac, "%02x:%02x:%02x:%02x:%02x:%02x", wlan_mac[0], wlan_mac[1], wlan_mac[2], 
												 wlan_mac[3], wlan_mac[4], wlan_mac[5]);	
	netcfg_set_mac_addr("eth0", pNetParam->mac);	
#endif

	/* 修改AP模式SSID */
	read_data_from_file(AP_CONFIG_FILE, data, &len);
#ifdef 	TUTK_P2P_PLATFORM
	FACTORY_PARAM *pFactoryParam = get_factory_param();
	sprintf(ap_ssid, "%s", AP_SSID_HEADER);
	if ((pStr = strstr(data, ap_ssid)))
	{
		for(i=0; i<6; i++)
		{
			pStr[strlen(AP_SSID_HEADER)+i] = pFactoryParam->uid[14+i];
		}
		write_data_to_file(AP_CONFIG_FILE, data, strlen(data));
	}
#else
	sprintf(ap_ssid, "%s", AP_SSID_HEADER);
	if ((pStr = strstr(data, ap_ssid)))
	{
		if ((wlan_mac[4] / 16) > 9)
			pStr[strlen(AP_SSID_HEADER)] = wlan_mac[4] / 16 -10 + 'A';
		else
			pStr[strlen(AP_SSID_HEADER)] = wlan_mac[4] / 16 + '0';
		if ((wlan_mac[4] % 16) > 9)
			pStr[strlen(AP_SSID_HEADER) + 1] = wlan_mac[4] % 16 -10 + 'A';
		else
			pStr[strlen(AP_SSID_HEADER) + 1] = wlan_mac[4] % 16 + '0';	
		//上面wlan_mac[5]加了1
		if (((wlan_mac[5] - 1) / 16) > 9)	
			pStr[strlen(AP_SSID_HEADER) + 2] = (wlan_mac[5] - 1) / 16 -10 + 'A';
		else
			pStr[strlen(AP_SSID_HEADER) + 2] = (wlan_mac[5] - 1) / 16 + '0';
		if (((wlan_mac[5] - 1) % 16) > 9)
			pStr[strlen(AP_SSID_HEADER) + 3] = (wlan_mac[5] - 1) % 16 - 10 + 'A';
		else
			pStr[strlen(AP_SSID_HEADER) + 3] = (wlan_mac[5] - 1) % 16 + '0';
//		sprintf(pStr, "%s%02x%02x", AP_SSID_HEADER, wlan_mac[4], wlan_mac[5]-1);
		write_data_to_file(AP_CONFIG_FILE, data, strlen(data));
	}
#endif

	/* 启动网卡 */
	eth0_exist  = netcfg_active("eth0", "up");
	wlan0_exist = netcfg_active("ra0", "up");	

	keylen  = strlen(pWifiParam->key);
	if (strlen(pWifiParam->ssid) && (((keylen == 0) && (pWifiParam->auth == AUTH_OPEN)) || keylen))
	{
		//set_param_int(RED_LED_STR, LED_FLASH_SLOW);	
		pRamParam->net_status = NET_LINKING;
		//config_wifi(pWifiParam);
		connect_wifi(pWifiParam->ssid,pWifiParam->key,pWifiParam->auth);
	}

#ifdef	ONLY_WIFI		/* 卡片机,无有线网卡 */		
	LOG_INFO("change network to wifi\n");
	//check wifi status
	CreateDetachedTask(&wifista_thrID,wifi_status_check,NULL);
	pRamParam->system_status.net_inf = USE_WLAN;			
	config_net("ra0", pNetParam);
#endif	

#ifdef 	BOTH_NET	
	while (mRunning)
	{
		if ((eth0_exist == SDK_OK) && netcfg_netlink_status("eth0") && (pRamParam->system_status.net_inf != USE_ETH))		/* 优先有线网络 */
		{
			LOG_INFO("change network to eth0\n");	
			system("/bin/ip address flush ra0");
			pRamParam->system_status.net_inf = USE_ETH;
			config_net("ra0", pNetParam);
		}
		else if ((wlan0_exist == SDK_OK) && 
				 (netcfg_netlink_status("eth0") == 0) &&		/* 有线断开则考虑无线 */
				 (wifi_check_link(pWifiParam->ssid) == WIFI_LINKED) && 
				 (pRamParam->system_status.net_inf != USE_WLAN))			
		{
			LOG_INFO("change network to wifi\n");
			system("/bin/ip address flush eth0");
			pRamParam->system_status.net_inf = USE_WLAN;			
			config_net("ra0", pNetParam);	
		}
		sleep(1);
	}
#endif	
}


int CTaskNet::Init(void)
{
	system("killall udhcpc");
	
	return SDK_OK;
}

void CTaskNet::Uninit(void)
{

}

int net_task_create(void)
{
	return CTaskNet::getInstance()->Create();
}

void net_task_destory(void)
{
	return CTaskNet::getInstance()->Destory();
}
