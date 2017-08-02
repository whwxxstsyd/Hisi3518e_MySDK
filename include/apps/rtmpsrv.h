#ifndef     __RTMPSRV_H__
#define     __RTMPSRV_H__

#include "param.h"
#include "linux_list.h"

#include "librtmp/rtmp_sys.h"
#include "librtmp/log.h" 

typedef enum RTMP_CODEC_ID_T
{
	FLV_CODECID_H264 = 7,
	FLV_CODECID_AAC = 160,
	FLV_CODECID_ADPCM = 161,
}RTMP_CODEC_ID;

typedef enum RTMP_PACKET_TYPE_T
{
	TYPE_CHUNK_SIZE			= 0x01,		/* 修改chunk size */
	TYPE_UNKNOWN1 			= 0x02,
	TYPE_BTYES_READ 		= 0x03,
	TYPE_PING 				= 0x04,
	TYPE_SERVER_BW 			= 0x05,
	TYPE_CLIENT_BW			= 0x06,
	TYPE_UNKNOWN2			= 0x07,
	TYPE_AUDIO_DATA			= 0x08,
	TYPE_VIDEO_DATA			= 0x09,
	TYPE_UNKNOWN3			= 0x0A,
	TYPE_FLEX_STREAM_SEND	= 0X0F,
	TYPE_FLEX_SHARED_OBJECT = 0x10,
	TYPE_FLEX_MESSAGE		= 0x11,
	TYPE_NOTIFY				= 0x12,
	TYPE_SHARE_OBJECT		= 0x13,
	TYPE_INVOKE 			= 0x14,
	TYPE_STREAM_DATA		= 0x16,
}RTMP_PACKET_TYPE;

typedef enum RTMP_CHANNEL_ID_T
{
	CH_ID_PING = 0x02,			/* ping和ByteRead通道 */
	CH_ID_INVOKE = 0x03,		/* Invoke通道 */
	CH_ID_MEDIA_DATA = 0x04,	/* 音视频数据 */
}RTMP_CHANNEL_ID;

typedef struct AAC_HEADER_T
{
	unsigned short audioObjectType: 5;
	unsigned short samplingFrequencyIndex: 4;
	unsigned short channelConfiguration: 4;
	unsigned short frameLengthFlag: 1;
	unsigned short dependsOnCoreCoder: 1;
	unsigned short extensionFlag: 1;
}AAC_HEADER;

typedef struct RTMP_METADATA_T
{
	/* video must be h264 type */
    int nVideoResolution;
	unsigned int	nFrameRate;		
	unsigned int	nVideoDataRate;	    /* bps */
	unsigned int	nSpsLen;
	unsigned char	Sps[64];
	unsigned int	nPpsLen;
	unsigned char	Pps[64];
	/* audio must be aac type */
	int	bHasAudio;
    unsigned int  nAudioType;
    float  nAudioDataRate;
	unsigned int	nAudioSampleRate;
	unsigned int	nAudioBitWidth;
	unsigned int	nAudioChannels;
	unsigned char	pAudioSpecCfg[64];
	unsigned int	nAudioSpecCfgLen;       /* AAC header len:2 */
}RTMP_METADATA;

/* 从STREAMING_SERVER改过来 */
typedef struct RTMP_SESSION_T
{
	int socket;
	int streamID;
	int arglen;
	int argc;
	unsigned int filetime;	        /* time of last download we started */
	AVal filename;	            /* name of last download */
	char *connect;
    RTMP rtmp;
    int *pSessionCnt;         /* 指向RtmpServer的mSessionCnt */
    RTMP_METADATA *pMetaData;   /* 指向RtmpServer的mMetaData */
    struct list_head list;
}RTMP_SESSION;

class CRtmpServer
{
public:
	static CRtmpServer *getInstance()
	{
		static CRtmpServer *obj = NULL;
		if (NULL == obj)
		{
			obj = new CRtmpServer();		
		}
		return obj;
	}

    CRtmpServer(void);
    ~CRtmpServer(void);      
    int start_rtmpserver(void);
    int stop_rtmpserver(void);
    int restart_rtmpserver(void);
    int start_rtmppusher(void);
    int stop_rtmppusher(void);
    int restart_rtmppusher(void);
    int rtmp_set_port(int port);
    int rtmp_set_pushURL(const char *url);
    int rtmp_set_mediainfo(RTMP_METADATA *pMetaData);    

    static void *SendMediaProc(void *arg);
    static int ServeInvoke(RTMP_SESSION *pSession, RTMPPacket *packet, unsigned int offset);
    static int ServePacket(RTMP_SESSION *pSession, RTMPPacket *packet);
    static void *rtmp_session_proc(void *arg);		
    static void *rtmp_server_proc(void *arg);
    static void *rtmp_push_proc(void *arg);
    
private: 
    RTMP_METADATA mMetaData;
    RTMP *mRtmpPusher;
    char mPushURL[STR_1024_LEN];
    int mPort;
    int mRtmpListenFd;
    int mRtmpPushFd;   
    int mSessionCnt;
    int mPushStream;
    struct list_head mSessionList;    
};

int start_rtmpserver(void);
int stop_rtmpserver(void);
int restart_rtmpserver(void);	
int start_rtmppusher(void);	
int stop_rtmppusher(void);	
int restart_rtmppusher(void);	
int rtmp_set_port(int port);
int rtmp_set_pushURL(const char *url);
int rtmp_set_mediainfo(RTMP_METADATA *pMetaData);

#endif
