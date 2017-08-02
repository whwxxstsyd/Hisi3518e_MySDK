#ifndef     __NTP_H__
#define     __NTP_H__

#ifdef		__cplusplus
extern	"C"		{
#endif

#define     NTP_MAXREQUEST_LEN      (32)
#define     NTP_SERVER_PORT         (123)
#define     UTC_TIME_DIFF           (2208988800U)  
#define     NTPMODETYPE             (3)          /* NTP mode type client */


typedef enum NTP_VER_T 
{
  NTP_V1 = 010,                   /* < 00 001 000 binary = v1 > */
  NTP_V2 = 020,                   /* < 00 010 000 binary = v2 > */
  NTP_V3 = 030,                   /* < 00 011 000 binary = v3 > */
}NTP_VER;

int ajust_time(const char *hostname, 		/* ntp主机地址 */
					int timezone, 		/* 本地时区 */
					int save_time);		/* 夏令时,调时时长 (0:  夏令时关闭)*/


#ifdef		__cplusplus
}
#endif

#endif
