#ifndef		__TASK_VIDEO_H__
#define		__TASK_VIDEO_H__

#include "task_base.h"
#include "common.h"
#include "param.h"
#include "sample_comm.h"
#include "linux_list.h"


#define		CNT_FPS			(0)

typedef struct IDR_INFO_T
{
	char *pps;
	int pps_len;
	char *sps;
	int sps_len;
	char *idr;
	int idr_len;
}IDR_INFO;

typedef enum ACTION_MASK_T
{
    EMAIL_BIT = (1 << 0),
    FTP_BIT = (1 << 1),
    SD_BIT = (1 << 2),        
}ACTION_MASK;

typedef struct FILE_INFO_T
{
    char **file_list;
    int cnt;
}FILE_INFO;

typedef struct EVENT_CONNECT_SESSION_T
{
	int sockfd;
	struct list_head list;
}EVENT_CONNECT_SESSION;

typedef struct NIHAO
{
	int Hi;
	int Hello;
}nihao;


class CTaskVideo: public CTaskBase
{
    public:
		static CTaskVideo *getInstance()
		{
			static CTaskVideo *obj = NULL;
			if (NULL == obj)
			{
				obj = new CTaskVideo();		
			}
			return obj;
		}

        CTaskVideo();
        ~CTaskVideo();
		int Init(void);
		void Uninit(void); 
#if  CNT_FPS       
		void Process(void);
#endif
		
	private:
		VIDEO_PARAM *mVideoParam;	
    	RAM_PARAM *mpRamParam;           
    	pthread_t mEventId;        

    public:
		//nihao		*My_nihao;
        AVC_HEADER  mpAvcHeader[2];
        ALARM_PARAM *mpAlarmParam;     
        int mGetRecFlag;   
        int mGetPicFlag;        
        int mPlayAudioFlag; 
#if  CNT_FPS        
        int mMainCnt;
        int mSub1Cnt;
        int mSub2Cnt;
        int mSub3Cnt;         
#endif        
        struct list_head mEventSessionList;           
};

int video_task_create(void);
void video_task_destory(void);

#endif
