#ifndef		__NET_H__
#define		__NET_H__

#ifdef      __cplusplus
extern  "C"     {
#endif

#define A_MAP_TYPE       0x0C
#define A_ARG_REQ        0x10	/* Set if an arg is required. */
#define A_NETMASK        0x20	/* Set if netmask (check for multiple sets). */
#define A_SET_AFTER      0x40	/* Set a flag at the end. */
#define A_COLON_CHK      0x80	/* Is this needed?  See below. */

#define N_CLR            0x01
#define M_CLR            0x02
#define N_SET            0x04
#define M_SET            0x08
#define N_ARG            0x10
#define M_ARG            0x20

#define A_CAST_TYPE      0x03
#define A_MAP_TYPE       0x0C
#define A_ARG_REQ        0x10	/* Set if an arg is required. */
#define A_NETMASK        0x20	/* Set if netmask (check for multiple sets). */
#define A_SET_AFTER      0x40	/* Set a flag at the end. */
#define A_COLON_CHK      0x80	/* Is this needed?  See below. */

#define A_MAP_ULONG      0x04	/* memstart */
#define A_MAP_USHORT     0x08	/* io_addr */
#define A_MAP_UCHAR      0x0C	/* irq */

#define A_CAST_CHAR_PTR  0x01
#define A_CAST_RESOLVE   0x01
#define A_CAST_HOST_COPY 0x02
#define A_CAST_HOST_COPY_IN_ETHER    A_CAST_HOST_COPY
#define A_CAST_HOST_COPY_RESOLVE     (A_CAST_HOST_COPY | A_CAST_RESOLVE)

#define M_MASK           (M_CLR | M_SET | M_ARG)
#define N_MASK           (N_CLR | N_SET | N_ARG)
#define SET_MASK         (N_SET | M_SET)
#define CLR_MASK         (N_CLR | M_CLR)
#define SET_CLR_MASK     (SET_MASK | CLR_MASK)
#define ARG_MASK         (M_ARG | N_ARG)

#define ARG_METRIC       (A_ARG_REQ /*| A_CAST_INT*/)
#define ARG_MTU          (A_ARG_REQ /*| A_CAST_INT*/)
#define ARG_TXQUEUELEN   (A_ARG_REQ /*| A_CAST_INT*/)
#define ARG_MEM_START    (A_ARG_REQ | A_MAP_ULONG)
#define ARG_IO_ADDR      (A_ARG_REQ | A_MAP_ULONG)
#define ARG_IRQ          (A_ARG_REQ | A_MAP_UCHAR)
#define ARG_DSTADDR      (A_ARG_REQ | A_CAST_HOST_COPY_RESOLVE)
#define ARG_NETMASK      (A_ARG_REQ | A_CAST_HOST_COPY_RESOLVE | A_NETMASK)
#define ARG_BROADCAST    (A_ARG_REQ | A_CAST_HOST_COPY_RESOLVE | A_SET_AFTER)
#define ARG_HW           (A_ARG_REQ | A_CAST_HOST_COPY_IN_ETHER)
#define ARG_POINTOPOINT  (A_CAST_HOST_COPY_RESOLVE | A_SET_AFTER)
#define ARG_KEEPALIVE    (A_ARG_REQ | A_CAST_CHAR_PTR)
#define ARG_OUTFILL      (A_ARG_REQ | A_CAST_CHAR_PTR)
#define ARG_HOSTNAME     (A_CAST_HOST_COPY_RESOLVE | A_SET_AFTER | A_COLON_CHK)

#define ifreq_offsetof(x)  offsetof(struct ifreq, x)
#define ETHTOOL_GLINK       0x0000000a          /* Get link status (ethtool_value) */  

//拷贝自:busybox(ifconfig.c)
//增加设置网卡物理地址及ifconfig_up_down需要
struct arg1opt 
{
	const char *name;
	unsigned short selector;
	unsigned short ifr_offset;
};

struct options 
{
	const char *name;
	const unsigned char  flags;
	const unsigned char  arg_flags;
	const unsigned short selector;
};

struct ethtool_value 
{  
    unsigned int   cmd;  
    unsigned int   data;   
}; 

/* 设置mac地址 [name: eth0/wlan0    addr: 00:12:34:56:78:84] */
int netcfg_set_mac_addr(const char *name, char *addr);
/* 激活网卡 [name: eth0/wlan0   action: down/up] */
int netcfg_active(const char *name, const char *action);
/* 查看网卡连接状态 [if_name: ath0/eth0] return: [-1-->error 0-->link_down 1-->link_up] */
int netcfg_netlink_status(const char * const if_name);
/* 获取ip [name: eth0/wlan0     net_ip: 192.168.0.10] */
int netcfg_get_ip_addr(const char *name, char *net_ip);
/* 获取mac地址 */
int netcfg_get_mac_addr(const char *name, char *net_mac);
/* 获取子网掩码 */
int netcfg_get_mask_addr(const char *name, char *net_mask);
/* 获取网关地址 */
int netcfg_get_gw_addr(const char *name,char *gateway_addr);


/* 设置ip地址 */
int netcfg_set_ip_addr(const char *name, const char *net_ip);
/* 设置子网掩码 */
int netcfg_set_mask_addr(const char *name, const char *mask_ip);
/* 删除网关 */
int netcfg_del_gw_addr(const char *name, const char *gateway_addr);
/* 设置网关 */
int netcfg_set_gw_addr(const char *name, const char *gateway_addr);

int netcfg_set_dns(const char *dns1, const char *dns2);

int netcfg_get_dns(char *dns1, char *dns2);

int netcfg_set_dns_v1(int index, const char *dns);

#ifdef      __cplusplus
}
#endif

#endif
