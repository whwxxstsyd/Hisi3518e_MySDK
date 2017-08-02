#ifndef		__CGI_SERVER__
#define		__CGI_SERVER__

#include "param.h"
#include "common.h"
#include "linux_list.h"
#include "hash.h"

#include <pthread.h>
#include <stdio.h>

#ifdef	__cplusplus
extern "C" {
#endif

#define     INT_NULL        (0)

typedef enum HASH_PARAM_TYPE_T
{
    HASH_TYPE_PARAM = 0,
    HASH_TYPE_CMD,
}HASH_PARAM_TYPE;

/* 将param和cmd两个hash表合成一个 */
typedef struct HASH_RECORD_INFO_T
{
	int type;   /* 0: Param, 1: Cmd */
	union
	{
		PARAM_INFO ParamInfo;		
		CMD_INFO CmdInfo;        
	};
}HASH_RECORD_INFO;

#ifdef __cplusplus		/* 兼容C语言 */
class CCgiServer
{
	public:
		static CCgiServer *getInstance()
		{
			static CCgiServer *obj = NULL;
			if (NULL == obj)
			{
				obj = new CCgiServer();	
                obj->Init();
			}
			return obj;
		}		
		CCgiServer();
		~CCgiServer();
		int Init(void);
		void Uninit(void); 
        int InitParamTable(void);
        int InitCmdTable(void);      
        int init_cmd_hash(const char *key_word, CMD_PROC_CB callback);  
        int init_param_hash(const char *key_word, char *param, int str_len, void *callback);
        int init_param_hash(const char *key_word, int type, void *param, int min_value, int max_value, void *callback);
		int set_param(const char *name, const char *value);
		int set_param(const char *name, int value);        
        int get_param(const char *name, char *value); 
        int cmd_proc(const char *key_word, const char *arg, char *out_str);  

        static void sync_param_cb(void *arg, void *pTimer);   
		
	private:
        CHash mHash;
        PARAM_INFO mParamTable[HASH_MAP_SIZE];
        CMD_INFO   mCmdTable[HASH_MAP_SIZE];
        FACTORY_PARAM *mFactoryParam;
        FLASH_PARAM *mFlashParam;
        RAM_PARAM *mRamParam;
        int mFlashSyncFlag;
        int mFactorySyncFlag;       
};
#endif

int set_param_str(const char *name, const char *value);
int set_param_int(const char *name, int value);
int get_param(const char *name, char *value);
int cmd_proc(const char *key_word, const char *arg, char *out_str); 
int param_server_init(void);
int config_net(const char *inf, NET_PARAM *pNetParam);
int cgi_set_timezone(const char *key_word, int value);
int set_sensitivity_power_on(void);
int func_config_power_on(void);

#ifdef	__cplusplus
}
#endif

#endif
