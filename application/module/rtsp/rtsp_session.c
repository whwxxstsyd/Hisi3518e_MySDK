#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/socket.h>

#include "rtsp_global.h"
#include "rtsp_session.h"
#include "rtsp_parse.h"
#include "rtsp_transfer.h"
#include "rtsp_rtp.h"
#include "socket.h"
#include "character_encode.h"
#include "rtsp_struct.h"

#define		DEBUG_LEVEL	LEVEL_WARN
#include "debug.h"

#define		PLATFORM_ONVIF		1		/* add by liwei */

void random_id(char *dest, int len)
{
	int i;

	for(i = 0; i < len; i++)
        dest[i] = (char)((random() % 10) + '0');
    
	dest[len] = 0;
}

/* camera_index + stream_index*/
int rtsp_get_chn(const char *pChn, int *camera_ch, int *stream_ch)
{
    int camera_index, stream_index;
    
    *camera_ch = -1;
    *stream_ch = -1;  
    
    if (pChn == NULL)
        return -1;
    /* 格式 rtsp://192.168.0.251:554/11 */
    if ((strlen(pChn) != 2) || !(isdigit(pChn[0])) || !(isdigit(pChn[1])))
        return -1;
    camera_index = pChn[0] - '0';
    stream_index = pChn[1] - '0';
    if (camera_index > RTSP_MAX_CAMERA)
        return -1;
    if (stream_index > RTSP_MAX_STREAM)
        return -1;
    *camera_ch = camera_index;
    *stream_ch = stream_index;

    return 0;
}

RTSP_SESSION_S *rtsp_session_create(int nSock)
{
    RTSP_SESSION_S *pSess = NULL;
	 static int nBasePort  = 19840;
	 int        nRtpSock[4]= {0};
	 int        nTryTimes  = 0;
	 int        nIndex     = 0;
	 
    if (0 == nSock)
    {
        LOG_ERROR("Cann't create new rtsp session.\n");
        return NULL;
    }
    
    if (NULL == (pSess = (RTSP_SESSION_S *)calloc(1, sizeof(RTSP_SESSION_S ))))
    {
        LOG_ERROR("Create session failed");
        return NULL;
    }

	pthread_mutex_init(&pSess->mutex, NULL);
    pSess->nRtspSocket = nSock;
    pSess->nChId       = -1;
	pSess->eState      = RTSP_STATE_INIT;
  
	check_rtsp_sock(nSock, pSess);

	 while (nTryTimes++ < 10)
	 {
	 	for (nIndex = 0; nIndex < 4; ++nIndex)
	 	{
			if ((nRtpSock[nIndex] = create_sock(SOCK_DGRAM)) < 0 ||
				bind_sock(nRtpSock[nIndex], INADDR_ANY, nBasePort+nIndex) < 0)
			{
				close_socket(&nRtpSock[nIndex]);
				close_socket(&pSess->stRtpSender.nRtpSock[0]);
			    close_socket(&pSess->stRtpSender.nRtcpSock[0]);
				close_socket(&pSess->stRtpSender.nRtpSock[1]);
				close_socket(&pSess->stRtpSender.nRtcpSock[1]);				
				break;
			}
			else
			{
				if (nIndex % 2 == 0)
				{
					pSess->stRtpSender.nRtpPort[nIndex/2]  = nBasePort+nIndex;
					pSess->stRtpSender.nRtpSock[nIndex/2]  = nRtpSock[nIndex];
				}
				else
				{
					pSess->stRtpSender.nRtcpPort[nIndex/2]  = nBasePort+nIndex;
					pSess->stRtpSender.nRtcpSock[nIndex/2]  = nRtpSock[nIndex];
				}
			}
	 	} 
		nBasePort += 4; 
		break;
	 }	
	 
	 
	 pSess->stRtpSender.nAudioSsrc   = RTP_DEFAULT_SSRC + nBasePort - 19840;
	 pSess->stRtpSender.nVideoSsrc   = RTP_DEFAULT_SSRC + 1024 + nBasePort - 19840;

	 random_id(pSess->szId, 8);
#if 0     
     LOG_INFO("create new rtsp session. remote host: %s:%d, session id: %s, socket:%d\n",
                pSess->szRemoteIP, pSess->u16RemotePort, pSess->szId, nSock);
#endif
     rtsp_session_lists_add(pSess);

    return pSess;
}

void rtsp_session_lists_add(RTSP_SESSION_S *pSess)
{
    if (NULL != pSess)
    {
		/*进入临界区，预防其他线程正在用rtsp session的的时候，增加rtsp session*/
		pthread_mutex_lock (&gRtspMutex);
        list_add_tail(&pSess->list, &gRtspSvr.sessList);
		pthread_mutex_unlock (&gRtspMutex);
    }
}

void rtsp_session_lists_remove(RTSP_SESSION_S* pSess)
{
    if (NULL != pSess)
    {
		/*进入临界区，预防其他线程正在用rtsp session的的时候，释放rtsp session*/
		pthread_mutex_lock (&gRtspMutex);
		/*关闭socket套接字*/
		close_socket(&pSess->nRtspSocket);
		close_socket(&pSess->stRtpSender.nRtpSock[0]);
		close_socket(&pSess->stRtpSender.nRtcpSock[0]);
		close_socket(&pSess->stRtpSender.nRtpSock[1]);
		close_socket(&pSess->stRtpSender.nRtcpSock[1]);
		
		if (pSess->stRtpSender.pFrameBuf != NULL)
		{
			free(pSess->stRtpSender.pFrameBuf);
			pSess->stRtpSender.pFrameBuf = NULL;
		}
		
		list_del(&pSess->list);
		pthread_mutex_destroy(&pSess->mutex);
		free(pSess);
		pSess = NULL;
		pthread_mutex_unlock(&gRtspMutex);
    }
}

unsigned long rtsp_session_close(RTSP_SESSION_S* pSess)
{
	int chno = 0;
	
    if (pSess == NULL)
       return 0;

	if(pSess->eState < RTSP_STATE_PLAY)
	{
		/*rtsp会话在stup之前失败，此时并没有加入rtp传输会话，直接从session列表中删除*/
		rtsp_session_lists_remove(pSess);
		return 0;
	}

	/*rtsp 会话计数减一*/
	gRtspSvr.clientCnt[pSess->nChId]--;
	chno = pSess->nChId;
	/*通知rtp_tcp传输线程, 结束*/
	pSess->eState = RTSP_STATE_STOP;
	/*等待rtp_tcp 线程结束之后，才释放pSess*/
	while(0 != pSess->sessThd)
		usleep(10 * 000);

	/*安全地将session从列表中删除,释放pSess所有资源*/
	rtsp_session_lists_remove(pSess);
    
	return 0; 
}

void rtsp_list_client()
{
	RTSP_SESSION_S   *pSess = NULL, *pSessN = NULL;
    int iCnt = 0;

    for(iCnt = 0; iCnt < RTSP_MAX_STREAM; iCnt++)
        LOG_INFO("chn:%d RTSP Client Count: %ld\n", iCnt, gRtspSvr.clientCnt[iCnt]);

    list_for_each_entry_safe(pSess, pSessN, &(gRtspSvr.sessList), list)
        LOG_INFO("sessid %s status %d\n", pSess->szId, pSess->eState);

    return;
}

void check_rtsp_session_state(void)
{
    RTSP_SESSION_S   *pSess = NULL, *pSessN = NULL;

    list_for_each_entry_safe(pSess, pSessN, &(gRtspSvr.sessList), list)
    {
		if(RTSP_STATE_STOP == pSess->eState)
		{
			rtsp_session_close(pSess);
			rtsp_list_client();	
		}
    }
}

void check_rtsp_sock(int fd, RTSP_SESSION_S *pSess)
{
    struct sockaddr_in	cli_addr; 
    struct sockaddr_in	host_addr;
	int					len_inet;
	unsigned  long		l_subnet = 0;
	unsigned  long		r_subnet = 0;
    unsigned  char		*p = NULL;

	/*获取远端IP*/
	len_inet = sizeof(cli_addr);   /*len_inet务必赋于值sizeof(cli_addr)*/
    memset(&cli_addr, 0, sizeof(cli_addr));
	if (getpeername(fd, (struct sockaddr *)&cli_addr, (socklen_t *)&len_inet) != 0)
	{
	    LOG_ERROR("getpeername error\n");
		return;
	}

	r_subnet = cli_addr.sin_addr.s_addr;
	p = (unsigned char *)&(r_subnet);

    strncpy(pSess->szRemoteIP, inet_ntoa(cli_addr.sin_addr), 64);
	pSess->u16RemotePort = (unsigned short)ntohs(cli_addr.sin_port);
    LOG_DEBUG("Remote IP:%s, PORT:%d\n", pSess->szRemoteIP, pSess->u16RemotePort);

	/*获取本地IP*/
	len_inet = sizeof(host_addr);	/*len_inet务必赋于值sizeof(host_addr)*/
	if( getsockname(fd, (struct sockaddr*)&host_addr, (socklen_t *)&len_inet) < 0 )
	{
	    LOG_ERROR("getsockname error\n");
		return ;
	}
	l_subnet = host_addr.sin_addr.s_addr;
	p = (unsigned char *)&(l_subnet);
    strncpy(pSess->szHostIP, inet_ntoa(host_addr.sin_addr), 64);
    LOG_DEBUG("Local IP:%s, PORT:%d\n", inet_ntoa(host_addr.sin_addr), ntohs(host_addr.sin_port));

	/*检查rtsp session是否为同一个网段*/
	if ((l_subnet & 0x0000ffff) & (r_subnet & 0x0000ffff))
	{
		LOG_INFO("client is in the same lan\n");
		pSess->nLan = 1;
	}

	return ;
}

int rtsp_session_process(RTSP_SESSION_S *pSess)
{
    unsigned long opcode = 0, rep = 0;
    unsigned short seq_num, status;
    int			hdr_len, body_len;
	
    if (pSess == NULL)
    {
        LOG_ERROR("process rtsp session, by input session is null\n");
        return 0;
    }	
    /*RTSP消息合法性分析*/
	 if(RTSP_METHOD_RCVD != rtsp_full_msg_parse(pSess, &hdr_len, &body_len))
	 {
		 return 0;
	 }

    /* check for response message. */
    rep = rtsp_valid_resp_msg( pSess->szBuffRecv, &seq_num, &status);
    if ( rep != RTSP_PARSE_IS_RESP )
    {  
        /* not a response message, check for method request. */
        opcode = rtsp_valid_req( pSess->szBuffRecv );
        if ( opcode == RTSP_PARSE_INVALID_OPCODE)
        {
            LOG_ERROR("Method requested was invalid.  Message discarded.  "
                   "Method = %s\n", pSess->szBuffRecv );
            return ERR_RTSP_PARSE_ERROR;
        }
        else if ( opcode == RTSP_PARSE_INVALID)
        {
             LOG_ERROR("Bad request line encountered.  Expected 4 valid "
                   "tokens.  Message discarded" );
            return ERR_RTSP_PARSE_ERROR;
        }
        int iCseq = rtsp_get_cseq(pSess->szBuffRecv);
        if (iCseq > 0 )
        {
            pSess->u32LastRecvSeq = iCseq;
        }
        else
        {
            LOG_ERROR("invalid cseq\n");
        }
        
        status = 0;
    }

    return  rtsp_handle_event(pSess,  opcode, status, pSess->szBuffRecv );
}


unsigned long rtsp_make_resp_header(RTSP_SESSION_S *pSess, int err)
{
    memset(pSess->szBuffSend, 0, RTSP_MAX_PROTOCOL_BUFFER);
    
    char *pTmp = pSess->szBuffSend;

    pTmp += sprintf(pTmp, "%s %d %s\r\n", RTSP_VER_STR, err, rtsp_get_status_str( err ));
    pTmp += sprintf(pTmp,"CSeq: %ld\r\n", pSess->u32LastRecvSeq);
    pTmp += sprintf(pTmp,"Server: Rtsp Server \r\n");

    return (strlen(pSess->szBuffSend)) ;
}


/*
C -- > S
OPTIONS rtsp://192.168.86.252:554/3 RTSP/1.0
CSeq: 141
User-Agent: VLC media player (LIVE555 Streaming Media v2008.07.24)

S --> C
RTSP/1.0 200 OK
CSeq: 141
Server: Rtsp Server V100R001
Public: DESCRIBE, SETUP, TEARDOWN, PLAY
*/
unsigned long rtsp_handle_options(RTSP_SESSION_S *pSess )
{
	int i = rtsp_make_resp_header(pSess, 200);
#if 0    
	sprintf(pSess->szBuffSend+ i , "Public: DESCRIBE, SETUP, TEARDOWN, PLAY, GET_PARAMETER\r\n\r\n");
#else
	sprintf(pSess->szBuffSend+ i , "Public: DESCRIBE, SETUP, TEARDOWN, PLAY\r\n\r\n");
#endif
	rtsp_send_reply(pSess, 200, NULL, 0);
    
	return 0;
}

/*
C -- > S
DESCRIBE rtsp:admin:admin@192.168.86.252:554/3 RTSP/1.0
CSeq: 142
Accept: application/sdp
User-Agent: VLC media player (LIVE555 Streaming Media v2008.11.13)


S -- > C

RTSP/1.0 401 Unauthorized
Server: Rtsp Server 
V100R001
CSeq: 142

WWW-Authenticate: Digest realm="Surveillance Server", nonce="4e405a1c"


C -- > S

DESCRIBE rtsp:admin:admin@192.168.86.252:554 RTSP/1.0
CSeq: 143
Accept: application/sdp
Authorization: Digest username="admin", realm="Surveillance Server", nonce="4e405a1c", uri="rtsp://192.168.86.103:554", response="9e6bc65e2367f64036c8f05d8ec5f35b"
User-Agent: VLC media player (LIVE555 Streaming Media v2008.11.13)

S -- > C
RTSP/1.0 200 OK
CSeq: 143
Server: Rtsp Server
Content-Type: application/sdp
Content-length: 364
Content-Base: rtsp://192.168.86.252/3

v=0
o=StreamingServer 3331435948 1116907222000 IN IP4 192.168.86.252
s=h264.mp4
c=IN IP4 0.0.0.0
t=0 0
a=control:*
m=video 0 RTP/AVP 96
a=control:trackID=0
a=rtpmap:96 H264/90000
a=fmtp:96 packetization-mode=1; sprop-parameter-sets=Z0LgFNoFglE=,aM4wpIA=
m=audio 0 RTP/AVP 97
a=control:trackID=1
a=rtpmap:97 G726/8000/0
a=fmtp:97 packetization-mode=1

1) 根据用户的URI, 服务器获取媒体信息， 
2) 组sdp消息包
3) 发送回复
*/

/*
S -- > C
			
RTSP/1.0 401 Unauthorized
Server: GrandStream Rtsp Server V100R001
CSeq: 142
WWW-Authenticate: Digest realm="Surveillance Server", nonce="4e405a1c"
*/

int rtsp_check_usr(RTSP_SESSION_S *pSess, char *szUrl)
{
   char*   pTemp     = NULL; 
	char*	pParseTmp = NULL;	
	char szName[32] = {0}, szLocalPwd[64] = {0}, szPwd[64] = {0}, szUri[128] = {0};
	unsigned char szMd5Pwd[64] = {0};	
	char szRealM[64]= {0};
	
	strncpy(szRealM, "Surveillance Server", 20);
	
	if((pParseTmp = strstr(pSess->szBuffRecv, RTSP_HDR_AUTHORIZATION)) == NULL)
	{	
			
		random_id(pSess->nonce, 8);
		pTemp = pSess->szBuffSend + rtsp_make_resp_header(pSess, 401);
 			
		pTemp += sprintf(pTemp,"WWW-Authenticate: Digest realm=\"Surveillance Server\", nonce=\"%s\"\r\n\r\n", pSess->nonce);
		strcpy(pSess->szUri, szUrl);
		rtsp_send_reply(pSess, 401, NULL, 0);
		return -1;
	}
	else
	{
		if(0 == rtsp_prase_user_info(pSess->szBuffRecv, szName, szPwd, szUri))
		{
			if(strcmp(szName, "guest") != 0)
			{
#if 0                
				if(gCheckUserCb(szName, szLocalPwd, 1))
#else       
                if (1)
#endif
				{
					md5_auth_build_resonse((char *)szMd5Pwd,
											RTSP_MAX_USERPASSWD_LEN,
											0,
											szName,
											szRealM,
											szLocalPwd,
											pSess->nonce,
											NULL,NULL,NULL,
											"DESCRIBE",
											szUri);

					if (0 != strcmp(szPwd, (char *)szMd5Pwd))
					{/*这里是否要弹出对话框 密码不匹配，回复401*/
						pTemp = pSess->szBuffSend + rtsp_make_resp_header(pSess, 401);
						random_id(pSess->nonce, 8);
	 					pTemp += sprintf(pTemp,"WWW-Authenticate: Digest realm=\"Surveillance Server\", nonce=\"%s\"\r\n\r\n",pSess->nonce);
						strcpy(pSess->szUri, szUrl);
						rtsp_send_reply(pSess, 401, NULL, 0);
						return -1;
					} 
				}
				else
				{ /*这里是否要弹出对话框 不存在该用户或数据库操作失败,回复401*/
					pTemp = pSess->szBuffSend + rtsp_make_resp_header(pSess, 401);	
					random_id(pSess->nonce,8);
					pTemp += sprintf(pTemp,"WWW-Authenticate: Digest realm=\"Surveillance Server\", nonce=\"%s\"\r\n\r\n",pSess->nonce);
					strcpy(pSess->szUri, szUrl);
					rtsp_send_reply(pSess, 401, NULL, 0);						
					return -1;
				
				}
			}
		}
		else
		{	/*这里是否要弹出对话框 用户或密码为空 回复401*/
			pTemp = pSess->szBuffSend + rtsp_make_resp_header(pSess, 401);
			random_id(pSess->nonce,8);
 			pTemp += sprintf(pTemp,"WWW-Authenticate: Digest realm=\"Surveillance Server\", nonce=\"%s\"\r\n\r\n",pSess->nonce);
			strcpy(pSess->szUri, szUrl);
				
			rtsp_send_reply(pSess, 401, NULL, 0);
			return -1;
		}
	}
	return 0;
}

extern int rtsp_get_video_media_info(int nCh, int *pPt, 
					char *profileLevelId, char *sps_pps);

int rtsp_fill_video_sdp(RTSP_SESSION_S* pSess, char *pSzBuff, int nCh)
{
	char szProfileLevelId[8] = {0}, szSpsPps[256] = {0};
	int  nPt = 0;
	
    /*视频媒体描述*/
    /*H264 TrackID=0 RTP_PT 96*/
    rtsp_get_video_media_info(nCh, &nPt, szProfileLevelId, szSpsPps);
    pSzBuff += sprintf(pSzBuff,"m=video 0 RTP/AVP %d\r\n", nPt);
    pSzBuff += sprintf(pSzBuff,"b=AS:70\r\n");
    pSzBuff += sprintf(pSzBuff,"a=control:trackID=0\r\n");
    pSzBuff += sprintf(pSzBuff,"a=rtpmap:%d %s/90000\r\n", nPt, 
                        (nPt == RTP_PT_H264) ? "H264" : "JPEG");
	pSess->stRtpSender.nPayLoad[0] = nPt;

	if (nPt == RTP_PT_H264) 		/* get SPS、PPS、profile level */
  		pSzBuff += sprintf(pSzBuff,"a=fmtp:96 profile-level-id=%s;packetization-mode=1; sprop-parameter-sets=%s\r\n", 
						 szProfileLevelId, szSpsPps);

	return 0;
}

extern int rtsp_get_audio_media_info(int nCh, int *pPt, int *pBits,
								   int *pSample, char *pInfo);

int rtsp_fill_audio_sdp(RTSP_SESSION_S* pSess, char *pSzBuff, int nCh)
{
	int nPt = -1, nBits = 0, nSample = 0;
	char szAudioInfo[32] = {0};

	rtsp_get_audio_media_info(nCh, &nPt, &nBits, &nSample, szAudioInfo);
	if (nPt != -1)
	{
		pSess->stRtpSender.nPayLoad[1] = nPt;
	
	/*主码流音频媒体描述*/
	pSzBuff += sprintf(pSzBuff, "m=audio 0 RTP/AVP %d\r\n", nPt);
	pSzBuff += sprintf(pSzBuff, "b=AS:48\r\n");
	pSzBuff += sprintf(pSzBuff, "a=control:trackID=1\r\n");
	pSzBuff += sprintf(pSzBuff, "a=rtpmap:%d %s/%d\r\n", nPt, szAudioInfo, nSample); 
	if (nPt == RTP_PT_AAC)
		pSzBuff += sprintf(pSzBuff, "a=fmtp:%d streamtype=5;"
		                            "profile-level-id=1;"
		                            "mode=AAC-hbr;sizelength=13;"
		                            "indexlength=3;indexdeltalength=3;"
		                            "config=1290\r\n", nPt); 
	}
	
	return 0;
}

unsigned long rtsp_handle_describe(RTSP_SESSION_S *pSess)
{
    char szUrl[256] = {0}, szSvr[128] = {0}, szChn[16] = {0};
    char szSdp[8192]= {0}, szVideoSdp[2048]= {0}, szAudioSdp[1024] = {0};
    int     nCh = 0, nPort = 0;
    char*   pTemp     = NULL; 
    char*	pParseTmp = NULL;
    int     camera_ch, stream_ch;

	/* sscanf之后 url = "rtsp://192.168.86.252:554/0" */
    if (!sscanf(pSess->szBuffRecv, "%*s %254s ", szUrl)) 
    {    
		LOG_ERROR("pSess->szBuffRecv:%s\n", pSess->szBuffRecv);
        rtsp_send_reply(pSess, RTSP_STATUS_BAD_REQUEST, NULL, 1);	/* bad request */
        return 0;
    }
    /* expects format 'rtsp://server[:port]/filename' */
    if (rtsp_parse_url(szUrl, szSvr, &nPort, szChn) == -1)
    {
        LOG_ERROR("szUrl:%s", szUrl);
        rtsp_send_reply(pSess, RTSP_STATUS_BAD_REQUEST, NULL, 1);
        return 0;
    }
    /*非法通道号*/
	nCh = atoi(szChn);
    if (rtsp_get_chn(szChn, &camera_ch, &stream_ch) < 0)
    {
    //出现错误
		LOG_ERROR("szChn: %s, %d, %d URL:[%s] \n", szChn, camera_ch, stream_ch, pSess->szBuffRecv);     
        rtsp_send_reply(pSess, RTSP_STATUS_BAD_REQUEST, NULL, 1);
        return 0;
    }
	/*登记通道号*/
    pSess->nChId = stream_ch;	
    /*获取客户端请求媒体的描述方法 application/sdp*/
	if (strstr(pSess->szBuffRecv, RTSP_HDR_ACCEPT) != NULL) 
    {
        if (strstr(pSess->szBuffRecv, "application/sdp") == NULL) 
        {
            LOG_ERROR("Only accept require\n");
            rtsp_send_reply(pSess, 551, NULL, 1);	/* Option not supported */
            return 0;
        }
    } 

    /*先定位到User-Agent, 再定位到第一个空格, 就是: 后面, 然后已知搜索到行尾*/
    /*User-Agent: VLC media player (LIVE555 Streaming Media v2008.07.24)*/
    if ((pParseTmp = strstr(pSess->szBuffRecv, RTSP_HDR_USER_AGENT)) != NULL)
    {
        char* p = pParseTmp;
        while( (char)(*p) != 0 )
        {/*碰到结尾, 就跳出*/
            if ( (char)(*p) == 10 || (char)(*p) == 13)
            {
                break;
            }
            if ( (char)*p == ':')
            {
                pParseTmp = p+2; /*jump to "VLC...."*/
            }
            p++;
        }
		//更新用户
        if (p != pParseTmp)
        {
            strncpy(pSess->szUserAgent, pParseTmp, p - pParseTmp);      
        }
    }
	
	if (gRtspSvr.nUseAuth && rtsp_check_usr(pSess, szUrl) < 0)
		return 0;
    
    memset(pSess->szBuffSend, 0, sizeof(pSess->szBuffSend));
    pTemp = pSess->szBuffSend;
    pTemp += sprintf(pTemp, "%s %d %s\r\n", RTSP_VER_STR, 200, rtsp_get_status_str( 200 ));
    pTemp += sprintf(pTemp,"CSeq: %ld\r\n", pSess->u32LastRecvSeq);	
    pTemp += sprintf(pTemp,"Content-Type: application/sdp\r\n");

    //TODO 把一些固定值改为动态值
    char *pTemp2 = szSdp;
    pTemp2 += sprintf(pTemp2,"v=0\r\n");
    pTemp2 += sprintf(pTemp2,"o=StreamingServer 3331435948 1116907222000 IN IP4 %s\r\n", pSess->szHostIP);
    pTemp2 += sprintf(pTemp2,"s=h264.mp4\r\n");
    pTemp2 += sprintf(pTemp2,"c=IN IP4 0.0.0.0\r\n");
    pTemp2 += sprintf(pTemp2,"t=0 0\r\n");
    pTemp2 += sprintf(pTemp2,"a=control:*\r\n");

    rtsp_fill_video_sdp(pSess, szVideoSdp, stream_ch);
    strcat(pTemp2, szVideoSdp);

    if (stream_ch > 2)
    {
        rtsp_fill_audio_sdp(pSess, szAudioSdp, nCh);
        strcat(pTemp2, szAudioSdp);
    }

    pTemp += sprintf(pTemp,"Content-length: %d\r\n", strlen(szSdp));     
    pTemp += sprintf(pTemp,"Content-Base: rtsp://%s/%d\r\n\r\n", pSess->szHostIP, nCh);

    strcat(pTemp, szSdp);
    strcpy(pSess->szUri, szUrl);
    rtsp_send_reply(pSess, 200, NULL, 0);

    return 0;
}

/*
C -- > S
SETUP rtsp://192.168.86.252/3/trackID=0 RTSP/1.0
CSeq: 143
Transport: RTP/AVP;unicast;client_port=4654-4655
User-Agent: VLC media player (LIVE555 Streaming Media v2008.07.24)

S -- > C
RTSP/1.0 200 OK
CSeq: 143
Server: Rtsp Server
Session: 65693745
Transport: RTP/AVP;unicast;client_port=4654-4655;source=192.168.86.252;server_port=32781-0;ssrc=00004E87
*/
int rtsp_handle_setup(RTSP_SESSION_S *pSess )
{
    char* pTemp;
    char* p = NULL; 
    char object[255] = {0}, server[255] = {0}, trash[255] = {0};
    char line[255] = {0}, url[255] = {0};
    int svrport = 0, rtpport = 0, rtcpport = 0, chn = 0, trackid = 0;

	/* Get the URL */
	if (!sscanf(pSess->szBuffRecv, " %*s %254s ", url)) 
    {
		rtsp_send_reply(pSess, 400, 0, 1);	/* bad request */
		return -1;
	}
    url[254] = '\0';
	/* Validate the URL */
	if (rtsp_parse_url(url, server, &svrport, object) != 0) 
    {   
		rtsp_send_reply(pSess, 400, 0, 1);	/* bad request */
		return -1;
	}
    /*获取trackID*/
	#if(PLATFORM_ONVIF == 1)
    	p = strstr(object, "trackID");
	#else
		p = strstr(object, "track");
	#endif
    if (p == NULL)
    {
        LOG_ERROR("No track info\n");
        /*ToDo 发什么回复*/
        rtsp_send_reply(pSess, 406, "Require: Transport settings of rtp/udp;port=nnnn. ", 1);	/* Not Acceptable */
        return -1;
    }
	#if(PLATFORM_ONVIF == 1)
    	sscanf(p, "%8s%d", trash, &trackid);
	#else
		sscanf(p, "%s%d", trash, &trackid);
	#endif
    /*获取Transport Token*/
	if ((p = strstr(pSess->szBuffRecv, RTSP_HDR_TRANSPORT)) == NULL) 
    {
		rtsp_send_reply(pSess, 406, "Require: Transport settings of rtp/udp;port=nnnn. ", 1);	/* Not Acceptable */
		return -1;
	}
    
	if (trackid == RTSP_TRACKID_VIDEO )
	{
		pSess->u8RequestStreamFlag[0] = 1;
	}
	else if (trackid == RTSP_TRACKID_AUDIO)
	{
		pSess->u8RequestStreamFlag[1] = 1;
	}
	else
	{
		LOG_ERROR("rtsp setup chn: %d, trackid %d error\n", chn, trackid);
		rtsp_send_reply(pSess, 400, NULL, 1);
		return -1;
	}

    /*    
    Transport: RTP/AVP;unicast;client_port=6972-6973;source=10.71.147.222;server_port=6970-6971;ssrc=00003654 
    trash = "Transport:"
    line = "RTP/AVP;unicast;client_port=6972-6973;source=10.71.147.222;server_port=6970-6971;ssrc=00003654"
    */
	if (sscanf(p, "%10s%255s", trash, line) != 2) 
    {
		LOG_ERROR("SETUP request malformed\n");
		rtsp_send_reply(pSess, 400, 0, 1);	/* Bad Request */
		return -1;
	}
	LOG_DEBUG("trash: %s\n", trash);
	LOG_DEBUG("line: %s\n", line);

	/* RTP 使用UDP作传输*/
	if( (p = strstr(line, "client_port") )!= NULL)
    {
		p = strstr(p, "=");
		sscanf(p + 1, "%d", &(rtpport));
		p = strstr(p, "-");
		sscanf(p + 1, "%d", &(rtcpport));  

		pSess->u16RemoteRTPPort[trackid]  = rtpport;
		pSess->u16RemoteRTCPPort[trackid] = rtcpport;
		pSess->eTransType = RTP_TRANSPORT_UDP;
		pTemp  = pSess->szBuffSend;
		pTemp += rtsp_make_resp_header(pSess, 200);
		//pTemp += sprintf(pTemp,"Date: 23 Jan 1997 15:35:06 GMT\r\n");
		pTemp += sprintf(pTemp,"Session: %s\r\n", pSess->szId);
		if (trackid == RTSP_TRACKID_VIDEO )
		{
			pSess->u8RequestStreamFlag[RTSP_TRACKID_VIDEO] = 1;
			pTemp += sprintf(pTemp,"Transport: RTP/AVP;unicast;client_port=%d-%d;source=%s;"
								   "server_port=%d-%d;ssrc=00004E87;mode=\"play\"\r\n\r\n",
								   rtpport, 
								   rtcpport, 
								   pSess->szHostIP,
								   pSess->stRtpSender.nRtpPort[0],
								   pSess->stRtpSender.nRtcpPort[0]);
		}
		else
		{
			pSess->u8RequestStreamFlag[RTSP_TRACKID_AUDIO] = 1;
			pTemp += sprintf(pTemp,"Transport: RTP/AVP;unicast;client_port=%d-%d;source=%s;"
									   "server_port=%d-%d;ssrc=00004E87\r\n\r\n",
									   rtpport, 
									   rtcpport, 
									   pSess->szHostIP,
									   pSess->stRtpSender.nRtpPort[1],
								   	   pSess->stRtpSender.nRtcpPort[1]);
		}
	}
	/* RTP 使用TCP作传输*/
	else if( (p = strstr(line, "/TCP") )!= NULL)
	{
		pSess->eTransType = RTP_TRANSPORT_TCP;
		pTemp = pSess->szBuffSend;
		pTemp += rtsp_make_resp_header(pSess, 200);
		pTemp += sprintf(pTemp,"Session: %s;timeout=120\r\n", pSess->szId);

		if( (p = strstr(line, "interleaved")) != NULL)
		{
			if (trackid == RTSP_TRACKID_VIDEO )
			{
				p = strstr(p, "=");
				sscanf(p + 1, "%d", &(pSess->stInterleaved[0].RTP));
				if ( (p = strstr(p, "-")) != NULL )
				{
					sscanf(p + 1, "%d", &(pSess->stInterleaved[0].RTCP));
				}
				else
				{
					pSess->stInterleaved[0].RTCP = pSess->stInterleaved[0].RTP + 1;
				}

				pTemp += sprintf(pTemp,"Transport: RTP/AVP/TCP;unicast;interleaved=%d-%d\r\n\r\n",\
								pSess->stInterleaved[0].RTP, \
								pSess->stInterleaved[0].RTCP);
			}
			else if(trackid == RTSP_TRACKID_AUDIO )
			{
				p = strstr(p, "=");
				sscanf(p + 1, "%d", &(pSess->stInterleaved[1].RTP));
				if ( (p = strstr(p, "-")) != NULL )
				{
					sscanf(p + 1, "%d", &(pSess->stInterleaved[1].RTCP));
				}
				else
				{
					pSess->stInterleaved[1].RTCP = pSess->stInterleaved[1].RTP + 1;
				}
				pTemp += sprintf(pTemp,"Transport: RTP/AVP/TCP;unicast;interleaved=%d-%d\r\n\r\n",\
								pSess->stInterleaved[1].RTP, \
								pSess->stInterleaved[1].RTCP);
			}
			else
			{
				LOG_ERROR("rtsp setup chn: %d, trackid %d  Unsupported Transport\n", chn, trackid);
				rtsp_send_reply(pSess, 461,  "Unsupported Transport", 1);
				return -1;
			}
		}
	}
	else
	{
		LOG_ERROR("rtsp setup chn: %d, trackid %d  Unsupported Transport\n", chn, trackid);
		rtsp_send_reply(pSess, 461,  "Unsupported Transport", 1);
		return -1;
	}

    rtsp_send_reply(pSess, 200, NULL, 0);
    return 0;
}
/*
C --> S
PLAY rtsp://192.168.86.252/3 RTSP/1.0
CSeq: 179
Session: 06261879
Range: npt=0.000-
User-Agent: VLC media player (LIVE555 Streaming Media v2008.07.24)

S --> C
RTSP/1.0 200 OK
CSeq: 179
Server: 
Rtsp Server

1) 准备ok
2) rtp可以开始发送数据
3) 发送回复
*/

int rtsp_handle_play(RTSP_SESSION_S *pSess)
{
	char* pTemp = NULL;
	char  szUrl[256] = {0}, szSvr[128] = {0}, szChn[32] ={0};
	int   nPort = 0;
	int camera_ch, stream_ch;

	/* sscanf之后 url = "rtsp://192.168.86.252:554/01" */
	if (!sscanf(pSess->szBuffRecv, " %*s %254s ", szUrl)) 
	{
		LOG_ERROR("pSess->szBuffRecv: %s\n", pSess->szBuffRecv);   
		rtsp_send_reply(pSess, RTSP_STATUS_BAD_REQUEST, NULL, 1);	/* bad request */
		return 0;
	}
	if (-1 == rtsp_parse_url(szUrl, szSvr, &nPort, szChn))
	{
		LOG_ERROR("szUrl: %s\n", szUrl);   
		rtsp_send_reply(pSess, RTSP_STATUS_BAD_REQUEST, NULL, 1);
		return 0;
	}

	/*非法通道号 camera_ch:摄像机通道, stream_ch: 码流通道 */
	if (rtsp_get_chn(szChn, &camera_ch, &stream_ch) < 0)
	{
		LOG_ERROR("szChn: %s\n", szChn);   
		rtsp_send_reply(pSess, RTSP_STATUS_BAD_REQUEST, NULL, 1);
		return 0;
	}
	/*登记通道号*/
	pSess->nChId = stream_ch;       	
	/*检查rtsp会话已经经过setup过程*/
	if(RTSP_STATE_READY != pSess->eState)
	{
		return -1;
	}

	/*增加RTP TCP传输通道*/
	if(RTP_TRANSPORT_TCP == pSess->eTransType)
	{
		pSess->stRtpSender.stInterleaved[0].RTP = pSess->stInterleaved[0].RTP;
		pSess->stRtpSender.stInterleaved[0].RTCP = pSess->stInterleaved[0].RTCP;
		pSess->stRtpSender.stInterleaved[1].RTP = pSess->stInterleaved[1].RTP;
		pSess->stRtpSender.stInterleaved[1].RTCP = pSess->stInterleaved[1].RTCP;
		pSess->stRtpSender.nRtpTcpSock = pSess->nRtspSocket;
		pSess->stRtpSender.u16LastTs[0] = 3600;		/*初始的ts*/
		pSess->stRtpSender.u16LastTs[1] = 3600;
		pSess->stRtpSender.nAudioSsrc = RTP_DEFAULT_SSRC + pSess->nChId;		/*音频同步源*/
		pSess->stRtpSender.nVideoSsrc = RTP_DEFAULT_SSRC + pSess->nChId + 128;	/*视频同步源*/
	}
	else
	{
		/*增加RTP UDP传输通道*/
		pSess->stRtpSender.addrRemote[0].sin_family  = AF_INET;
		pSess->stRtpSender.addrRemote[1].sin_family  = AF_INET;
		pSess->stRtpSender.addrRemote[0].sin_port    = htons(pSess->u16RemoteRTPPort[0]);
		pSess->stRtpSender.addrRemote[1].sin_port    = htons(pSess->u16RemoteRTPPort[1]);     
		pSess->stRtpSender.addrRemote[0].sin_addr.s_addr = inet_addr(pSess->szRemoteIP); 
		pSess->stRtpSender.addrRemote[1].sin_addr.s_addr = inet_addr(pSess->szRemoteIP); 
		LOG_INFO("remote url %s:%d\n", pSess->szRemoteIP, pSess->u16RemoteRTPPort[0]);
	}
	pSess->eState = RTSP_STATE_PLAY;

	pTemp  = pSess->szBuffSend;
	pTemp += rtsp_make_resp_header(pSess, 200);
#if 0       /* modify by liwei */
	pTemp += sprintf(pTemp,"Session: %s;timeout=120\r\n\r\n", pSess->szId);
#else
	pTemp += sprintf(pTemp,"Range: npt=now-\r\n\r\n");
#endif
	rtsp_send_reply(pSess, 200, NULL, 0);
	create_rtsp_av_transmit(pSess);

	return 0;
}

/*
C --> S
    TEARDOWN rtsp://10.71.147.222/1.mp4 RTSP/1.0
    CSeq: 13
    Session: 117381456211096
    User-Agent: QuickTime/7.0.2 (qtver=7.0.2;os=Windows NT 5.1Service Pack 1)

S --> C
    RTSP/1.0 200 OK
    Server: DSS/5.0.1.1 (Build/464.1.1; Platform/Win32; Release/5;)
    Cseq: 13
    Session: 117381456211096
    Connection: Close
*/
int rtsp_handle_teardown(RTSP_SESSION_S *pSess)
{
	char* pTemp;

	pTemp = pSess->szBuffSend;
	pTemp += rtsp_make_resp_header(pSess, 200);
	pTemp += sprintf(pTemp,"Session: %s\r\n", pSess->szId);
	pTemp += sprintf(pTemp,"Connection: Close\r\n\r\n");
	rtsp_send_reply(pSess, 200, NULL, 0);
	rtsp_session_close(pSess);
//	rtsp_list_client();
	LOG_INFO("sid %s teardown\n", pSess->szId);

    return 0;
}

int rtsp_handle_param(RTSP_SESSION_S *pSess)
{
	char* pTemp = NULL;

	pTemp  = pSess->szBuffSend;
	pTemp += rtsp_make_resp_header(pSess, 200);
	pTemp += sprintf(pTemp,"Session: %s;timeout=120\r\n\r\n", pSess->szId);
	rtsp_send_reply(pSess, 200, NULL, 0);

	return 0;
}

/*
rfc2326bis06-Page42 SETUP can be used in all three states; INIT,and READY
InitState:
    Describe: --> init_state
    Setup   : --> *ready_state
    Teardown: --> init_state
    Options : --> init_state
    Play    : --> init_state
    Pause   : --> init_state

ReadyState:
    Play    : --> *play_state
    Setup   : --> ready
    Teardown: --> ??
    Options : --> 
    pause   : -->
    describe: --> 
*/
unsigned long rtsp_handle_event(RTSP_SESSION_S *pSess, int event, int status, char *buf)
{
    unsigned long u32Ret = 0;

    switch (event) 
    {
        case RTSP_OPTIONS_METHOD:
			LOG_DEBUG("RTSP_OPTIONS_METHOD");
            u32Ret = rtsp_handle_options(pSess);
            break;
        case RTSP_DESCRIBE_METHOD:
			LOG_DEBUG("RTSP_DESCRIBE_METHOD");
            u32Ret = rtsp_handle_describe(pSess);
            break;
        case RTSP_SETUP_METHOD:
			LOG_DEBUG("RTSP_SETUP_METHOD");
			if(0 == (u32Ret = rtsp_handle_setup(pSess)))
				pSess->eState = RTSP_STATE_READY;
            break;
        case RTSP_PLAY_METHOD:
			LOG_DEBUG("RTSP_PLAY_METHOD");
            if (0 == (u32Ret = rtsp_handle_play(pSess)))
            {
    			pSess->eState = RTSP_STATE_PLAY;
    			gRtspSvr.clientCnt[pSess->nChId]++;
//    			rtsp_list_client();
            }
            break;
        case RTSP_TEARDOWN_METHOD:
			LOG_DEBUG("RTSP_TEARDOWN_METHOD");
            u32Ret = rtsp_handle_teardown(pSess);
            break;
        case RTSP_PAUSE_METHOD:
			LOG_DEBUG("RTSP_PAUSE_METHOD");
            u32Ret = rtsp_send_reply(pSess, 455, "Accept: OPTIONS, SETUP, PLAY, TEARDOWN\r\n", 1);
            break;
	    case RTSP_GET_PARAM_METHOD:
			LOG_DEBUG("RTSP_GET_PARAM_METHOD");
            u32Ret = rtsp_handle_param(pSess);
	    	break;
	    case RTSP_SET_PARAM_METHOD:
			LOG_DEBUG("RTSP_SET_PARAM_METHOD");
            u32Ret = rtsp_handle_param(pSess);
	    	break;            
    	default:
			LOG_DEBUG("default");
            u32Ret = rtsp_send_reply(pSess, 501, "Accept: OPTIONS, SETUP, PLAY, TEARDOWN\n", 1);
            break;
    }
    return u32Ret;
}


