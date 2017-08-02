#ifndef     __WIFI_H__
#define     __WIFI_H__

#include "common.h"

#ifdef		__cplusplus
extern "C" {
#endif

#ifdef   __cplusplus   
class CWifi
{
	public:
		static CWifi& getInstance()     /* ¼ÓËø */
		{
			static CWifi mInstance;

            return mInstance;
		}		
		CWifi();
		~CWifi();
		int init(void);
		void uninit(void);		
		int add_network(void);
		int del_network(int index);
		int set_network(int index,const char *name,const char *value);
		int set_network2wep(int index, const char *name,const char *value);
		int set_network3wep(int index, const char *name,const int value);
		int enable_network(int index);			
		int disable_network(int index);
		int scan(void);
		int scan_result(void);		
		int check_link(const char *ssid);


		/*add by He*/
		int StdinGetChar(int Timeout);
		int StdinGetString(char *buf,int Timeout);
		int get_line_from_buf(int index, char *line);
		int Ready_Linking(char **pStr);
		int check_link_Y_N(void);
		/*end add*/

		int select_network(int index); /* add by zhang */
		static int popen_cmd(char * cmd, char * type, char * ret, int retSize);
		static void wpa_cli_terminate(int sig, void *ctx);	

			
	private:
		struct wpa_ctrl *mWpaCtrl;		
		char mRecvBuf[STR_1024_LEN * 8];
        int mInit;
};
#endif

int wifi_init(void);
void wifi_uninit(void);
int wifi_add_network(void);

int wifi_del_network(int index);
int wifi_set_network(int index,const char *name,const char *value);
int wifi_set_network2wep(int index, const char *name, const char *value);
int wifi_set_network3wep(int index, const char *name, int value);


int wifi_enable_network(int index);			
int wifi_select_network(int index);
int wifi_is_wep(const char *ssid);//add by zhang
int wifi_disable_network(int index);
int wifi_scan(void);
int wifi_scan_result(void);
int wifi_check_link(const char *ssid);
int wifi_check_link_Y_N(void);
int StdinGetChar(int Timeout);
int StdinGetString(char *buf,int Timeout);


/*add by He*/
int wifi_Enter_Password(char **pStr,int net_id);
int wifi_Enter_Ssid(char **pStr,int *ap);
int wifi_Ready_Linking(char **pStr,int *ap);

/*end add*/




#ifdef		__cplusplus
}
#endif

#endif
