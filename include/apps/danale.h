#ifndef     __DANALE_H__
#define     __DANALE_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>        /* true/false define */
#include <inttypes.h>
#include <signal.h>
#include <setjmp.h>

#include "param.h"
#include "common.h"

#include "libdanavideo/danavideo.h"
#include "libdanavideo/danavideo_cmd.h"

#define     BYTE        unsigned char
#define     DWORD       int

typedef struct USER_DATA_T 
{
    pdana_video_conn_t *danavideoconn;

    volatile bool run_media;
    volatile bool exit_media;
    pthread_t thread_media;

    volatile bool run_talkback;
    volatile bool exit_talback;
    pthread_t thread_talkback;

    volatile bool run_audio_media;
    volatile bool exit_audio_media;
    pthread_t thread_audio_media;

    uint32_t chan_no;
    char appname[32];
    void *pDanale;
    int quality;
    int64_t record_timeswap;
    int record_state;
    char record_file[64];
}USER_DATA_S;


class CDanale
{
    public:
		static CDanale *getInstance()
		{
			static CDanale *obj = NULL;
			if (NULL == obj)
			{
				obj = new CDanale();		
			}
			return obj;
		}          
        CDanale();
        ~CDanale();        
        int Init(void);
        void Uninit(void);
        void SetParamInf(SET_PARAM_STR_FUNC set_param_str_inf, 
                        SET_PARAM_INT_FUNC set_param_int_inf, 
                        GET_PARAM_FUNC get_param_inf, 
                        CMD_PROC_FUNC cmd_proc_inf);  
        int PushMsg(void);
        
    public:
        int mRunning;
        int mLibDanavideoInited;
        int mLibDanavideoStarted;
        int mDanaAirLinkInited;
        int mDanaAirLinkStarted;
        int mPushProcRun;
        int mAlarmFlag;
        /* 模块外部接口 */
        CGI_INTERFACE mCgiFunc; 
        VIDEO_PARAM_INFO mVideoParam[4];
};

int danale_init(void);
void danale_uninit(void);
void danale_SetParamInf(SET_PARAM_STR_FUNC set_param_str_inf, 
	                SET_PARAM_INT_FUNC set_param_int_inf, 
	                GET_PARAM_FUNC get_param_inf, 
	                CMD_PROC_FUNC cmd_proc_inf);

#endif
