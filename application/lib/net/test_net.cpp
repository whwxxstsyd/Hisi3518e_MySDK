#include "net.h"
#include "debug.h"
#include "wifi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#if 0
int main(int argc, char *argv[])
{	
	while (1)
	{
		LOG_INFO("eth0: %d, wlan0: %d\n", netcfg_netlink_status("eth0"), 
											wifi_check_link());
		sleep(1);
	}

	return 0;
}
#elif 0
int main(int argc, char *argv[])
{
	int ret;

	ret = netcfg_active("eth0", "up");
	LOG_INFO("ret: %d\n", ret);
	ret = netcfg_active("wlan0", "up");
	LOG_INFO("ret: %d\n", ret);
	
	return 0;
}
#elif 1
int main(int argc, char *argv[])
{
#if 0
	int fd, sockfd, nLen;
	struct sockaddr_in adrrClient;	

	printf("before xx");	

	while(1) sleep(1);
	nLen = sizeof(adrrClient);
	printf("before tcp_create_and_listen");		
	fd = tcp_create_and_listen(555);
	printf("after tcp_create_and_listen");	
	while (1)
	{
		printf("before accept");
		if ((sockfd = accept(fd, (struct sockaddr *)&adrrClient, (socklen_t *)&nLen)) < 0)
		{
			usleep(100 * 1000);
			continue;
		}
		printf("end accept\n");
	    LOG_DEBUG("Remote IP:%s, PORT:%d\n", inet_ntoa(adrrClient.sin_addr), 
            (unsigned short)ntohs(adrrClient.sin_port)); 		
	}

	return 0;
#else
	int fd, sockfd, nLen;
//	struct sockaddr_in adrrClient;	
 
	printf("before xx");	
 
 	while(1) sleep(1);

//	return 0;
#endif
}
#else
int main(int argc, char *argv[])
{
	while (1)
	{
		printf("hello=========\n");
		sleep(1);
	}

	return 0;
}
#endif

