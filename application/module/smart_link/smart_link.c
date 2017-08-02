#include "debug.h"
#include "linux_list.h"
#include "common.h"
#include "wireless_tool/iwlib.h"	/* for [struct iwreq]... */
#include "param_base.h"
#include "normal.h"
#include "param.h"
#include "param_str.h"


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <net/if.h>                 /* struct ifreq */
#include <net/ethernet.h>           /* struct ether_header */
#include <linux/if_packet.h>        /* struct sockaddr_ll */
#include <linux/sockios.h>          /* SIOCGIFINDEX */

#undef		TARGET_LEVEL
#define		TARGET_LEVEL		LEVEL_INFO

//#define     SMARTLINK_FILE      "/proc/smart_connection"
#define     SMARTLINK_FILE      "/proc/elian"

void StartSmartlink(void)
{
   // system("echo \"start\" > /proc/smart_connection"); echo "start" > /proc/elian iwpriv wlan0 elian result
   system("echo \"start\" > /proc/elian");
   LOG_DEBUG("start smartlink\n");   	
}

void StopSmartlink(void)
{
  //  system("echo \"stop\" > /proc/smart_connection");
    system("echo \"stop\" > /proc/elian");
    LOG_DEBUG("stop smartlink\n");
}

void ClearSmartlink(void)
{
    //system("echo \"clear\" > /proc/smart_connection");
     system("echo \"clear\" > /proc/elian");
}

int DisplayResult(char *ssid, char *key, char *authmode)
{
    int fd;
    char data[256] = {0};

    if ((NULL == ssid) || (NULL == key))
        return -1;

    fd = open(SMARTLINK_FILE, O_RDONLY);
    if (fd < 0)
    {
        LOG_ERROR("open fail[%s]\n", SMARTLINK_FILE);
        return -1;
    }
    if (read(fd, data, sizeof(data)) < 0)
    {
        LOG_ERROR("read fail\n");
        close(fd);
        return -1;
    }
    //sscanf(data, "%*[^:]: %[^\n]%*[^:]: %[^\n]%*[^:]: %[^\n]",authmode, ssid, key);
	sscanf(data, "AM=%[^ ]%*[^=]=%[^ ]%*[^=]=%[^ ]",authmode,ssid,key);
    if ((strcmp(ssid, "PASSWORD\t: ") == 0) && (strcmp(key, "0") == 0))
    {
        close(fd);        
        return 0;
    }
    if (strlen(ssid))
    {
    	set_param_int(RED_LED_STR, LED_BLUE_FLASH);/*蓝灯闪烁*/
        printf("ssid: %s, key: %s\n, authmode:%s\n", ssid, key, authmode);        
        close(fd);        
        return 1;
    }
    close(fd);
    
    return 0;
}

int checktime(struct timespec start, int timeout_ms)   /* 1: 超时, 0: 没超时 */
{
	struct timespec current;

	clock_gettime(CLOCK_MONOTONIC, &current);
    if ((current.tv_sec - start.tv_sec < timeout_ms / 1000) ||
        ((current.tv_sec - start.tv_sec == timeout_ms / 1000) && (current.tv_nsec - start.tv_sec) < (timeout_ms % 1000 * 1000)))
        return 0;
    else
        return 1;
}

int SmartLinkProc(char *ssid, char *key, char *authmode, int timeout_ms)
{
    int ret;
	struct timespec start;
    RAM_PARAM *pRamParam = NULL;


    LOG_INFO("enter SmartLinkProc\n");
    pRamParam = get_ram_param();
    StartSmartlink();   
    clock_gettime(CLOCK_MONOTONIC, &start);   
    while (1)
    {
        usleep(100 * 1000);
        ret = DisplayResult(ssid, key, authmode);
        if (ret == 1)
            break;
        else if (ret < 0)       /* 文件读取失败 */
            break;
        /*     不做超时
        if (checktime(start, timeout_ms) > 0) 
        {
            ret = -1;
            break;          
        }
        */
        if (pRamParam->smart_link_flag == 0)
        {
            ret = -1;
            break;
        }
    }

    StopSmartlink();    
    ClearSmartlink();
    
    return ret;
}

