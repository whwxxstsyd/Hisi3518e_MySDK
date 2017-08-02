#include "normal.h"
#include "net.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/route.h>
#include <net/if_arp.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netinet/if_ether.h>
#include <linux/sockios.h>

static const struct arg1opt Arg1Opt[] = 
{
	{"SIOCSIFMETRIC",  SIOCSIFMETRIC,  ifreq_offsetof(ifr_metric)},
	{"SIOCSIFMTU",     SIOCSIFMTU,     ifreq_offsetof(ifr_mtu)},
	{"SIOCSIFTXQLEN",  SIOCSIFTXQLEN,  ifreq_offsetof(ifr_qlen)},
	{"SIOCSIFDSTADDR", SIOCSIFDSTADDR, ifreq_offsetof(ifr_dstaddr)},
	{"SIOCSIFNETMASK", SIOCSIFNETMASK, ifreq_offsetof(ifr_netmask)},
	{"SIOCSIFBRDADDR", SIOCSIFBRDADDR, ifreq_offsetof(ifr_broadaddr)},
	{"SIOCSIFHWADDR",  SIOCSIFHWADDR,  ifreq_offsetof(ifr_hwaddr)},
	{"SIOCSIFDSTADDR", SIOCSIFDSTADDR, ifreq_offsetof(ifr_dstaddr)},
	{"SIOCSIFMAP",     SIOCSIFMAP,     ifreq_offsetof(ifr_map.mem_start)},
	{"SIOCSIFMAP",     SIOCSIFMAP,     ifreq_offsetof(ifr_map.base_addr)},
	{"SIOCSIFMAP",     SIOCSIFMAP,     ifreq_offsetof(ifr_map.irq)},
	/* Last entry if for unmatched (possibly hostname) arg. */
	{"SIOCSIFADDR",    SIOCSIFADDR,    ifreq_offsetof(ifr_addr)},
};


static const struct options OptArray[] = 
{
	{"metric",       N_ARG,         ARG_METRIC,      0},
    {"mtu",          N_ARG,         ARG_MTU,         0},
	{"txqueuelen",   N_ARG,         ARG_TXQUEUELEN,  0},
	{"dstaddr",      N_ARG,         ARG_DSTADDR,     0},
	{"netmask",      N_ARG,         ARG_NETMASK,     0},
	{"broadcast",    N_ARG | M_CLR, ARG_BROADCAST,   IFF_BROADCAST},
	{"hw",           N_ARG,         ARG_HW,          0},
	{"pointopoint",  N_ARG | M_CLR, ARG_POINTOPOINT, IFF_POINTOPOINT},
	{"mem_start",    N_ARG,         ARG_MEM_START,   0},
	{"io_addr",      N_ARG,         ARG_IO_ADDR,     0},
	{"irq",          N_ARG,         ARG_IRQ,         0},
	{"arp",          N_CLR | M_SET, 0,               IFF_NOARP},
	{"trailers",     N_CLR | M_SET, 0,               IFF_NOTRAILERS},
	{"promisc",      N_SET | M_CLR, 0,               IFF_PROMISC},
	{"multicast",    N_SET | M_CLR, 0,               IFF_MULTICAST},
	{"allmulti",     N_SET | M_CLR, 0,               IFF_ALLMULTI},
	{"dynamic",      N_SET | M_CLR, 0,               IFF_DYNAMIC},
	{"up",           N_SET        , 0,               (IFF_UP | IFF_RUNNING)},
	{"down",         N_CLR        , 0,               IFF_UP},
	{ NULL,          0,             ARG_HOSTNAME,    (IFF_UP | IFF_RUNNING)}
};

//============================================================================
//* 函数名称: in_ether()
//* 功能描述: 地址转换
//* 其他说明:
//============================================================================
static int in_ether(char *bufp, struct sockaddr *sap)
{
	unsigned char *ptr;
	int i, j;
	unsigned char val;
	unsigned char c;

	sap->sa_family = ARPHRD_ETHER;
	ptr = (unsigned char *)sap->sa_data;

	i = 0;
	do {
		j = val = 0;
		
		/* We might get a semicolon here - not required. */
		if (i && (*bufp == ':')) {
			bufp++;
		}

		do {
			c = *bufp;
			if (((unsigned char)(c - '0')) <= 9) {
				c -= '0';
			} else if (((unsigned char)((c|0x20) - 'a')) <= 5) {
				c = (c|0x20) - ('a'-10);
			} else if (j && (c == ':' || c == 0)) {
				break;
			} else {
				return -1;
			}
			++bufp;
			val <<= 4;
			val += c;
		} while (++j < 2);
		*ptr++ = val;
	} while (++i < ETH_ALEN);

	return (int) (*bufp);	/* Error if we don't end at end of string. */
}


//============================================================================
// 函数名称: netcfg_set_mac_addr()
// 功能描述: 设置指定名字网卡的物理地址
// 其他说明:
//============================================================================
int netcfg_set_mac_addr(const char *name, char *addr)  			// name : eth0 eth1 lo and so on
{												  			// addr : 12:13:14:15:16:17
	const struct arg1opt *a1op;
	const struct options *op;
	int 	sockfd;
	struct 	ifreq ifr;
	struct 	sockaddr sa;
	unsigned char mask;
	char 	*p = "hw";
	char 	host[128];
	
	/* Create a channel to the NET kernel. */
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return SDK_ERROR;
	
	/* get interface name */
	safe_strncpy(ifr.ifr_name, name, IFNAMSIZ);
	mask = N_MASK;
	
	for (op = OptArray; op->name; op++)
	{		
		if (!strcmp("hw", op->name)) 
		{
			mask = (mask & op->flags);
			goto SET_NET_PHYADDR_FOUND_ARG;
		}
	}
	close(sockfd);
	return SDK_ERROR;
	
SET_NET_PHYADDR_FOUND_ARG:
	a1op = Arg1Opt + 6;
	safe_strncpy(host, addr, sizeof(host));
	if (in_ether(host, &sa)) 
	{
		close(sockfd);
		return SDK_ERROR;
	}
	p = (char *)&sa;
	memcpy((char *)(&ifr) + a1op->ifr_offset, p, sizeof(struct sockaddr));
	if (ioctl(sockfd, a1op->selector, &ifr) < 0)
	{
		close(sockfd);
		return SDK_ERROR;
	}
	
	close(sockfd);
	return SDK_OK;
}

//============================================================================
// 函数名称: netcfg_active()
// 功能描述: 打开关闭网卡
// 其他说明:
//============================================================================
int netcfg_active(const char *name, const char *action) // name :  eth0 eth1 lo and so on
{											   			// action: down up
	const struct options *op;
	int 	sockfd;
	int 	selector;
	struct 	ifreq ifr;
	unsigned char mask;

	/* Create a channel to the NET kernel. */
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return SDK_ERROR;

	/* get interface name */
	safe_strncpy(ifr.ifr_name, name, IFNAMSIZ);
	mask = N_MASK;

	for (op = OptArray; op->name; op++)
	{
		if (!strcmp(action, op->name))
		{
			if (mask &= op->flags)
				goto IFCONFIG_UP_DOWN_FOUND_ARG;
		}
	}
	close(sockfd);
	return SDK_ERROR;

IFCONFIG_UP_DOWN_FOUND_ARG:
	if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0)
	{
		close(sockfd);
		return SDK_ERROR;
	}
	if (mask & SET_MASK)
		ifr.ifr_flags |= op->selector;
	else
		ifr.ifr_flags &= ~op->selector;	

	if (ioctl(sockfd, SIOCSIFFLAGS, &ifr) < 0)
	{
		close(sockfd);
		return SDK_ERROR;
	}

	close(sockfd);
	return SDK_OK;
}

//============================================================================
// 函数名称: netcfg_netlink_status()
// 功能描述: 获取有线网卡连接状态
// 其他说明:
//============================================================================

// if_name like "ath0", "eth0". Notice: call this function
// need root privilege.
// return value:
// -1 -- error , details can check errno
// 1 -- interface link up
// 0 -- interface link down.int get_netlink_status(const char * const if_name)
//只能获取到有线网口的物理连接状态
int netcfg_netlink_status(const char * const if_name)
{
    int skfd;
    struct ifreq ifr;
    struct ethtool_value edata;

    edata.cmd = ETHTOOL_GLINK;
    edata.data = 0;

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, if_name, sizeof(ifr.ifr_name) - 1);
    ifr.ifr_data = (char *) &edata;

    if (( skfd = socket( AF_INET, SOCK_DGRAM, 0 )) < 0)
        return -1;

    if(ioctl( skfd, SIOCETHTOOL, &ifr ) == -1)
    {
        close(skfd);
        return -1;
    }

    close(skfd);
        
    return edata.data;
}

int netcfg_get_mac_addr(const char *name, char *net_mac)
{
	struct ifreq ifr;
	int ret = 0;
	int fd;

	strcpy(ifr.ifr_name, name);
	ifr.ifr_addr.sa_family = AF_INET;

	fd = socket(PF_INET, SOCK_DGRAM, 0);
	if(fd < 0)
		ret = -1;

	ret = ioctl(fd, SIOCGIFHWADDR,(char *)&ifr, sizeof(ifr));
    if(ret == 0)
	{
        memcpy(net_mac, ifr.ifr_hwaddr.sa_data, 6);
	}
	close(fd);

	return ret;
}

int netcfg_get_ip_addr(const char *name, char *net_ip)
{
	struct ifreq ifr;
	int ret = 0;
	int fd;
	
	strcpy(ifr.ifr_name, name);
	ifr.ifr_addr.sa_family = AF_INET;
	
	fd = socket(PF_INET, SOCK_DGRAM, 0);
	if(fd < 0)
		ret = -1;
		
	if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) 
	{
		ret = -1;
	}
	else
		strcpy(net_ip,inet_ntoa(((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr));

	close(fd);

	return	ret;
}

int netcfg_get_mask_addr(const char *name, char *net_mask)
{
	struct ifreq ifr;
	int ret = 0;
	int fd;	
	
	strcpy(ifr.ifr_name, name);
	ifr.ifr_addr.sa_family = AF_INET;
	
	fd = socket(PF_INET, SOCK_DGRAM, 0);
	if(fd < 0)
		ret = -1;
		
	if (ioctl(fd, SIOCGIFNETMASK, &ifr) < 0) 
	{
		ret = -1;
	}
	
	strcpy(net_mask,inet_ntoa(((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr));

	close(fd);

	return	ret;
}

int netcfg_get_gw_addr(const char *name,char *gateway_addr)
{
	char buff[256];
	char ifname[32] = {0};
	int  nl = 0 ;
	struct in_addr gw;
	int flgs, ref, use, metric;
	unsigned  long d,g,m;
	FILE	*fp;
	
	if((fp = fopen("/proc/net/route", "r")) == NULL)
		return -1;	
		
	nl = 0 ;
	while( fgets(buff, sizeof(buff), fp) != NULL ) 
	{
		if(nl) 
		{
			int ifl = 0;
			if(sscanf(buff, "%s%lx%lx%X%d%d%d%lx",
				   ifname,&d, &g, &flgs, &ref, &use, &metric, &m)!=8) 
			{
				//continue;
				fclose(fp);
				return	-2;
			}

			ifl = 0;        /* parse flags */
			if(flgs&RTF_UP && (strcmp(name,ifname)== 0)) 
			{			
				gw.s_addr   = g;
					
				if(d==0)
				{
					strcpy(gateway_addr,inet_ntoa(gw));
					fclose(fp);
					return 0;
				}				
            }
		}
		nl++;
	}	
	
	if(fp)
	{
		fclose(fp);
		fp = NULL;
	}
	
	return	-1;
}

int netcfg_set_ip_addr(const char *name, const char *net_ip)
{	
	struct sockaddr	addr;
	struct ifreq ifr;
	char gateway_addr[32] = {0};
	int ret = 0;
	int fd;	
	
	((struct sockaddr_in *)&(addr))->sin_family = PF_INET;
	((struct sockaddr_in *)&(addr))->sin_addr.s_addr = inet_addr(net_ip);

	ifr.ifr_addr = addr;
	strcpy(ifr.ifr_name,name);

	fd = socket(PF_INET, SOCK_DGRAM, 0);
	if(fd < 0)
		ret = -1;

	netcfg_get_gw_addr(name,gateway_addr);		/* 在设置IP时会清除网络,所以先保存好网络设置 */

	if (ioctl(fd, SIOCSIFADDR, &ifr) != 0) 
	{
		ret = -1;
	}
	close(fd);
	netcfg_get_gw_addr(name, gateway_addr);		/*恢复网络的网关设置 */

	return	ret;
}

int netcfg_set_mask_addr(const char *name, const char *mask_ip)
{	
	struct sockaddr	addr;
	struct ifreq ifr;
	char gateway_addr[32];
	int ret = 0;
	int fd;	
	
	((struct sockaddr_in *)&(addr))->sin_family = PF_INET;
	((struct sockaddr_in *)&(addr))->sin_addr.s_addr = inet_addr(mask_ip);
	ifr.ifr_netmask = addr;
	strcpy(ifr.ifr_name,name);

	fd = socket(PF_INET, SOCK_DGRAM, 0);
	if(fd < 0)
		ret = -1;
		
	netcfg_get_gw_addr(name, gateway_addr); /* 在设置IP时会清除网络,所以先保存好网络设置 */

	if (ioctl(fd, SIOCSIFNETMASK, &ifr) != 0) 
	{
		ret = -1;
	}

	close(fd);

    netcfg_set_gw_addr(name, gateway_addr); /*恢复网络的网关设置 */

	return	ret;
}

int netcfg_del_gw_addr(const char *name, const char *gateway_addr)
{
	struct rtentry rt;
	unsigned long gw;
	int ret = 0;
	int fd;
	
	fd = socket(PF_INET, SOCK_DGRAM, 0);
	if(fd < 0)
		ret = -1;
	
	gw = inet_addr(gateway_addr);
	memset((char *) &rt, 0, sizeof(struct rtentry));

	rt.rt_flags = RTF_UP | RTF_GATEWAY ;
	
	((struct sockaddr_in *)&(rt.rt_dst))->sin_family = PF_INET;
	
	((struct sockaddr_in *)&(rt.rt_gateway))->sin_addr.s_addr = gw;	
	((struct sockaddr_in *)&(rt.rt_gateway))->sin_family = PF_INET;

	
	((struct sockaddr_in *)&(rt.rt_genmask))->sin_addr.s_addr = 0;
	((struct sockaddr_in *)&(rt.rt_genmask))->sin_family = PF_INET;
	
	rt.rt_dev = (char *)name;
		
	if (ioctl(fd, SIOCDELRT, &rt) < 0) 	
	{
		ret = -1;
	}

	close(fd);

	return	ret;
}

int netcfg_set_gw_addr(const char *name, const char *gateway_addr)
{
	char old_gateway_addr[32];
	struct rtentry rt;
	unsigned long gw;
	int fd;
	int ret = 0;
	
	netcfg_get_gw_addr(name, old_gateway_addr);
	netcfg_del_gw_addr(name,old_gateway_addr);
	
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		ret = -1;
	}	

	gw = inet_addr(gateway_addr);
	memset((char *) &rt, 0, sizeof(struct rtentry));

	((struct sockaddr_in *)&(rt.rt_dst))->sin_addr.s_addr = 0;

	rt.rt_flags = RTF_UP | RTF_GATEWAY ;
	//rt.rt_flags = 0x03;

	((struct sockaddr_in *)&(rt.rt_dst))->sin_family = PF_INET;
	
	((struct sockaddr_in *)&(rt.rt_gateway))->sin_addr.s_addr = gw;	
	((struct sockaddr_in *)&(rt.rt_gateway))->sin_family = PF_INET;
	

	((struct sockaddr_in *)&(rt.rt_genmask))->sin_addr.s_addr = 0;
	((struct sockaddr_in *)&(rt.rt_genmask))->sin_family = PF_INET;
	rt.rt_dev = (char *)name;

	if (ioctl(fd, SIOCADDRT, &rt) < 0)
	{
		ret = -1;
	}
	close(fd);
	
	return	ret;
}

int netcfg_set_dns(const char *dns1, const char *dns2)
{
	int fd;
	int ret;
	char data[STR_128_LEN] = {0};
	char *pStr = data;

	fd = open("/etc/resolv.conf", O_RDWR | O_TRUNC | O_CREAT);
	if (fd < 0)
		return SDK_ERROR;

	ret = sprintf(pStr, "nameserver %s\n", dns1);
	pStr += ret;
	sprintf(pStr, "nameserver %s\n", dns2);
		
	write(fd, data, strlen(data));
	close(fd);

	return SDK_OK;
}

int netcfg_get_dns(char *dns1, char *dns2)
{
	int fd;
	int ret;
	char data[STR_128_LEN] = {0};
	char *pStr = data;

	fd = open("/etc/resolv.conf", O_RDONLY);
	if (fd < 0)
		return SDK_ERROR;
		
	read(fd, data, sizeof(data));   
    sscanf(data, "nameserver %s\nnameserver %s\n", dns1, dns2);
	close(fd);

	return SDK_OK;
}


int netcfg_set_dns_v1(int index, const char *dns)
{
	int fd;
    char dns1[STR_16_LEN] = {0}, dns2[STR_16_LEN] = {0};
	char data[STR_128_LEN] = {0};

	fd = open("/etc/resolv.conf", O_RDWR | O_CREAT);
	if (fd < 0)
		return SDK_ERROR;
    read(fd, data, sizeof(data));
    sscanf(data, "nameserver %s\nnameserver %s\n", dns1, dns2);
    printf("old [%s:%s]\n", dns1, dns2);
    lseek(fd, 0, SEEK_SET);
    if (index == 0)
        sprintf(data, "nameserver %s\nnameserver %s\n", dns, dns2);
    else
        sprintf(data, "nameserver %s\nnameserver %s\n", dns1, dns);		
	write(fd, data, strlen(data));
	close(fd);

	return SDK_OK;
}

