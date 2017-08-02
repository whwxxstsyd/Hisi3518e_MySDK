#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#include "rtsp_session.h"
#include "rtsp_h264.h"
#include "rtsp_mjpeg.h"
#include "rtsp_transfer.h"
#include "socket.h"
#include "debug.h"
#include "common.h"
#include "param.h"
#include "av_buffer.h"
#include "rtsp_struct.h"
#include "param_ext.h"
#include "base64.h"
#include "task.h"

int g_rtsp_port = DEFAULT_RTSP_PORT;
int g_auth_enable = 0;                  /* 使用用户验证 */

RTSP_SVR gRtspSvr;
pthread_mutex_t		 gRtspMutex;

int rtsp_get_video_media_info(int nCh, int *pPt, char *profileLevelId, char *sps_pps)
{
    AVC_HEADER *pAvcHeader = NULL;
	unsigned char szBase64sps[256] ={0}, szBase64pps[256] ={0};
    switch (nCh)
    {
        /* 主码流 */
        case 0:
        case 3:            
            pAvcHeader = get_avc_header();
            sprintf(profileLevelId, "%02x%02x%02x", 
                        pAvcHeader[0].sps.data[0], 
                        pAvcHeader[0].sps.data[1], 
                        pAvcHeader[0].sps.data[2]);
            to64frombits((unsigned char*)szBase64sps, pAvcHeader[0].sps.data, pAvcHeader[0].sps.len);
            to64frombits((unsigned char*)szBase64pps, pAvcHeader[0].pps.data, pAvcHeader[0].pps.len);
            sprintf(sps_pps, "%s,%s", szBase64sps, szBase64pps);
            break;
        /* 次码流 */
        case 1:            
        case 4:
            pAvcHeader = get_avc_header();        
            sprintf(profileLevelId, "%02x%02x%02x", 
                        pAvcHeader[1].sps.data[0], 
                        pAvcHeader[1].sps.data[1], 
                        pAvcHeader[1].sps.data[2]);
            to64frombits((unsigned char*)szBase64sps, pAvcHeader[1].sps.data, pAvcHeader[1].sps.len);
            to64frombits((unsigned char*)szBase64pps, pAvcHeader[1].pps.data, pAvcHeader[1].pps.len);          
            sprintf(sps_pps, "%s,%s", szBase64sps, szBase64pps);
            break;
        /* 第三码流 */
        case 2:         
            break;		
        default:
            return -1;
            break;
    }
	*pPt = RTP_PT_H264;
    LOG_INFO("profileLevelId: %s, sps_pps: %s\n", profileLevelId, sps_pps);

	return 0;
}

int rtsp_get_audio_media_info(int nCh, int *pPt, int *pBits, int *pSample, char *pInfo)
{    
    *pPt = RTP_PT_ALAW;
    switch(*pPt)
    {
        case RTP_PT_ADPCM:
            strcpy(pInfo, " PCM");
            break;
        case RTP_PT_G726:
            strcpy(pInfo, "G726-16");
            break;
        case RTP_PT_ULAW:
            strcpy(pInfo, " PCMU");
            break;
        case RTP_PT_ALAW:
            strcpy(pInfo, " PCMA");
            break;
        case RTP_PT_AAC:
            strcpy(pInfo, "MPEG4-GENERIC");
            break;
    }
    *pBits = 16;
    *pSample = 8000;	

    return 0;		
}

void rtsp_socket_build(int *pSockArray, int *pSockNum)
{
	RTSP_SESSION_S   *pSess = NULL, *pSessN = NULL;    
	
	*pSockNum     = 1;
	pSockArray[0] = gRtspSvr.nSvrSocket;
    list_for_each_entry_safe(pSess, pSessN, &(gRtspSvr.sessList), list)
    {
        if (pSess->nRtspSocket > 0)
        {
            pSockArray[*pSockNum] = pSess->nRtspSocket;
            *pSockNum += 1;
        }
    }
}

int rtsp_new_conn_proc(void)
{
	int  nLen = 0, nClientSock = 0, nRecvLen = 0;
	struct sockaddr_in adrrClient;
	char szBuff[RTSP_MAX_PROTOCOL_BUFFER] = {0};
	RTSP_SESSION_S *pSess = NULL;
        
	nLen = sizeof(adrrClient);
	//tcp以阻塞的方式接受数据
	if ((nClientSock = tcp_block_accept(gRtspSvr.nSvrSocket, 
				(struct sockaddr *)&adrrClient, &nLen)) < 0)
	{
		return -1 ;
	}

	set_sock_noblock(nClientSock);
	set_sock_attr(nClientSock, 1, 0,0,0,0);
	sock_set_linger(nClientSock);
	set_sock_nodelay(nClientSock);
	if (my_select(&nClientSock, 0x1, 0x1, 15000) != (0x10000 | nClientSock))
	{
		close_socket(&nClientSock);
		return -1 ;
	}

	if ((nRecvLen = tcp_noblock_recv(nClientSock, szBuff, sizeof(szBuff))) <= 0)
	{
		close_socket(&nClientSock);
		return -1 ;	
	}
	/*创建rtsp会话，并把它加入rtsp会话链表之中*/
	if ((pSess = rtsp_session_create(nClientSock)) == NULL)
	{/*创建新的rtsp session 失败*/
		close_socket(&nClientSock);
		return -1 ;	
	}	  
	
	//将接收到的数据存入session的BUFF中
	memcpy(pSess->szBuffRecv, szBuff, RTSP_MAX_PROTOCOL_BUFFER);
	pSess->nMsgSize = nRecvLen;
	/*处理rtsp会话消息*/
	if ((nRecvLen = rtsp_session_process(pSess)) != 0 ) 
	{                  
		rtsp_session_close(pSess);
		rtsp_list_client();	
		return -1 ;
	}	
	return 0;
}

void *rtsp_svr_proc(void *arg)
{
	char szPort[32] = {0};
	int  nSocket[32]= {0};
	int  nAddrLen = 0, nSocketNum = 0, nRet = 0, nRecvLen = 0;
	RTSP_SESSION_S   *pSess = NULL, *pSessN = NULL;
	
	sprintf(szPort, "%d", gRtspSvr.nSvrPort);
#if 0    /* 错误写法(导致getpeername和getsockname取不到地址) */
	if ((gRtspSvr.nSvrSocket = tcp_listen(NULL, szPort, &nAddrLen)) < 0)
#else
	if ((gRtspSvr.nSvrSocket = tcp_create_and_listen(gRtspSvr.nSvrPort)) < 0)
#endif
	{
		return (void *)NULL;
	}

	gRtspSvr.nSvrExits = 0;
	while (gRtspSvr.nSvrExits == 0)
	{	/* 检查rtsp会话状态 */
		check_rtsp_session_state();
		/* 重新获取socket fd */
		rtsp_socket_build(nSocket, &nSocketNum);
        nRet = my_select(nSocket, nSocketNum, 0x1, 15000);
        if (nRet < 0)
        {
			sleep(1);
			continue;
		}
		if (nRet == 0) /*time out*/
			continue;

		/*在这里开始进入RTSP的会话处理*/
		if (nRet == (gRtspSvr.nSvrSocket | 0x10000))                    /* 有新的连接  */
		{
			rtsp_new_conn_proc();
			continue;
		}
		
		LOG_DEBUG("Ready into list_for_each_entry_safe\n");	

        list_for_each_entry_safe(pSess, pSessN, &(gRtspSvr.sessList), list)
        {
            if (nRet == (pSess->nRtspSocket | 0x10000))                 /* 有数据接收 */
            {
                nRet = -1;
                memset(pSess->szBuffRecv, 0, RTSP_MAX_PROTOCOL_BUFFER);
                if ((nRecvLen = tcp_noblock_recv(pSess->nRtspSocket, 
						pSess->szBuffRecv, RTSP_MAX_PROTOCOL_BUFFER)) <= 0)
					goto __rtsp_session_error_handle;
                				/*处理rtsp会话消息*/
				pSess->nMsgSize = nRecvLen;
				nRet = rtsp_session_process(pSess); 
		__rtsp_session_error_handle:  
				if (nRet != 0) 
				{				   
					rtsp_session_close(pSess);
					rtsp_list_client(); 
				} 
				break;
            }
        }
    }
    list_for_each_entry_safe(pSess, pSessN, &(gRtspSvr.sessList), list)
        rtsp_session_close(pSess);
	close_socket(&gRtspSvr.nSvrSocket);
	gRtspSvr.thd = 0;
    
	return NULL;
}

int	rtsp_start_real(int nRtspPort, int bUseAuth)
{
	int nRet = 0;
	
	memset(&gRtspSvr, 0, sizeof(RTSP_SVR));
	gRtspSvr.nSvrPort = nRtspPort;
	gRtspSvr.nUseAuth = bUseAuth;
	
	pthread_mutex_init (&gRtspMutex, NULL);	
	INIT_LIST_HEAD(&(gRtspSvr.sessList));

	if ((nRet = CreateDetachedTask(&(gRtspSvr.thd), rtsp_svr_proc, NULL)) != 0)	/* for rtcp control */
		return -1;
 
	return 0;
}

/****************************  rtsp外部接口  **************************/
int rtsp_stop(void)
{
	int nTimes = 0;
	
	gRtspSvr.nSvrExits = 1;
	close_socket(&gRtspSvr.nSvrSocket);
	while ((gRtspSvr.thd != 0) && (nTimes++ < 500))     /* gRtspSvr.thd == 0: 线程退出了 */
	{
		usleep(10000);
		continue;
	}
	pthread_mutex_destroy(&gRtspMutex);
	
	return 0;
}

int rtsp_start(void)
{
	LOG_DEBUG("start rtsp sever[%d]\n", g_rtsp_port);
    return rtsp_start_real(g_rtsp_port, g_auth_enable);
}

int rtsp_set_port(int port)
{
    g_rtsp_port = port;

    return 0;
}

int rtsp_start_v1(int port)
{
	rtsp_set_port(port);
    
    return rtsp_start();
}

int rtsp_restart(void)
{
    rtsp_stop();
    rtsp_start();

    return 0;
}

