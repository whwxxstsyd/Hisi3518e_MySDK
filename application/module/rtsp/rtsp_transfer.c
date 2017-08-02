#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>

#include "rtsp_transfer.h"
#include "rtsp_global.h"
#include "socket.h"
#include "rtsp_session.h"
#include "rtsp_h264.h"
#include "rtsp_mjpeg.h"
#include "rtsp_g7xx.h"
#include "rtsp_aac.h"

#include "debug.h"
#include "common.h"
#include "av_buffer.h"
#include "normal.h"
#include "task.h"
#include "param_base.h"

int rtsp_send_reply(RTSP_SESSION_S* pSess, int err, char *addon , int simple)
{
    char* pTmp = pSess->szBuffSend;

    if (simple == 1)
    {
        pTmp += rtsp_make_resp_header(pSess, err);
    }
    if (addon != NULL)
    {
        pTmp += sprintf(pTmp, "%s", addon );
    }

    if (simple)
        strcat(pSess->szBuffSend, RTSP_LRLF);
    pthread_mutex_lock(&pSess->mutex);
    if (tcp_noblock_send(pSess->nRtspSocket, pSess->szBuffSend, strlen(pSess->szBuffSend)) 
        != strlen(pSess->szBuffSend))
    {	
        pthread_mutex_unlock(&pSess->mutex);
	    return -1;
    }
    pthread_mutex_unlock(&pSess->mutex);
    return 0;
}

const int StreamCh2FrameType[] = 
{
    FRAME_TYPE_IDR | FRAME_TYPE_I | FRAME_TYPE_P | FRAME_TYPE_B,                       
    FRAME_TYPE_SUB1_IDR | FRAME_TYPE_SUB1_I | FRAME_TYPE_SUB1_P | FRAME_TYPE_SUB1_B,    
    FRAME_TYPE_SUB2_IDR | FRAME_TYPE_SUB2_I | FRAME_TYPE_SUB2_P | FRAME_TYPE_SUB2_B,   
    FRAME_TYPE_IDR | FRAME_TYPE_I | FRAME_TYPE_P | FRAME_TYPE_B | FRAME_TYPE_NEAR_G711,                       
    FRAME_TYPE_SUB1_IDR | FRAME_TYPE_SUB1_I | FRAME_TYPE_SUB1_P | FRAME_TYPE_SUB1_B | FRAME_TYPE_NEAR_G711,    
    FRAME_TYPE_SUB2_IDR | FRAME_TYPE_SUB2_I | FRAME_TYPE_SUB2_P | FRAME_TYPE_SUB2_B | FRAME_TYPE_NEAR_G711,      
};


void rtsp_av_transmit_proc(void *arg)			
{
	int ret;
	int my_real_fps[2] = {0};
	RTSP_SESSION_S *pSess = (RTSP_SESSION_S *)arg;  
    int pre_frame_num = 0;
    int  start_flag = 1, frame_type = 0;         /* time stamp 30ms */
	REAL_FRAME real_frame = {0};        /* 一定要清零(保证magic为0) */  
    /******add by HeTianqi******/
    unsigned long u32TimeStamp = 0;
	char *pData;
	my_real_fps[0] = my_real_fps[1] = 15;
	
	//FILE *pFile = fopen("nihao123.h264", "wb");
	/*********end add*********/
	LOG_INFO("fps: %d_%d\n", my_real_fps[0], my_real_fps[1]);
    if ((pSess->nChId < 0) || (pSess->nChId > ARRAY_SIZE(StreamCh2FrameType)))
    {
        LOG_ERROR("bad channel id\n");
        goto ERROR_OUT;
    }
	LOG_INFO("ChId: %d\n", pSess->nChId);
    frame_type = StreamCh2FrameType[pSess->nChId];
	while (pSess->eState != RTSP_STATE_STOP)
	{
		if (av_read_frame_ext(frame_type, &real_frame, 1) < 0)
		{
            LOG_ERROR("video loss\n");
            goto ERROR_OUT;
		}
		
//		fwrite(real_frame.data + sizeof(FRAME_HEAD),real_frame.len - sizeof(FRAME_HEAD),1,pFile);


		if (check_frame(&real_frame) != SDK_OK)
		{
			LOG_WARN("========  bad frame  =======\n");
          	av_clear_sem();
			continue;
		}
		//ADD :测试发送的数据是否正确
		//fwrite(real_frame.data + sizeof(FRAME_HEAD),real_frame.len - sizeof(FRAME_HEAD),1,pFile);
		//end ADD

        switch (real_frame.type)
        {
            case FRAME_TYPE_NEAR_G726:  
            case FRAME_TYPE_NEAR_G711:
                u32TimeStamp = pSess->stRtpSender.u32LastSndTs[1] + (AUDIO_RAW_LEN / 2);
                ret = rtsp_g7xx_audio_send(pSess, real_frame.data + HISI_G726_HEAD, real_frame.len - HISI_G726_HEAD, u32TimeStamp, 8000); 
				if (ret < 0)
					goto ERROR_OUT;                 
                break;				
			default:		/* for h264 stream send */       
				//计算时间戳
                if (frame_type & FRAME_TYPE_IDR)
                {
				  u32TimeStamp = pSess->stRtpSender.u32LastSndTs[0] + (90000 / my_real_fps[0]);
				}
                    
                ret = rtsp_h264_video_send(pSess, real_frame.data, real_frame.len, u32TimeStamp);		/* u32TimeStamp 没有使用,置0 */	
				if (ret < 0)
					goto ERROR_OUT;
                break;				
        }
	}
	

ERROR_OUT:
	pSess->sessThd = 0;
	LOG_ERROR("video Down!!!!!!!!!!!\n");
	pSess->eTransType = RTSP_STATE_STOP;
}

int create_rtsp_av_transmit(RTSP_SESSION_S *pSess)
{
    CreateDetachedTask(&(pSess->sessThd), (void *)rtsp_av_transmit_proc, pSess);
    
	return 0;
}

int send_rtp_data(RTSP_SESSION_S* pSess, char *szRtpData, int nDataLen, int bAudio)
{
	int len = 0;

	if (pSess->eTransType == RTP_TRANSPORT_TCP)
	{
		pthread_mutex_lock(&pSess->mutex);
		if (tcp_noblock_send(pSess->stRtpSender.nRtpTcpSock, szRtpData, nDataLen) != nDataLen)	
		{
			LOG_ERROR("tcp_noblock_send send fail\n");
    		pthread_mutex_unlock(&pSess->mutex);
			return -1;
		}
		pthread_mutex_unlock(&pSess->mutex);
	}
	else
	{
 		if ((len = udp_send(pSess->stRtpSender.nRtpSock[bAudio], szRtpData, nDataLen,
			(struct sockaddr *)&(pSess->stRtpSender.addrRemote[bAudio]))) != nDataLen)	
 		{
 			LOG_ERROR("udp_send fail[%d:%d]\n", len, nDataLen);
			return -1;		
 		}
	}
    
	return nDataLen;
}

