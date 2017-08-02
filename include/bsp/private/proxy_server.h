#ifndef     __PROXY_SERVER_H__
#define     __PROXY_SERVER_H__

#ifdef	__cplusplus
extern "C" { 
#endif

#include "hash.h"
#include "common.h"

#include <stdio.h>
#include <stdlib.h>

class CProxyServer
{
    public:
		static CProxyServer *getInstance()
		{
			static CProxyServer *obj = NULL;
			if (NULL == obj)
			{
				obj = new CProxyServer();		
			}
			return obj;
		}        
        CProxyServer();
        ~CProxyServer();  
        int Init(void);
        void Uninit(void);
        int init_param_hash(const char *key_word, int type, void *callback);
        int cmd_proc(const char *key_word, char *arg, char *out_str);       
        
    private:
        CHash mHash;       
};

int proxy_server_init(void);
int proxy_cmd_proc(const char *key_word, char *arg, char *out_str);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif

