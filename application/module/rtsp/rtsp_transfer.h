#ifndef _RTSP_TRANSFER_H_
#define _RTSP_TRANSFER_H_

#include "rtsp_struct.h"

#ifdef __cplusplus
extern "C" {
#endif 

typedef int (*SEND_VIDEO_FUN)(RTSP_SESSION_S* pSess, const char *szBuff, int nFrameLen,
								unsigned long u32TimeStamp);

typedef int (*SEND_AUDIO_FUN)(RTSP_SESSION_S* pSess, const char *szBuff, int nFrameLen,
								unsigned long u32TimeStamp, unsigned long u32SampleRate);

int rtsp_send_reply(RTSP_SESSION_S* pSess, int err, char *addon , int simple);

int create_rtsp_av_transmit(RTSP_SESSION_S *pSess);

int send_rtp_data(RTSP_SESSION_S* pSess, char *szRtpData, int nDataLen, int bAudio);


#ifdef __cplusplus
}
#endif 

#endif

