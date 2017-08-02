/* 修改成可视化协议 */
#include "protocol/h264_protocol.h"
#include "av_buffer.h"
#include "common.h"
#include "param.h"
#include "normal.h"
#include "debug.h"
#include "socket.h"
#include "avilib.h"
#include "check_param.h"
#include "param_base.h"
#include "param_ext.h"
#include "param_str.h"
#include "task.h"
#include "string_parser.h"
#include "linux_list.h"

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/sem.h>
#include <sys/msg.h>

#undef		TARGET_LEVEL
#define		TARGET_LEVEL		LEVEL_INFO

extern CGI_INTERFACE gWebCgiInf;
extern void *event_proc(void *arg);

static int g_session_id = 0;
static struct list_head g_connect_list;
static int g_session_cnt = 0;

/* 从h264_procotol拷贝出来,避免与core.h中的STREAM_SESSION冲突 */
typedef struct STREAM_SESSION_T
{
    int session_id;          			/* 从sdk端返回,为了让sdk识别相应的通道,控制speaker时用到,值为对应sdk的socket值 */
	int stream_fd;		    			/* 流通道的socket */
	char rec_name[STR_32_LEN];		    /* 录像文件 */	
	int stream_ch;		    			/* 音视频流通道 */
    int running;
    struct list_head list;              /* 链入上级 */
}STREAM_SESSION;

typedef struct CONNECT_SESSION_T
{
	int session_id;
    int stream_cnt;    
    int ctrl_fd;
    int talk_fd;
    int running;
    pthread_t talk_thread;
    struct list_head list;              /* 链入上级 */
    struct list_head stream_list;
}CONNECT_SESSION;

void pre_init_protocol(void)
{
	pthread_t ThreadEventID;    

    g_session_cnt = 0;
    INIT_LIST_HEAD(&g_connect_list);
     
	CreateDetachedTask(&ThreadEventID, event_proc, &g_connect_list);  
}

void destory_stream_session(STREAM_SESSION *pSession)
{    
    if (pSession)
    {
        list_del(&pSession->list);
        if (pSession->stream_fd != -1)
            close_socket(&pSession->stream_fd);
        free(pSession);
        LOG_INFO("destory_stream_session\n");       
    }  
}

void destory_session(CONNECT_SESSION *pSession)
{
    STREAM_SESSION *pStreamSession = NULL, *n_pStreamSession = NULL;

    LOG_INFO("enter destory_session\n");      
    if (pSession)
    {  
        list_del(&pSession->list);
        pSession->running = 0;
        pSession->session_id = -1;
        if (pSession->ctrl_fd != -1)
            close_socket(&pSession->ctrl_fd);
        if (pSession->talk_fd != -1)
            close_socket(&pSession->talk_fd);            
        pSession->stream_cnt = 0;        
        list_for_each_entry_safe(pStreamSession, n_pStreamSession, &pSession->stream_list, list) 
        {
            pStreamSession->running = 0;
            if (pStreamSession->stream_fd != -1)
                close_socket(&pStreamSession->stream_fd);
        }        
        while (1)     /* 等待流退出 */
        {
            usleep(10 * 1000);  
            if (list_empty(&pSession->stream_list))
                break;
        }
        if (pSession->talk_thread)
            pthread_join(pSession->talk_thread, NULL);   
        free(pSession);
        g_session_cnt--;
    }
    LOG_INFO("destory_session[%d]\n", g_session_cnt);      
}

/* 接收命令函数 */
/* 返回实际接收消息正文长度 */
/* 注意: 
    1、实际接收的消息正文小于接收缓存大小时出错
    2、type != NULL 返回type
    3、index != NULL 返回index (消息索引, 避免同类型消息接收有问题)
    4、head.len = 0 没有消息正文
*/
int recv_sdk_cmd(int fd, 			/* 接收socket */
					int *index, 	/* 消息索引 */
					int *type, 		/* 接收的命令类型 */
					void *data, 	/* 接收缓存 */
					int len)		/* 接收缓存大小 */
{
    int ret;
	DATA_HEAD head;

	ret = tcp_block_recv(fd, (char *)&head, sizeof(head));
	if (ret <= 0)   							/* 接收失败或者网络断开 */
	{
        LOG_DEBUG("recv data error[%d]\n", ret);
		return NET_ERROR;
    }
	else if (ret != sizeof(DATA_HEAD))			/* 检查数据头接收 */
	{
        LOG_DEBUG("recv head error\n");
		return NET_ERROR;
    }
	if (CHECK_MARK(head.mark) == SDK_ERROR)		/* 检查标识 */
		return BAD_MARK;
	if (len < head.len)							/* 检查接收长度 */
		return NO_MEMORY;
	if (type)									/* 检查是否需要类型 */
		*type = head.type;
	if (index)									/* 检查是否需要索引 */
		*index = head.index;
	if (head.len == 0)							/* 命令正文长度为0 */
		return 0;
	
    ret = tcp_block_recv(fd, (char *)data, head.len);
	if (ret <= 0)   							/* 接收失败或者网络断开 */
	{
        LOG_DEBUG("recv data error\n");        
		return NET_ERROR;
    }
    else if (ret != head.len)					/* 检查数据体接收 */
    {
        LOG_DEBUG("recv context error\n");                
        return NET_ERROR;
    }

    return head.len;	
}

/* 命令一次性发送, 提高发送效率 */
int send_sdk_cmd(int fd, 			/* 发送socket */
					int type, 		/* 命令类型 */
					int index, 		/* 命令索引 */
					void *data, 	/* 命令数据体, (data可以为NULL, 此时len=0, 没有命令正文) */
					int len)		/* 数据长度 */
{
	int ret;
	DATA_HEAD head;
    static char Buf[512] = {0};
    char *pBuf = Buf;    

    SET_MARK(head.mark);
	head.len = len;
	head.type = type;
	head.index = index;
  
    if (len + sizeof(head) > 512)   /* 过长的消息申请内存 */
    {
        pBuf = (char *)calloc(1, len + sizeof(head));
        if (NULL == pBuf)
        {
            LOG_ERROR("calloc fail\n");
            return -1;
        }
    }
    memcpy(pBuf, &head, sizeof(head));
    if (len > 0)
        memcpy(pBuf + sizeof(head), data, len);
    ret = tcp_block_send(fd, pBuf, len + sizeof(head));
    if (ret != len + sizeof(head))  /* 发送数据失败 */
    {
        if (pBuf != Buf) free(pBuf);
        return NET_ERROR;    
    }
    if (pBuf != Buf) free(pBuf);
	
	return SDK_OK;
}

STREAM_SESSION *stream_id_to_stream_seesion(CONNECT_SESSION *pSession, int stream_id)
{
    STREAM_SESSION *pStreamSession = NULL;

    list_for_each_entry(pStreamSession, &(pSession->stream_list), list)
        if (pStreamSession->session_id == stream_id)
            return pStreamSession;  

    return NULL;
}

void *send_live_stream(void *arg)   /* 发送出去的音视频数据带帧头 */
{
	REAL_FRAME *pFrame; /* 把大内存分配到堆中,否则可能导致fork cgi时内存不够(fork 会直接拷贝父进程的栈,堆copy on write) */ 
    FRAME_HEAD Header;
    STREAM_SESSION *pSession = (STREAM_SESSION *)arg;
    MISC_PARAM *pMiscParam = get_misc_param();
    char send_buf[2048] = {0};
    int video_init = 1;       /* video_init == 0 取IDR */
    int old_stream_ch = 0;

	LOG_DEBUG("send_live_stream start\n");  

    pFrame = (REAL_FRAME *)malloc(sizeof(REAL_FRAME));
    if (NULL == pFrame)
    {
        LOG_ERROR("malloc REAL_FRAME fail\n");
        return NULL;
    }
	pFrame->magic = 0;							/* 一定要置0, 第一帧取关键帧(IDR) */ 
	while (pSession->running)
	{
		int frame_type = 0;

        if ((old_stream_ch != pMiscParam->stream_index) && (video_init == 1))    /* 分辨率改变,重新初始化获取IDR */
        {
            old_stream_ch = pMiscParam->stream_index;
            LOG_INFO("session->stream_ch [0x%x]\n", pMiscParam->stream_index);
            video_init = 0; 
        }
        if (pSession->stream_ch & WEB_CH)
        {
            if (pMiscParam->stream_index == MAIN_STREAM_CH)
            {
    			frame_type |= MAIN_STREAM_FRAME; 		   
                if (video_init == 0)
                    gWebCgiInf.set_param_int_func(GET_IDR_STR, 0);   
                    
            }
            if (pMiscParam->stream_index == SUB1_STREAM_CH)
            {
                frame_type |= SUB1_STREAM_FRAME; 	
                if (video_init == 0)         
                    gWebCgiInf.set_param_int_func(GET_IDR_STR, 1);   
            }
            if (pMiscParam->stream_index == SUB2_STREAM_CH)
            {
                frame_type |= SUB2_STREAM_FRAME;         
                if (video_init == 0)
                    gWebCgiInf.set_param_int_func(GET_IDR_STR, 2);   
             }           
        }  
        else
        {
            if (pSession->stream_ch & MAIN_CH)
            {
    			frame_type |= MAIN_STREAM_FRAME; 		   
                if (video_init == 0)
                    gWebCgiInf.set_param_int_func(GET_IDR_STR, 0);   
             }
            else if (pSession->stream_ch & SUB1_CH)
            {
                frame_type |= SUB1_STREAM_FRAME; 	
                if (video_init == 0)
                    gWebCgiInf.set_param_int_func(GET_IDR_STR, 1);   
            }
        }
        if (pSession->stream_ch & AUDIO_G726)
			frame_type |= FRAME_TYPE_NEAR_G726; 		   			            
        if (pSession->stream_ch & AUDIO_G711)
            frame_type |= FRAME_TYPE_NEAR_G711;           
		if (frame_type == 0)    /* 视频暂停 */
		{
            usleep(100 * 1000);
            continue;
		}
		if (av_read_frame_ext(frame_type, pFrame, !video_init) < 0)
		{
			LOG_ERROR("video loss\n");
			goto DESTORY_STREAM_SESSION;
		}
        if ((video_init == 0) && !(pFrame->type & IDR_MASK))  /* 没有获取到IDR */
        {
            LOG_WARN("get wrong frame[0x%x]\n", pFrame->type);
            continue;
        }
        if (video_init == 0)
        {
            LOG_INFO("frame type [0x%x]\n", pFrame->type);
            video_init = 1;
        }
		if (check_frame(pFrame) != SDK_OK)      /* 检查帧头 */
		{
			LOG_WARN("========  bad frame  =======\n");
            pFrame->magic = 0;
			continue;
		}	
		if (tcp_block_send_ext(pSession->stream_fd, pFrame->data, pFrame->len) != pFrame->len)
		{
			LOG_ERROR("send stream error\n");
			goto DESTORY_STREAM_SESSION;
		}
	}
	
DESTORY_STREAM_SESSION:
    if (pFrame)
        free(pFrame);
	destory_stream_session(pSession); 
    
	return NULL;
}

void parse_filename(char *filename, char *path, char *start_time, char *end_time)
{
	char day[STR_32_LEN] = {0};
	char start[STR_32_LEN] = {0};
	char end[STR_32_LEN] = {0};	

	sscanf(filename, "%08s_%06s_%06s.avi", day, start, end);
	sprintf(path, "/sdcard/%s/record/%s", day, filename);
	if (start_time)
		sprintf(start_time, "%s%s", day, start);
	if (end_time)
		sprintf(end_time, "%s%s", day, end);	
}

void *send_record_file(void *pArg)
{  
    STREAM_SESSION *pSession = (STREAM_SESSION *)pArg;
	int ret;
    int frame_len;
	int frame_type;
	REAL_FRAME frame;
    FRAME_HEAD frame_head;
	int playback_cmd;
	avi_t *AviFile = NULL;
	char path[STR_64_LEN] = {0};
	REC_FILE_INFO file_info;

    print_pid(__func__);    
		
    memset(&frame_head, 0, FRAME_HEAD_LEN);

	parse_filename(pSession->rec_name, path, file_info.start_time, file_info.end_time);
	AviFile = AVI_open_input_file(path, 1);
	LOG_INFO("path: %s, avifile: %p\n", path, AviFile);
	if (AviFile == NULL)    /* some bug */
	{
		memset(&file_info, 0, sizeof(file_info));
		if (tcp_block_send(pSession->stream_fd, &file_info, sizeof(file_info)) < 0)
			goto DESTORY_STREAM_SESSION;
		goto DESTORY_STREAM_SESSION;
	}
	file_info.stream_ch = 0;
	file_info.width = AVI_video_width(AviFile);
	file_info.height = AVI_video_height(AviFile);	
	file_info.fps = AVI_frame_rate(AviFile);
	if (tcp_block_send(pSession->stream_fd, &file_info, sizeof(file_info)) < 0)
		goto DESTORY_STREAM_SESSION;
	
	AVI_seek_start(AviFile);
	while (pSession->running)
	{
		if (recv_sdk_cmd(pSession->stream_fd, NULL, &playback_cmd, NULL, 0) < 0)
		{
			LOG_ERROR("recv playback cmd error\n");
			goto DESTORY_STREAM_SESSION;
		}		
		if (playback_cmd == CMD_GET_FRAME)  /* 获取下一帧 */
		{
			frame_len = read_avi_frame(AviFile, frame.data + FRAME_HEAD_LEN, &frame_type);
			if (frame_len < 0) break;   /* avi文件读取失败, some bug */
    		LOG_DEBUG("ret: %d, type: %d\n", frame_len, frame_type);
			switch (frame_type)
			{
				case 1: 	/* audio */
	                frame_head.type = FRAME_TYPE_NEAR_G726;
					break;
				case 2:		/* P/B frame */
	                frame_head.type = FRAME_TYPE_P;
					break;
				case 3:		/* i frame */
	                frame_head.type = FRAME_TYPE_IDR;
					break;				
				default:	/* end */                                  
					goto FILE_AT_END;				
	                break;
			}
			frame_head.len = frame_len;
	        frame.len = frame_len + FRAME_HEAD_LEN; 
            memcpy(frame.data, &frame_head, FRAME_HEAD_LEN);
	        if (tcp_block_send(pSession->stream_fd, frame.data, frame.len) != frame.len)
			{
				LOG_ERROR("send stream error\n");
				goto DESTORY_STREAM_SESSION;
			}
		}
		else if (playback_cmd == CMD_GET_PRE_VIDEO)   /* 获取前一帧 */
		{
			frame_len = read_pre_video_frame(AviFile, frame.data, &frame_len);
		}
	}
    
FILE_AT_END:       /* 发送一个空包 */
	frame_head.len = 0;
    frame.len = FRAME_HEAD_LEN;  
    memcpy(frame.data, &frame_head, FRAME_HEAD_LEN);    
    if (tcp_block_send(pSession->stream_fd, frame.data, frame.len) != frame.len)
	{
		LOG_ERROR("send stream error\n");
		goto DESTORY_STREAM_SESSION;
	}    
	LOG_INFO("record file end\n");
    
DESTORY_STREAM_SESSION:
	destory_stream_session(pSession); 
	if (AviFile)
	    AVI_close(AviFile);	   
   
	return NULL;    
}

void *event_proc(void *arg)
{
    struct list_head *SessionList =  (struct list_head *)arg;
    CONNECT_SESSION *pSession = NULL, *n_pSession = NULL;
    int ret, retry = 10;
    int ctrl_fd;
    int msg_id;
    MESSAGE recv_msg;
    IPC_EVENT alarm_event;
    int MsgHandle = -1;
    int sockfd;
    int alarm_type;

    print_pid(__func__);

    while (retry--)
    {
        sockfd = tcp_noblock_connect(LOCAL_HOST, EVENT_PORT, 3000);
        if (sockfd >= 0)
            break;
        sleep(1);
    }
    if (sockfd < 0)
    {
        LOG_ERROR("tcp_noblock_connect fail[%s:%d]\n", LOCAL_HOST, EVENT_PORT);
        return NULL;
    }
    set_sock_block(sockfd);
    while (1)
    {
    	if (tcp_block_recv(sockfd, &alarm_type, sizeof(int)) < 0)   /* ? */
    	{
    		LOG_ERROR("tcp_block_recv fail\n");
            return NULL;
    	}	
        
        if (alarm_type == MD_EVENT)
        {
            LOG_DEBUG("============== [web]recv md event =============\n");
            alarm_event.type = MD_EVENT;
    	    list_for_each_entry_safe(pSession, n_pSession, &g_connect_list, list)
    	    {
        		if (send_sdk_cmd(pSession->ctrl_fd, CMD_IPC_EVENT, 0, &alarm_event, sizeof(alarm_event)) < 0)
                {
                    LOG_ERROR("send alarm event fail\n");
                    destory_session(pSession);
                }
            }
        }
    }

    return NULL;
}

void *talk_recv_proc(void *pArg)
{
    CONNECT_SESSION *pSession = (CONNECT_SESSION *)pArg;
	int ret;
	unsigned char audio_buf[1024] = {0};
    FRAME_HEAD *frame_head;

    print_pid(__func__);    

    frame_head = (FRAME_HEAD *)audio_buf;
    set_talk_alive(ON);
	while (pSession->running)
	{
		ret = tcp_force_recv(pSession->talk_fd, audio_buf, sizeof(FRAME_HEAD) + 164);  
		if (ret < 0)
		{
			LOG_INFO("tcp_force_recv fail\n");
			goto DESTORY_TALK_SESSION;
		}
		if (audio_playback(audio_buf + FRAME_HEAD_LEN + 4, 160) < 0)
			LOG_WARN("audio_playback fail\n");
	}
	
DESTORY_TALK_SESSION:
    if (pSession->talk_fd != -1)
    	close_socket(&pSession->talk_fd);
	set_talk_alive(OFF);    

    return NULL;
}

int login_in_proc(CONNECT_SESSION *pSession, PC_LOGIN_IN *login_in_req, IPC_LOGIN_IN *login_in_resp)
{	
	login_in_resp->level = check_user(login_in_req->user, login_in_req->pwd);
	if (login_in_resp->level < 0)
	{
		login_in_resp->session_id = 0;
		login_in_resp->result = AUTH_ERR;
        
		LOG_DEBUG("user(%s:%s) login fail\n", login_in_req->user, login_in_req->pwd);	        
	}
	else
	{			
		login_in_resp->session_id = pSession->session_id;
		login_in_resp->result = AUTH_OK;		
		LOG_DEBUG("user(%s:%s) login in\n", login_in_req->user, login_in_req->pwd);	
    }

	return sizeof(IPC_LOGIN_IN);
}

int login_out_proc(CONNECT_SESSION *pSession, PC_LOGIN_OUT *login_out_req, IPC_LOGIN_OUT *login_out_resp)
{
	login_out_resp->result = RESP_OK;

	return sizeof(IPC_LOGIN_OUT);
}

int start_stream_proc(CONNECT_SESSION *pSession, PC_START_STREAM *start_stream_req, IPC_START_STREAM *start_stream_resp)
{
	STREAM_SESSION *pStreamSession = NULL;
    MISC_PARAM *pMiscParam = get_misc_param();
	
	pStreamSession = stream_id_to_stream_seesion(pSession, start_stream_req->stream_id);
	if (pStreamSession == NULL)
	{
		start_stream_resp->result = BAD_STREAM_ID;
        LOG_INFO("bad stream id[%d]\n", start_stream_req->stream_id);
		goto OUT;
	}
    LOG_DEBUG("start_stream_req->stream_mask[0x%x:0x%x]\n", pStreamSession->stream_ch, start_stream_req->stream_mask);
    pStreamSession->stream_ch |= start_stream_req->stream_mask;
	start_stream_resp->result = RESP_OK;

OUT:	
	return sizeof(IPC_START_STREAM);
}

int stop_stream_proc(CONNECT_SESSION *pSession, PC_STOP_STREAM *stop_stream_req, IPC_STOP_STREAM *stop_stream_resp)
{
	STREAM_SESSION *pStreamSession = NULL;

	pStreamSession = stream_id_to_stream_seesion(pSession, stop_stream_req->stream_id);
	if (pStreamSession == NULL)
	{
		stop_stream_resp->result = BAD_STREAM_ID;
        LOG_WARN("BAD_STREAM_ID [%d]\n", stop_stream_req->stream_id);
        goto OUT;
    }  
    LOG_DEBUG("stop_stream_req->stream_mask[0x%x:0x%x]\n", pStreamSession->stream_ch, stop_stream_req->stream_mask);
	pStreamSession->stream_ch &= ~stop_stream_req->stream_mask;	
	stop_stream_resp->result = RESP_OK;
    
OUT:	
	return sizeof(IPC_STOP_STREAM);
}

int destory_stream_proc(CONNECT_SESSION *pSession, PC_DESTORY_STREAM *destory_stream_req, IPC_DESTORY_STREAM *destory_stream_resp)
{
	STREAM_SESSION *pStreamSession = NULL;

	pStreamSession = stream_id_to_stream_seesion(pSession, destory_stream_req->stream_id);
	if (pStreamSession == NULL)
	{
        LOG_ERROR("destory_stream_proc stream_id[%d]\n", destory_stream_req->stream_id);
		destory_stream_resp->result = BAD_STREAM_ID;
        goto OUT;
    }
    pStreamSession->running = 0;
    close_socket(&pStreamSession->stream_fd);    
	destory_stream_resp->result = RESP_OK;
    
OUT:	
	return sizeof(IPC_DESTORY_STREAM);
}

int get_param_proc(CONNECT_SESSION *pSession, PC_GET_PARAM *get_param_req, IPC_GET_PARAM *get_param_resp)
{
    FTP_PARAM *pFtpParam = NULL;
    EMAIL_PARAM *pEmailParam = NULL;        
    ALARM_PARAM *pAlarmParam = NULL;
    VIDEO_PARAM *pVideoParam = NULL;
    AUDIO_PARAM *pAudioParam = NULL;
    NET_PARAM *pNetParam = NULL;
    WIFI_PARAM *pWifiParam = NULL;
    DDNS_PARAM *pInnerDdnsParam = NULL;
    DDNS_PARAM *pThrDdnsParam = NULL;  
    TIME_PARAM *pTimeParam = NULL;
    USER_PARAM *pUserParam = NULL;
    MULTI_PARAM *pMultiParam = NULL;
    SNAP_PARAM *pSnapParam = NULL;
    RECORD_PARAM *pRecordParam = NULL;
    PTZ_PARAM *pPtzParam = NULL;
    MISC_PARAM *pMiscParam = NULL;
    
	switch (get_param_req->type)
	{
		case TYPE_FTP_PARAM:		
            pFtpParam = get_ftp_param();
			memcpy(get_param_resp->data, pFtpParam, sizeof(FTP_PARAM));
			get_param_resp->len = sizeof(FTP_PARAM);          
			break;
		case TYPE_EMAIL_PARAM:
            pEmailParam = get_email_param();
			memcpy(get_param_resp->data, pEmailParam, sizeof(EMAIL_PARAM));
			get_param_resp->len = sizeof(EMAIL_PARAM);             
			break;			
		case TYPE_ALARM_PARAM:
            pAlarmParam = get_alarm_param();
			memcpy(get_param_resp->data, pAlarmParam, sizeof(ALARM_PARAM));
			get_param_resp->len = sizeof(ALARM_PARAM);         
			break;			
		case TYPE_VIDEO_PARAM:
            pVideoParam = get_video_param();
			memcpy(get_param_resp->data, pVideoParam, sizeof(VIDEO_PARAM));
			get_param_resp->len = sizeof(VIDEO_PARAM);             
			break;			
		case TYPE_AUDIO_PARAM:
            pAudioParam = get_audio_param();
			memcpy(get_param_resp->data, pAudioParam, sizeof(AUDIO_PARAM));
			get_param_resp->len = sizeof(AUDIO_PARAM);              
			break;						
		case TYPE_STATIC_NET_PARAM:
            pNetParam = get_net_param();
			memcpy(get_param_resp->data, pNetParam, sizeof(NET_PARAM));
			get_param_resp->len = sizeof(NET_PARAM);               
			break;			
		case TYPE_WIFI_PARAM:
            pWifiParam = get_wifi_param();
			memcpy(get_param_resp->data, pWifiParam, sizeof(WIFI_PARAM));
			get_param_resp->len = sizeof(WIFI_PARAM);             
			break;			
		case TYPE_THR_DDNS_PARAM:
            pThrDdnsParam = get_thr_ddns_param();
			memcpy(get_param_resp->data, pThrDdnsParam, sizeof(DDNS_PARAM));
			get_param_resp->len = sizeof(DDNS_PARAM);                
			break;			
		case TYPE_TIME_PARAM:
            pTimeParam = get_time_param();
			memcpy(get_param_resp->data, pTimeParam, sizeof(TIME_PARAM));
			get_param_resp->len = sizeof(TIME_PARAM);              
			break;			
		case TYPE_USER_PARAM:
            pUserParam = get_user_param();
			memcpy(get_param_resp->data, pUserParam, sizeof(USER_PARAM));
			get_param_resp->len = sizeof(USER_PARAM);              
			break;			
		case TYPE_MULTI_PARAM:
            pMultiParam = get_multi_param();
			memcpy(get_param_resp->data, pMultiParam, sizeof(MULTI_PARAM));
			get_param_resp->len = sizeof(MULTI_PARAM);               
			break;			
		case TYPE_SNAP_PARAM:
            pSnapParam = get_snap_param();
			memcpy(get_param_resp->data, pSnapParam, sizeof(SNAP_PARAM));
			get_param_resp->len = sizeof(SNAP_PARAM);              
			break;			
		case TYPE_RECORD_PARAM:
            pRecordParam = get_record_param();
			memcpy(get_param_resp->data, pRecordParam, sizeof(RECORD_PARAM));
			get_param_resp->len = sizeof(RECORD_PARAM);               
			break;			
		case TYPE_PTZ_PARAM:
            pPtzParam = get_ptz_param();
			memcpy(get_param_resp->data, pPtzParam, sizeof(PTZ_PARAM));
			get_param_resp->len = sizeof(PTZ_PARAM);               
			break;			
		case TYPE_MISC_PARAM:
            pMiscParam = get_misc_param();
			memcpy(get_param_resp->data, pMiscParam, sizeof(MISC_PARAM));
			get_param_resp->len = sizeof(MISC_PARAM);              
			break;
		default:
			break;
	}
	get_param_resp->result = RESP_OK;
	
	return sizeof(IPC_GET_PARAM);	
}

int set_param_proc(CONNECT_SESSION *pSession, PC_SET_PARAM *set_param_req, IPC_SET_PARAM *set_param_resp)
{
    FTP_PARAM *pFtpParam = NULL;
    EMAIL_PARAM *pEmailParam = NULL;        
    ALARM_PARAM *pAlarmParam = NULL;
    VIDEO_PARAM *pVideoParam = NULL;
    AUDIO_PARAM *pAudioParam = NULL;
    NET_PARAM *pNetParam = NULL;
    WIFI_PARAM *pWifiParam = NULL;
    DDNS_PARAM *pInnerDdnsParam = NULL;
    DDNS_PARAM *pThrDdnsParam = NULL;  
    TIME_PARAM *pTimeParam = NULL;
    USER_PARAM *pUserParam = NULL;
    MULTI_PARAM *pMultiParam = NULL;
    SNAP_PARAM *pSnapParam = NULL;
    RECORD_PARAM *pRecordParam = NULL;
    PTZ_PARAM *pPtzParam = NULL;
    MISC_PARAM *pMiscParam = NULL; 
    char tmp_value[64] = {0};

	switch (set_param_req->type)
	{
		case TYPE_FTP_PARAM:		
            pFtpParam = get_ftp_param();
            if (check_ftp_param((FTP_PARAM *)set_param_req->data) == SDK_OK)
    			memcpy(pFtpParam, set_param_req->data, sizeof(FTP_PARAM));
			break;
		case TYPE_EMAIL_PARAM:
            pEmailParam = get_email_param();
            if (check_email_param((EMAIL_PARAM *)set_param_req->data) == SDK_OK)
                memcpy(pEmailParam, set_param_req->data, sizeof(EMAIL_PARAM));
            break;	            
			break;			
		case TYPE_ALARM_PARAM:
            pAlarmParam = get_alarm_param();
            if (check_alarm_param((ALARM_PARAM *)set_param_req->data) == SDK_OK)
                memcpy(pAlarmParam, set_param_req->data, sizeof(ALARM_PARAM));
            break;			
		case TYPE_VIDEO_PARAM:
            pVideoParam = get_video_param();
            if (check_video_param((VIDEO_PARAM *)set_param_req->data) == SDK_OK)
            {   
                VIDEO_PARAM *pNewParam = (VIDEO_PARAM *)set_param_req->data;
                
                gWebCgiInf.set_param_int_func(VIDEO_RES0_STR, pNewParam->stream[0].resolution);                    
                gWebCgiInf.set_param_int_func(VIDEO_FPS0_STR, pNewParam->stream[0].fps);                
                gWebCgiInf.set_param_int_func(VIDEO_IDR0_STR, pNewParam->stream[0].idr);                                
                gWebCgiInf.set_param_int_func(VIDEO_BITRATE0_STR, pNewParam->stream[0].bitrate);  
                gWebCgiInf.set_param_int_func(VIDEO_RES1_STR, pNewParam->stream[1].resolution);                    
                gWebCgiInf.set_param_int_func(VIDEO_FPS1_STR, pNewParam->stream[1].fps);                
                gWebCgiInf.set_param_int_func(VIDEO_IDR1_STR, pNewParam->stream[1].idr);                                
                gWebCgiInf.set_param_int_func(VIDEO_BITRATE1_STR, pNewParam->stream[1].bitrate); 
                gWebCgiInf.set_param_int_func(VIDEO_RES2_STR, pNewParam->stream[2].resolution);                    
                gWebCgiInf.set_param_int_func(VIDEO_FPS2_STR, pNewParam->stream[2].fps);                
                gWebCgiInf.set_param_int_func(VIDEO_IDR2_STR, pNewParam->stream[2].idr);                                
                gWebCgiInf.set_param_int_func(VIDEO_BITRATE2_STR, pNewParam->stream[2].bitrate);                 
                gWebCgiInf.set_param_int_func(VIDEO_FLIP_STR, pNewParam->flip);
                gWebCgiInf.set_param_int_func(VIDEO_WDR_STR, pNewParam->wideDynamic);   
                gWebCgiInf.set_param_int_func(VIDEO_BRIGHTNESS_STR, pNewParam->brightness);                                
                gWebCgiInf.set_param_int_func(VIDEO_CONTRAST_STR, pNewParam->contrast);                                
                gWebCgiInf.set_param_int_func(VIDEO_SATURATION_STR, pNewParam->saturation);                                
                gWebCgiInf.set_param_int_func(VIDEO_HUE_STR, pNewParam->hue);                                                
                memcpy(pVideoParam, set_param_req->data, sizeof(VIDEO_PARAM));
            }
            break;	            		
		case TYPE_AUDIO_PARAM:
            pAudioParam = get_audio_param();
            if (check_audio_param((AUDIO_PARAM *)set_param_req->data) == SDK_OK)
            {
                AUDIO_PARAM *pNewParam = (AUDIO_PARAM *)set_param_req->data;
                
                gWebCgiInf.set_param_int_func(AUDIO_IN_VOL_STR, pNewParam->in_vol);
                gWebCgiInf.set_param_int_func(AUDIO_OUT_VOL_STR, pNewParam->out_vol);                    
                memcpy(pAudioParam, set_param_req->data, sizeof(AUDIO_PARAM));            
            }
			break;						
		case TYPE_STATIC_NET_PARAM:
            pNetParam = get_net_param();
            if (check_net_param((NET_PARAM *)set_param_req->data) == SDK_OK)
            {
                NET_PARAM *pNewParam = (NET_PARAM *)set_param_req->data;                

                gWebCgiInf.set_param_int_func(NET_DHCP_STR, pNewParam->dhcp);                
                gWebCgiInf.set_param_str_func(NET_STATIC_IP_STR, pNewParam->ip);                                
                gWebCgiInf.set_param_str_func(NET_STATIC_NETMASK_STR, pNewParam->netmask);                                                
                gWebCgiInf.set_param_str_func(NET_STATIC_GATEWAY_STR, pNewParam->gateway);                                                                
                gWebCgiInf.set_param_str_func(NET_STATIC_DNS1_STR, pNewParam->dns1);   
                gWebCgiInf.set_param_str_func(NET_STATIC_DNS2_STR, pNewParam->dns2);                   
                gWebCgiInf.set_param_str_func(NET_ETH_MAC_STR, pNewParam->mac);                               
                memcpy(pNetParam, set_param_req->data, sizeof(NET_PARAM));               
            }
			break;	
		/*
		     case TYPE_WIFI_PARAM:
	            pWifiParam = get_wifi_param();
	            if (check_wifi_param((WIFI_PARAM *)set_param_req->data) == SDK_OK)
	            {
	                WIFI_PARAM *pNewParam = (WIFI_PARAM *)set_param_req->data; 
	                
	                if (strcmp(pNewParam->key, pWifiParam->key) || strcmp(pNewParam->ssid, pWifiParam->key))
	            	    connect_wifi(pNewParam->ssid, pNewParam->key ,pNewParam->auth);                
	                memcpy(pWifiParam, set_param_req->data, sizeof(WIFI_PARAM));              
	            }
				break;	
		*/
		
		case TYPE_THR_DDNS_PARAM:
            pThrDdnsParam = get_thr_ddns_param();
            if (check_thr_ddns_param((DDNS_PARAM *)set_param_req->data) == SDK_OK)
            {
                DDNS_PARAM *pNewParam = (DDNS_PARAM *)set_param_req->data;                 

                /* 重启DDNS服务 */
                gWebCgiInf.set_param_int_func(THR_ENABLE_STR, pNewParam->enable);                 
                gWebCgiInf.set_param_int_func(THR_TYPE_STR, pNewParam->type);                                 
                gWebCgiInf.set_param_str_func(THR_DOMAIN_STR, pNewParam->domain);                 
                gWebCgiInf.set_param_str_func(THR_SERVER_STR, pNewParam->server);                                 
                gWebCgiInf.set_param_str_func(THR_USER_STR, pNewParam->user);                 
                gWebCgiInf.set_param_str_func(THR_PWD_STR, pNewParam->pwd);                                 
                memcpy(pThrDdnsParam, set_param_req->data, sizeof(DDNS_PARAM));               
            }
			break;			
		case TYPE_TIME_PARAM:
            pTimeParam = get_time_param();
            if (check_time_param((TIME_PARAM *)set_param_req->data) == SDK_OK)
            {
                TIME_PARAM *pNewParam = (TIME_PARAM *)set_param_req->data;   

                gWebCgiInf.set_param_int_func(TIME_SYNC_TYPE_STR, pNewParam->sync_type); 
                gWebCgiInf.set_param_int_func(TIME_SAVING_TIME_STR, pNewParam->saving_time);                 
                gWebCgiInf.set_param_int_func(DST_ENB_STR, pNewParam->enb_savingtime);                                 
                gWebCgiInf.set_param_int_func(TIME_TIMEZONE_STR, pNewParam->timezone);                                                 
                memcpy(pTimeParam, set_param_req->data, sizeof(TIME_PARAM));             
            }
			break;			
		case TYPE_USER_PARAM:
            pUserParam = get_user_param();
            if (check_user_param((USER_PARAM *)set_param_req->data) == SDK_OK)
                memcpy(pUserParam, set_param_req->data, sizeof(USER_PARAM));              
			break;			
		case TYPE_MULTI_PARAM:
            pMultiParam = get_multi_param();
            if (check_multi_param((MULTI_PARAM *)set_param_req->data) == SDK_OK)
                memcpy(pMultiParam, set_param_req->data, sizeof(MULTI_PARAM));             
			break;			
		case TYPE_SNAP_PARAM:
            pSnapParam = get_snap_param();
            if (check_snap_param((SNAP_PARAM *)set_param_req->data) == SDK_OK)
                memcpy(pSnapParam, set_param_req->data, sizeof(SNAP_PARAM));               
			break;			
		case TYPE_RECORD_PARAM:
            pRecordParam = get_record_param();
            if (check_record_param((RECORD_PARAM *)set_param_req->data) == SDK_OK)
                memcpy(pRecordParam, set_param_req->data, sizeof(RECORD_PARAM));              
			break;			
		case TYPE_PTZ_PARAM:
            pPtzParam = get_ptz_param();
            if (check_ptz_param((PTZ_PARAM *)set_param_req->data) == SDK_OK)
            {
                PTZ_PARAM *pNewParam = (PTZ_PARAM *)set_param_req->data; 

                gWebCgiInf.set_param_int_func(PTZ_SPEED_STR, pNewParam->speed);                 
                memcpy(pPtzParam, set_param_req->data, sizeof(PTZ_PARAM));              
            }
			break;			
		case TYPE_MISC_PARAM:
            pMiscParam = get_misc_param();
            if (check_misc_param((MISC_PARAM *)set_param_req->data) == SDK_OK)
            {
                MISC_PARAM *pNewParam = (MISC_PARAM *)set_param_req->data; 

                gWebCgiInf.set_param_int_func(PWR_FREQ_STR, pNewParam->pwr_freq);                 
                gWebCgiInf.set_param_int_func(IR_LED_STR, pNewParam->ir_led);                                 
                gWebCgiInf.set_param_int_func(HTTP_PORT_STR, pNewParam->http_port);                  
                gWebCgiInf.set_param_int_func(RTSP_PORT_STR, pNewParam->rtsp_port);                  
                gWebCgiInf.set_param_int_func(ONVIF_PORT_STR, pNewParam->onvif_port);                                  
                gWebCgiInf.set_param_int_func(P2P_ENABLE_STR, pNewParam->p2p_enable);                                                  
                gWebCgiInf.set_param_int_func(UPNP_ENABLE_STR, pNewParam->upnp_enable);                                                                  
                memcpy(pMiscParam, set_param_req->data, sizeof(MISC_PARAM));              
            }
			break;
		default:
			break;
	}
	set_param_resp->result = RESP_OK;

	return sizeof(IPC_SET_PARAM);
}

/* 暂不使用,有问题 */
int search_reclist_proc(CONNECT_SESSION *pSession, PC_SEARCH_RECLIST *search_relist_req, char **data)
{	
	*data = find_rec(search_relist_req->start_time, search_relist_req->end_time);

	return strlen(*data);
}

int keep_alive_proc(CONNECT_SESSION *pSession, PC_KEEP_ALIVE *keep_alive_req, IPC_KEEP_ALIVE *keep_alive_resp)
{
	LOG_DEBUG("------------  client alive  -------------\n");
    keep_alive_resp->result = 0;
    
	return sizeof(IPC_KEEP_ALIVE);
}

/* 
Request Format: 
name1&name2&name3 

Response Format:
var name1 = "value1";\r\n
var name2 = "value2";\r\n
var name3 = "value3";\r\n
var name4 = "bad param";\r\n
*/
int get_param_ext_proc(CONNECT_SESSION *pSession, char *req, char *resp)
{
    char **pTmp = NULL, **formlist = NULL, pStr[STR_256_LEN] = {0}, out_str[STR_1024_LEN * 4] = {0};
    int ret;

	pTmp = formlist = ParserString(req);
    LOG_DEBUG("string: %s\n", req);
	while (*formlist)
	{  
        LOG_DEBUG("param: [%s]\n", *formlist);
		ret = gWebCgiInf.get_param_func(*formlist, out_str);
		if (ret != SDK_OK)
			sprintf(pStr, "var %s = \"bad_param\";\r\n", *formlist);
		else
			sprintf(pStr, "var %s = \"%s\";\r\n", *formlist, out_str);
		strcat(resp, pStr);	 
		formlist += 2;        
    }
    FreeParserVars(pTmp);
    
	return strlen(resp);
}

/*
Request Format: 
name1=value1&name2=value2&name3=value3 

Response Format:
var set_name1 = "set_ok";\r\n
var set_name2 = "set_ok";\r\n
var set_name3 = "set_fail";\r\n
var set_name4 = "set_ok";\r\n
*/
int set_param_ext_proc(CONNECT_SESSION *pSession, char *req, char *resp)
{
    char **pTmp = NULL, **formlist = NULL, pStr[STR_256_LEN] = {0};
    int ret;

	pTmp = formlist = ParserString(req);
	while (*formlist)
	{  
		ret = gWebCgiInf.set_param_str_func(*formlist, *(formlist + 1));
		if (ret != SDK_OK)
			sprintf(pStr, "var set_%s = \"set_fail;\";\r\n", *formlist);
		else
			sprintf(pStr, "var set_%s = \"set_ok;\";\r\n", *formlist);
		strcat(resp, pStr);       
		formlist += 2;        
    }
    FreeParserVars(pTmp);    
    
	return strlen(resp);
}

/* 
Request Format: 
name1=value1&name2=value2&name3=value3 

Response Format:
var set_name1 = "bad_param";\r\n
var set_name2 = "proc_result2";\r\n
var set_name3 = "proc_result3";\r\n
var set_name4 = "proc_result4";\r\n
*/
int cmd_ext_proc(CONNECT_SESSION *pSession, char *req, char *resp)
{
    char **pTmp = NULL, **formlist = NULL, pStr[STR_1024_LEN * 4] = {0}, out_str[STR_1024_LEN * 4] = {0};
    int ret;

	pTmp = formlist = ParserString(req);
	while (*formlist)
	{  
		ret = gWebCgiInf.cmd_proc_func(*formlist, *(formlist + 1), out_str);
		if (ret != SDK_OK)
			sprintf(pStr, "var %s = \"bad_param\";\r\n", *formlist);
		else
			sprintf(pStr, "%s", out_str);
		strcat(resp, pStr); 
		formlist += 2;        
    }
    FreeParserVars(pTmp);    
    
	return strlen(resp);
}

int get_devinfo_proc(CONNECT_SESSION *pSession, char *req, char *resp)
{
    IPC_DEVINFO DevInfo;

    DevInfo.Version = (MAJOR_VERSION << 16) | (MINOR_VERSION << 8) | REVISION_VERSION;
    DevInfo.Width = MAX_WIDTH_SIZE;
    DevInfo.Height = MAX_HEIGHT_SIZE;  
    DevInfo.MaxFps = MAX_FRAME_RATE;
    DevInfo.StreamCnt = 2;
#ifdef  WITH_PTZ
    DevInfo.PtzExist = 1; 
#endif
#ifdef  WITH_AUDIO
    DevInfo.AudioExist = 1;
#endif
#ifdef  WITH_WIFI
    DevInfo.WifiExist = 1;
#endif
#ifdef  BOTH_NET
    DevInfo.WifiExist = 1;
#endif

    return sizeof(IPC_DEVINFO);
}

/*
int wifi_test_proc(CONNECT_SESSION *pSession, char *req, char *resp)
{
    WIFI_PARAM *pWifiParam = (WIFI_PARAM *)req;
    WIFI_PARAM *pSysWifiParam = get_wifi_param();

    if (wifi_test(pWifiParam->ssid, pWifiParam->key, pWifiParam->auth, pSysWifiParam->ssid, pSysWifiParam->key, pSysWifiParam->auth) < 0)
        sprintf(resp, "wifi_test = fail");
    else
        sprintf(resp, "wifi_test = ok");

    return strlen(resp);
}
*/


void *h264_ctrl_proc(void *arg)
{
    CONNECT_SESSION *pSession = (CONNECT_SESSION *)arg;
	int ret;
	int index;    /* 消息索引 */
	int type;     /* 消息类型 */
	int resp_len;
	char recv_buf[STR_1024_LEN] = {0};
	char send_buf[STR_1024_LEN * 4] = {0};	

    print_pid(__func__);

	while (pSession->running)
	{
NEXT_LOOP:        
		memset(recv_buf, 0, sizeof(recv_buf));
		memset(send_buf, 0, sizeof(send_buf));        
		ret = recv_sdk_cmd(pSession->ctrl_fd, &index, &type, recv_buf, sizeof(recv_buf));
		if (ret < 0)
		{
			LOG_ERROR("recv_req error[0x%x][%d]\n", ret, pSession->ctrl_fd);
			goto RESET_SESSION;
		}
        LOG_INFO("recv cmd type[0x%x]\n", type);
		switch (type)
		{
			case CMD_LOGIN_IN: 
				resp_len = login_in_proc(pSession, (PC_LOGIN_IN *)recv_buf, (IPC_LOGIN_IN *)send_buf);
				break;	
			case CMD_LOGIN_OUT:
				resp_len = login_out_proc(pSession, (PC_LOGIN_OUT *)recv_buf, (IPC_LOGIN_OUT *)send_buf);          
				break;	
            case CMD_START_STREAM:	
				resp_len = start_stream_proc(pSession, (PC_START_STREAM *)recv_buf, (IPC_START_STREAM *)send_buf);
                break;
            case CMD_STOP_STREAM:
				resp_len = stop_stream_proc(pSession, (PC_STOP_STREAM *)recv_buf, (IPC_STOP_STREAM *)send_buf);
                break;   
            case CMD_DESTORY_STREAM:
				resp_len = destory_stream_proc(pSession, (PC_DESTORY_STREAM *)recv_buf, (IPC_DESTORY_STREAM *)send_buf);
                break;                 
			case CMD_GET_PARAM:	
				resp_len = get_param_proc(pSession, (PC_GET_PARAM *)recv_buf, (IPC_GET_PARAM *)send_buf);
				break;				
			case CMD_SET_PARAM:		
				resp_len = set_param_proc(pSession, (PC_SET_PARAM *)recv_buf, (IPC_SET_PARAM *)send_buf);				
				break;	
			case CMD_SEARCH_RECLIST:
			{
				char *data;
				
				resp_len = search_reclist_proc(pSession, (PC_SEARCH_RECLIST *)recv_buf, &data);
				send_sdk_cmd(pSession->ctrl_fd, type, index, data, resp_len);
                goto NEXT_LOOP;
				break;	
			}
            case CMD_KEEP_ALIVE:
				resp_len = keep_alive_proc(pSession, (PC_KEEP_ALIVE *)recv_buf, (IPC_KEEP_ALIVE *)send_buf);
                break;
	        case CMD_GET_PARAM_EXT:
                LOG_DEBUG("get cmd CMD_GET_PARAM_EXT\n");
                resp_len = get_param_ext_proc(pSession, recv_buf, send_buf); 
                break;
	        case CMD_SET_PARAM_EXT:
                LOG_DEBUG("get cmd CMD_SET_PARAM_EXT\n");                
                resp_len = set_param_ext_proc(pSession, recv_buf, send_buf);                 
                break;
	        case CMD_PROC_EXT:
                LOG_DEBUG("get cmd CMD_PROC_EXT\n");                
                resp_len = cmd_ext_proc(pSession, recv_buf, send_buf);
                if (strlen(send_buf) == 0)
                {
                    sprintf(send_buf, "cmd_proc_ok");
                    resp_len = strlen(send_buf);
                }
                break;
            case CMD_GET_DEVINFO:
                resp_len = get_devinfo_proc(pSession, recv_buf, send_buf);
                break;
            case CMD_WIFI_TEST:
                //resp_len = wifi_test_proc(pSession, recv_buf, send_buf);                
                break;
			default:
				LOG_WARN("error CTRL_CMD: %d\n", type);
                goto NEXT_LOOP;
				break;
		}	
		if (send_sdk_cmd(pSession->ctrl_fd, type, index, send_buf, resp_len) != SDK_OK)
		{
			LOG_ERROR("ret: %d, type: %d\n", ret, type);
			goto RESET_SESSION;
		}			
		switch (type)			
		{		
			case CMD_LOGIN_IN:		/* 结束线程 */
			{
				IPC_LOGIN_IN *login_in_resp = (IPC_LOGIN_IN *)send_buf;
				
				if (login_in_resp->result != AUTH_OK)					
					goto RESET_SESSION;
				break;
			}
			case CMD_LOGIN_OUT:		/* 结束线程 */	
			{
				IPC_LOGIN_OUT *login_out_resp = (IPC_LOGIN_OUT *)send_buf;
				
				if (login_out_resp->result != SDK_OK)					
					goto RESET_SESSION;
				break;
			}
			default:
				break;
		}
	}
	
RESET_SESSION:
	LOG_INFO("h264_ctrl_proc delete_session\n");
	destory_session(pSession);

    return NULL;
}

int CreateTalkSession(int fd, char *http_request)
{
	int ret = SDK_OK;
	int session_id;
	char str_format[128] = {0};	
	pthread_t ThreadSendID;
	IPC_CREATE_TALK create_talk; 
    CONNECT_SESSION *pSession = NULL;    

    create_talk.result = RESP_OK;
	strcat(str_format, TALK_START);
	strcat(str_format, "_%d");		                /* TALK_START | session_id */
	sscanf(http_request, str_format, &session_id);
    /* 检查session_id */
	list_for_each_entry(pSession, &g_connect_list, list)
        if (pSession->session_id == session_id)
            break;
    if (pSession->session_id != session_id)    /* 没有找到匹配的session */
    {
		LOG_INFO("bad session_id[%d], http_request: %s\n", session_id, http_request);
		create_talk.result = BAD_SESSION_ID;
		ret = BAD_SESSION_ID; 
        goto SEND_RESPONSE;
    } 
    /* 不支持混音时,只允许单通道输入 */
    if (get_talk_alive() == ON)
    {
        LOG_INFO("talk has opened\n");
        create_talk.result = TALK_HAS_OPENED;
        ret = TALK_HAS_OPENED;
    }

SEND_RESPONSE:
	if (send_sdk_cmd(fd, CMD_CREATE_TALK, 0, &create_talk, sizeof(create_talk)) != SDK_OK)
        ret = NET_ERROR;
	if (ret == SDK_OK)
	{	
        LOG_INFO("CreateTalkSession OK\n");	
		pSession->talk_fd = fd;
		CreateTask(&pSession->talk_thread, talk_recv_proc, (void *)pSession);			
	}
    else
        close_socket(&fd);  

	return ret;
}

int CreateStreamSession(int fd, char *http_request)
{
	int ret = SDK_OK;
	int session_id;
	struct timeval tv;      
	int stream_ch = 0;
	char stream_mask[STR_64_LEN] = {0};
	char filename[STR_64_LEN] = {0};
	char str_format[128] = {0};	
	pthread_t ThreadSendID;
	IPC_CREATE_STREAM create_stream;	
	char rec_path[STR_64_LEN];
    CONNECT_SESSION *pSession = NULL;
    STREAM_SESSION *pStreamSession = NULL;

    create_stream.result = RESP_OK;
	/* check session id */
	strcat(str_format, STREAM_START);
	strcat(str_format, "_%d_%s");		/* STREAM_START | session_id | stream_mask */
	sscanf(http_request, str_format, &session_id, stream_mask);
	if (!strncmp(stream_mask, "Live", strlen("Live")))
		sscanf(stream_mask, "Live%d", &stream_ch);
	else if (!strncmp(stream_mask, "File", strlen("File")))
	{
		sscanf(stream_mask, "File%s", filename);		
		parse_filename(filename, rec_path, NULL, NULL);
		if (access(rec_path, F_OK) != 0)
		{
			create_stream.result = REC_NOT_EXIT;
            ret = REC_NOT_EXIT;
        }
	}
    /* 检查session_id */
	list_for_each_entry(pSession, &g_connect_list, list)
        if (pSession->session_id == session_id)
            break;
    if (pSession->session_id != session_id)    /* 没有找到匹配的session */
    {
		LOG_INFO("bad session_id[%d], http_request: %s\n", session_id, http_request);
		create_stream.result = BAD_SESSION_ID;
		ret = BAD_SESSION_ID; 
        goto SEND_RESPONSE;
    }
    if (pSession->stream_cnt >= ONE_CONNECT_STREAM_CH)    /* 暂不使用(不限制流个数) */
    {
        create_stream.result = STREAM_CNT_OVERFLOW;
        ret = STREAM_CNT_OVERFLOW;
        goto SEND_RESPONSE;  
    }
    if ((pStreamSession = (STREAM_SESSION *)calloc(1, sizeof(STREAM_SESSION))) == NULL)
    {
        create_stream.result = STREAM_CNT_OVERFLOW;             
        ret = MALLOC_FAIL;
    }
    create_stream.stream_id = ++g_session_id;   /* 发送stream_id到客户端,关闭码流需要 */
SEND_RESPONSE:    
	if (send_sdk_cmd(fd, CMD_CREATE_STREAM, 0, &create_stream, sizeof(create_stream)) != SDK_OK)
        ret = NET_ERROR;
    if (ret == SDK_OK)
    {
        LOG_DEBUG("CreateStreamSession[%d] OK\n", fd);	        
        pStreamSession->session_id = create_stream.stream_id;
        pStreamSession->stream_fd = fd;
        pStreamSession->stream_ch = stream_ch;  
        strcpy(pStreamSession->rec_name, filename);
        list_add_tail(&pStreamSession->list, &pSession->stream_list);
		if (strlen(filename))		/* 发送录像流 */			
			CreateDetachedTask(&ThreadSendID, send_record_file, pStreamSession);						
		else						/* 发送实时流 */
		{
			CreateDetachedTask(&ThreadSendID, send_live_stream, pStreamSession);			
			pStreamSession->running = 1;			
		}
    }
    else
    {
        if (pStreamSession) 
            free(pStreamSession);
	    close_socket(&fd);
    }

	return ret;
}

int CreateCtrlSession(int fd)
{
	int ret = SDK_OK;
	pthread_t ThreadCtrlID;
	struct timeval tv;
	PC_CREATE_CTRL create_ctrl; 
    CONNECT_SESSION *pSession = NULL;

    create_ctrl.result = RESP_OK;
    
	/* 检查是否超过连接数 */
    if (g_session_cnt >= MAX_CONNECT_CNT)
    {
		LOG_WARN("have on available connection\n");
		create_ctrl.result = NO_AVAILABLE_CONNECTION;
        ret = NO_AVAILABLE_CONNECTION;
        goto SEND_RESPONSE;
	}
    if ((pSession = (CONNECT_SESSION *)calloc(1, sizeof(CONNECT_SESSION))) == NULL)
    {
		create_ctrl.result = NO_AVAILABLE_CONNECTION;        
        ret = MALLOC_FAIL;    
    }
    
SEND_RESPONSE:    
	if (send_sdk_cmd(fd, CMD_CREATE_CTRL, 0, &create_ctrl, sizeof(create_ctrl)) != SDK_OK)
        ret = NET_ERROR;
	if (ret == SDK_OK)
	{
		LOG_DEBUG("CreateCtrlSession OK\n");      
        pSession->session_id = ++g_session_id;	/* 随机生成会话id */
        pSession->ctrl_fd = fd;
        pSession->talk_fd = -1;   
        pSession->running = 1;
        pSession->stream_cnt = 0; 
        pSession->talk_thread = 0;
        INIT_LIST_HEAD(&pSession->stream_list);
        list_add_tail(&pSession->list, &g_connect_list);
        g_session_cnt++;

		CreateDetachedTask(&ThreadCtrlID, h264_ctrl_proc, pSession);		
	}
    else
    {
        if (pSession) free(pSession);
	    close_socket(&fd);
    }
	
	return ret;
}

int CheckH264Procotol(int fd, char *http_request)
{
	if (!strcmp(http_request, CTRL_START))
	{
		LOG_DEBUG("recv a ctrl session\n");
		set_sock_block(fd);
		set_sock_attr(fd, 1, 5000, 120 * 1000, 1024, 1024);         /* 接收超时120s, 客户端60s发送一次心跳 */
		return CreateCtrlSession(fd);
	}
	else if(!strncmp(http_request, STREAM_START, strlen(STREAM_START)))
	{
		LOG_DEBUG("recv a stream session\n");
		set_sock_block(fd);	     
#if 0   /* 导致严重后果,待分析 */
		set_sock_attr(fd, 1, 5000, 5000, 700 * 1024, 1024);
#endif
        set_sock_nodelay(fd);
		return CreateStreamSession(fd, http_request);
	}
    /* talk数据使用独立的socket, 发送流关闭后语音数据还可以接收 */
	else if(!strncmp(http_request, TALK_START, strlen(TALK_START))) 
	{
        LOG_DEBUG("recv a talk session\n");
        set_sock_block(fd);
		set_sock_attr(fd, 1, 5000, 5000, 1024, 1024);	      
    	return CreateTalkSession(fd, http_request);
	}
    else
        return SDK_ERROR;

    return SDK_OK;
}

