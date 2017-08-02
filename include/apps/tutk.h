#ifndef     __TUTK_H__
#define     __TUTK_H__

#include "common.h"
#include "linux_list.h"
#include "param.h"

#include "tutk/AVIOCTRLDEFs.h"
#include "tutk/AVAPIs.h"
#include "tutk/IOTCAPIs.h"
#include "tutk/AVFRAMEINFO.h"

#include <pthread.h>
#include <netinet/in.h>

#define     MAX_SESSION_COUNT       (20)
#define		MAX_IOCTRL_BUF_LEN		(1024)
#define 	IOTC_UDP_PORT        	(20000)
#define 	MAX_CLIENT_NUM	    	(10)
#define     MAX_PARAM_NUM           (5)

#define      PUSH_SERVER     "push.iotcplatform.com"

typedef enum TUTK_SERVER_VALUE_T
{
	AUDIO_IN = 0,
	AUDIO_OUT,
	PTZ,
	EVENT_LIST,
	PLAYBACK,
	WIFI,
	EVENT_SET,
	RECORD_SET,
	SDCARD,
	FLIP,
	ENV_MODE,
	MULTI_STREAM,
	AUDIO_OUT_FORMATE,
	QUALITY,
	DEVICE_INFO,
	TIMEZONE,
	IPC_DVR,
}TUTK_SERVER_VALUE;


#define		SERVER_TYPE_MASK				\
   ~( BIT(AUDIO_IN) | 				\
		BIT(AUDIO_OUT) | 				\
		BIT(WIFI) | 					\
		BIT(FLIP) | 				\
		BIT(QUALITY) | 				\
		BIT(DEVICE_INFO) | 					\
		BIT(ENV_MODE) | 				\
		BIT(EVENT_SET) |            \
		BIT(TIMEZONE) |             \
		BIT(PLAYBACK) |             \
		BIT(ENV_MODE) |             \
		BIT(SDCARD)     |           \
		BIT(RECORD_SET) |           \
		BIT(IPC_DVR)    |           \
		BIT(EVENT_LIST))    


typedef struct TUTK_CMD_T
{
	unsigned int type;
	char context[MAX_IOCTRL_BUF_LEN];
}TUTK_CMD;

typedef union TUTK_RECV_MSG_T
{
    SMsgAVIoctrlAVStream                AvStream;    
    SMsgAVIoctrlPtzCmd                  PtzCmd;
    SMsgAVIoctrlSetStreamCtrlReq        SetStreamCtrl;
    SMsgAVIoctrlSetVideoModeReq         SetVideoMode;  
    SMsgAVIoctrlSetEnvironmentReq       SetEnvironment;
    SMsgAVIoctrlSetPasswdReq            SetPasswd;
    SMsgAVIoctrlSetMotionDetectReq      SetMotionDetect;
    SMsgAVIoctrlSetWifiReq              SetWifi;
    SMsgAVIoctrlTimeZone                TimeZone;  
    SMsgAVIoctrlTimeZoneExt             TimeZoneExt;
    SMsgAVIoctrlListEventReq            ListEvent;
    SMsgAVIoctrlPlayRecord              PlayRecord;
    SMsgAVIoctrlFormatExtStorageReq     FormatStorage;
    SMsgAVIoctrlSetRecordReq            SetRecordReq;  
	SMsgAVIoctrlWifiIrLedCtrlInfo			WifiIrLedStatus;
	SMsgAVIoctrlAlarmRecPlan	AlarmRecPlan;
    SMsgAVIoctrlGetBrightReq            GetBright;
    SMsgAVIoctrlSetBrightReq            SetBright; 
    SMsgAVIoctrlGetContrastReq            GetContrast;
    SMsgAVIoctrlSetContrastReq            SetContrast;     
    SMsgAVIoctrlGetVideoStreamReq       GetVideoStream;
    SMsgAVIoctrlSetVideoStreamReq       SetVideoStream;    
}TUTK_RECV_MSG;

typedef union TUTK_SEND_MSG_T
{
    SMsgAVIoctrlGetSupportStreamResp    SupportStream;	
	SMsgAVIoctrlGetAudioOutFormatResp   AudioOutFormat;
	SMsgAVIoctrlDeviceInfoResp          DevInfo; 
	SMsgAVIoctrlGetStreamCtrlResq       GetStreamCtrl;    
    SMsgAVIoctrlSetStreamCtrlResp       SetStreamCtrl; 
    SMsgAVIoctrlSetVideoModeResp        SetVideoMode; 
    SMsgAVIoctrlGetVideoModeResp        GetVideoMode;    
    SMsgAVIoctrlGetEnvironmentResp      GetEnvironment;  
    SMsgAVIoctrlSetEnvironmentResp      SetEnvironment;
    SMsgAVIoctrlSetPasswdResp           SetPasswd;
    SMsgAVIoctrlGetMotionDetectResp     GetMotionDetect;
    SMsgAVIoctrlSetMotionDetectResp     SetMotionDetect; 
    SMsgAVIoctrlSetWifiResp             SetWifi;
    SMsgAVIoctrlGetWifiResp             GetWifi;
    SMsgAVIoctrlTimeZone                TimeZone;       /* ? response */   
    SMsgAVIoctrlTimeZoneExt             TimeZoneExt;    /* ? response */
    SMsgAVIoctrlListEventResp           ListEvent;
	SMsgAVIoctrlFormatExtStorageResp    FormatStorage;
    SMsgAVIoctrlGetRecordResq           GetRecord;
    SMsgAVIoctrlSetRecordResp           SetRecord;
	SMsgAVIoctrlWifiIrLedCtrlInfo		WifiIrLedStatus;
	SMsgAVIoctrlWifiIrLedCtrlResp	WifiIrLedStatusresp;
	SMsgAVIoctrlAlarmRecPlan	AlarmRecPlan;
	SMsgAVIoctrlAlarmRecPlanResp	AlarmRecPlanResp;
    SMsgAVIoctrlGetBrightResp            GetBright;
    SMsgAVIoctrlSetBrightResp            SetBright;  
    SMsgAVIoctrlGetContrastResp            GetContrast;
    SMsgAVIoctrlSetContrastResp            SetContrast;         
    SMsgAVIoctrlGetVideoStreamResp       GetVideoStream;
    SMsgAVIoctrlSetVideoStreamResp       SetVideoStream;     
	SMsgAVIoctrlUpgrade					 Upgrade;
    SMsgAVIoctrlSnapshotResp	Snapshot;
	SMsgAVIoctrlMainInterfaceConfigResp MInterfaceCfg;
	SMsgAVIoctrlOnOffConfigResp		onoffcfg;

}TUTK_SEND_MSG;

typedef struct PLAYBACK_SESSION_T
{
    int StateMathine;
    int StopFlag;
    int ch_id;
    int AvCh;
    char RecPath[STR_128_LEN];
    int diftime;
    int StepFps;	
}PLAYBACK_SESSION;

typedef struct TUTK_SESSION_T
{
    int session_id;
    int ctrl_cmd;
    unsigned int ch_id;
	unsigned int talk_ch;	
    int talk_client_ch;
	int WithAudio;	
    int RecvAudioProcRun;
    int StreamProcRun;
    int SessionProcRun;
    int count;          /* 引用计数 */
    pthread_t MediaProcId;
    pthread_t PlaybackId;    
	TUTK_RECV_MSG *pRecvMsg;  
    PLAYBACK_SESSION playback_session;
    void *pTutk;
	struct list_head list;
}TUTK_SESSION;

class CTutk
{
    public:
		static CTutk *getInstance()
		{
			static CTutk *obj = NULL;
			if (NULL == obj)
			{
				obj = new CTutk();		
			}
			return obj;
		}          
	    CTutk(void);
	    ~CTutk(void);

		int Init(char *uid, int audio_type);
		void Uninit(void);	
		int PlatformInit(void);
		void PlatformUninit(void);
		void SessionInit(void);
		void SessionUninit(void);
        int StartServer(char *uid);
        int StopSession(TUTK_SESSION *pSession);        
        int StopServer(void);  
        int SetAudio(int audio_type);
        void SetParamInf(SET_PARAM_STR_FUNC set_param_str_inf, 
                        SET_PARAM_INT_FUNC set_param_int_inf, 
                        GET_PARAM_FUNC get_param_inf, 
                        CMD_PROC_FUNC cmd_proc_inf);
        char *GetRegisterMessageString(CTutk *pTutk);
        int RegisterUidToPusher(CTutk *pTutk);
        char *GetPushMessageString(CTutk *pTutk, int event_type);
        int SendPushMessage(CTutk *pTutk, int event_type); 
        
        static int Platform_PTZControl(CTutk *pTutk, unsigned char cmd, unsigned char speed);  
        static int Platform_PlaybackCtrl(TUTK_SESSION *pSession);
		static void SessionDestroy(TUTK_SESSION *pSession);
		static int start_recv_audio(TUTK_SESSION *pSession);
		static int start_send_media(TUTK_SESSION *pSession);	
		static int AuthCallback(char *pViewAcc,char *pViewPwd);
		static int handle_ioctrl_cmd(TUTK_SESSION *pSession, unsigned int type, char *buf);
		static void *LoginProc(void *arg);
		static void *ListenProc(void *arg);
		static void *MediaProc(void *arg);        
		static void *SessionProc(void *arg);
		static void *RecvProc(void *arg);
		static void *PushProc(void *arg);   
        static void *thread_ConfigWifi(void *arg);
        static void *thread_ListWifi(void* arg);
        static void *thread_PlayBack(void* arg);
        static void *thread_ListEvent(void* arg);

    private:
		int mOnline;
        int mSessionCnt;
        int mAudioType; 
        int mQuality;  
        int mListenProcRun;
        int mPushProcRun;
        int mLoginProcRun;
        pthread_t mLoginId;
        pthread_t mListenId;
        pthread_t mPushId;
        char mUid[STR_64_LEN];
        char mPushSrv[STR_64_LEN];
        int mPushSrvPort;    
        struct sockaddr_in mPushServerAddr;
        VIDEO_PARAM_INFO mVideoParam[MAX_PARAM_NUM];                     
		struct list_head mSessionList;
        
    public:
        /* 模块外部接口 */
        CGI_INTERFACE mCgiFunc;
};

int tutk_server_start(char *uid);
int tutk_server_stop(void);
int tutk_set_audio(int audio_type);
void tutk_SetParamInf(SET_PARAM_STR_FUNC set_param_str_inf, 
	                SET_PARAM_INT_FUNC set_param_int_inf, 
	                GET_PARAM_FUNC get_param_inf, 
	                CMD_PROC_FUNC cmd_proc_inf);
typedef struct UpgradeReqT
{
		unsigned char result;
}UpgradeReqS;

#endif
