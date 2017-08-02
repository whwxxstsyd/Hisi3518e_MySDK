#ifndef _RTSP_G7XX_H_
#define _RTSP_G7XX_H_

#ifdef __cplusplus
extern "C" {
#endif 

#include "rtsp_struct.h"

int rtsp_g7xx_audio_send(RTSP_SESSION_S* pSess, const char *szBuff, int nFrameLen, 
						 unsigned long u32TimeStamp, unsigned long u32SampleRate);

#ifdef __cplusplus
}
#endif 


#endif
