#include "param_base.h"
#include "param_ext.h"
#include "param.h"
#include "av_buffer.h"
#include "task/task_video.h"
#include "common.h"
#include "hi_codec.h"
#include "hi_osd.h"
//#include "hi_video.h"
//#include "hi_md_ext.h"
#include "normal.h"
//#include "mw.h"
//#include "ftp.h"
//#include "smtp.h"
#include "task.h"
#include "debug.h"
//#include "msg.h"
//#include "motor.h"
//#include "hi_playback.h"
#include "socket.h"

#include <unistd.h>
#include <fcntl.h>
#include <stdint.h> 			/* for uint32_t, is not a good solution */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/ip_icmp.h>
#include <net/if.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>

#undef		TARGET_LEVEL
#define		TARGET_LEVEL		LEVEL_INFO


//nihao	*My_nihao[2];

CTaskVideo::CTaskVideo(void)
{
	int test = 1;
	strcpy(mTaskName, "VIDEO");
	mpAlarmParam  = NULL;
	mpAvcHeader[0].pps.len   = 0;
	mpAvcHeader[0].sps.len   = 0;		
	//My_nihao->Hello = test;
	mGetRecFlag = 0;	
	mGetPicFlag = 0; 
	mPlayAudioFlag = 0;
}

CTaskVideo::~CTaskVideo(void)
{

}


/* format [SPS | PPS | SEI | IDR] */
int GetNal(char *data, int len, IDR_INFO *idr_info)		/* 分离 SPS PPS IDR */
{
	int index = 0, nal_start = -1, *pLen = NULL;
	char *pData = data;

	if ((pData[0] != 0x0) || (pData[1] != 0x0) ||
		(pData[2] != 0x0) || (pData[3] != 0x1) || ((pData[4] & 0x1f) != TYPE_SPS))
	{
		LOG_INFO("NAL_ERROR\n");
		return SDK_ERROR;	
	}
	else 
	{
		LOG_INFO("NAL_OK\n");
	}
	idr_info->idr = idr_info->pps = idr_info->sps = NULL;
	//	判断帧类型
	while (1)
	{
		if ((pData[index]     == 0x0) && (pData[index + 1] == 0x0) &&
			(pData[index + 2] == 0x0) && (pData[index + 3] == 0x1))
		{
			if (NULL != pLen)
				*pLen = index - nal_start;
			index += 4;			
			nal_start = index;
			LOG_INFO("Frame_Type = %d\n",pData[nal_start] & 0x1f);
			switch (pData[nal_start] & 0x1f)
			{
				case TYPE_SPS:	
					idr_info->sps = &pData[nal_start];
					pLen = &idr_info->sps_len;
					break;
				case TYPE_PPS:
					idr_info->pps = &pData[nal_start];
					pLen = &idr_info->pps_len;
					break;
				case TYPE_IDR:		/* 加上00 00 00 01 */
					idr_info->idr = &pData[nal_start - 4];
					idr_info->idr_len = len - nal_start + 4;
					goto OUT;
				default:	/* SEI */
					pLen = NULL;
					break;
			}
		}
		else
			index++;
	}
	
OUT:
	LOG_INFO("len:%d, pps_len[%d], sps_len[%d], idr_len[%d]\n", 
				len, idr_info->pps_len, idr_info->sps_len, idr_info->idr_len);
	
	return SDK_OK;
}

/* 包含视频头 */
//len   = HI_FRAME_HEAD_SIZE + 真实数据长度
//data = 包头 + 真实数据
//

//FILE *pFile = fopen("put_stream_cb.h264", "wb");
void put_stream_cb(int ch, char *data, int len, unsigned int pts_ms, void *user_data)
{
	FRAME_HEAD *frame_head = (FRAME_HEAD *)data;
	IDR_INFO idr_info;
	char *pData;

	CTaskVideo *pTaskVideo = (CTaskVideo *)user_data;

//	
//	fflush(pFile);

	SET_MARK(frame_head->mark);
	frame_head->pts = pts_ms;
	frame_head->len = len - FRAME_HEAD_LEN;		//数据包的长度
	frame_head->type = STREAM_TYPE_VIDEO;		/* 与av_write_frame中的type不同 */
	unsigned int  i  = 0;

	if (!pTaskVideo->mpAvcHeader[0].pps.len || !pTaskVideo->mpAvcHeader[0].sps.len)
	{
		LOG_INFO("!!!!!!!!!!!!!!!!!!!Step 1!!!!!!!!!!!!!!!!!!!!\n");
		if (GetNal(data + FRAME_HEAD_LEN, len - FRAME_HEAD_LEN, &idr_info) == SDK_OK)
		{
			LOG_INFO("!!!!!!!!!!!!!!!!!!!Step 2!!!!!!!!!!!!!!!!!!!!\n");
			memcpy(pTaskVideo->mpAvcHeader[0].pps.data, idr_info.pps, idr_info.pps_len);
			memcpy(pTaskVideo->mpAvcHeader[0].sps.data, idr_info.sps, idr_info.sps_len);
			pTaskVideo->mpAvcHeader[0].pps.len = idr_info.pps_len;
			pTaskVideo->mpAvcHeader[0].sps.len = idr_info.sps_len;	
		}
	}

	

#if 1
	if ((data[FRAME_HEAD_LEN + 4] & 0x1f) == TYPE_SPS)
	{
		av_write_frame(FRAME_TYPE_IDR , data, len);
	}
	else
	{
		av_write_frame(FRAME_TYPE_P   , data, len); 
	}
#endif	

	//fwrite(data + sizeof(FRAME_HEAD),len - sizeof(FRAME_HEAD), 1, pFile);


}


#if 0
void *sd_thread_proc(void *arg)
{
	FILE_INFO *pFileInfo = (FILE_INFO *)arg;
	char day_str[STR_64_LEN] = {0};
	char path[STR_128_LEN] = {0};
	char dir_name[64] = {0};

	LOG_INFO("enter alarm sd_thread_proc\n");
	if (sd_exist() < 0)
		return NULL;

	get_time_str(NULL, day_str, NULL, NULL);
	if (strstr(pFileInfo->file_list[0], ".avi"))
		sprintf(path, "/mnt/sdcard/record/alarm/%s/%s", day_str, get_file_name(pFileInfo->file_list[0]));
	else if (strstr(pFileInfo->file_list[0], ".jpg"))
		sprintf(path, "/mnt/sdcard/picture/alarm/%s/%s", day_str, get_file_name(pFileInfo->file_list[0]));
	get_dir_name(path, dir_name);
	if (path_exist(dir_name) == -1)
		mkdirs(dir_name);
	copy_file(pFileInfo->file_list[0], path);
	
	return NULL;
}

void *ftp_thread_proc(void *arg)
{
	int ret;
	FILE_INFO *pFileInfo = (FILE_INFO *)arg;
	FTP_INFO ftp_info;
	FTP_PARAM *pFtpParam = NULL;
	
	pFtpParam = get_ftp_param();
	ftp_info.ctrlfd = -1;
	ftp_info.datafd = -1;	
	ftp_info.portfd = -1;		
	memcpy(&ftp_info.param, pFtpParam, sizeof(FTP_PARAM));
	ret = ftp_upload_one_file(&ftp_info, pFileInfo->file_list[0], get_file_name(pFileInfo->file_list[0]));
	if (ret < 0)
		LOG_ERROR("ftp_upload_one_file fail\n");

	return NULL;
}

void *email_thread_proc(void *arg)
{
	int ret;
	FILE_INFO *pFileInfo = (FILE_INFO *)arg;
	EMAIL_PARAM *pEmailParam = NULL;

	pEmailParam = get_email_param();
	ret = send_mail(pEmailParam, DEFAULT_ALARM_SUBJECT, DEFAULT_ALARM_CONTEXT, pFileInfo->file_list, pFileInfo->cnt);
	if (ret < 0)
		LOG_ERROR("email_send fail\n");	

	return NULL;
}

void *pic_thread_proc(void *pArg)
{
	CTaskVideo *pTaskVideo = (CTaskVideo *)pArg;
	int i;
	FILE_INFO *pFileInfo;	/* 变量释放的时候注意: 可能导致段错误 */
	pthread_t sd_thread = 0, ftp_thread = 0, email_thread = 0;	
	RAM_PARAM *pRamParam = get_ram_param(); 

	pFileInfo = (FILE_INFO *)calloc(1, sizeof(FILE_INFO));
	pFileInfo->file_list = get_pic(1);
	if ((pFileInfo->file_list == NULL) || (pFileInfo->file_list[0] == NULL))
	{
		LOG_ERROR("-------------- get_pic fail -------------\n");
		if (pFileInfo)
			free(pFileInfo);
		return NULL;
	}
	pFileInfo->cnt = 1;
	if ((pTaskVideo->mpAlarmParam->pic_to_sd) && (sd_exist() == 0))
	{
		remove_sd(FILE_TYPE_SNAP, MIN_LIMIT_FOR_SNAP, pRamParam->recpath);	
		CreateTask(&sd_thread, sd_thread_proc, (void *)pFileInfo);
	}
	if (pTaskVideo->mpAlarmParam->ftp_pic)
	{
		CreateTask(&ftp_thread, ftp_thread_proc, (void *)pFileInfo);
	}
	if (pTaskVideo->mpAlarmParam->email_pic)	
	{
		CreateTask(&email_thread, email_thread_proc, (void *)pFileInfo);
	}
	if (sd_thread > 0)
		pthread_join(sd_thread, NULL);	
	if (ftp_thread > 0)
	pthread_join(ftp_thread, NULL);
	if (email_thread > 0)	
		pthread_join(email_thread, NULL);	
	/* 删除文件 */	
	for (i = 0; i < pFileInfo->cnt; i++)
	{
		unlink(pFileInfo->file_list[i]);		
		free(pFileInfo->file_list[i]);
	}
	free(pFileInfo->file_list);
	free(pFileInfo);	
	pTaskVideo->mGetPicFlag = 0;

	return NULL;
}

void *rec_thread_proc(void *pArg)	/* 录像先存入sd卡, 如果sd不存在不录像 */
{
	CTaskVideo *pTaskVideo = (CTaskVideo *)pArg;
	pthread_t ftp_thread = 0;
	FILE_INFO *pFileInfo;
	char start_time_str[STR_64_LEN] = {0}, end_time_str[STR_64_LEN] = {0}, day_str[STR_64_LEN] = {0};	
	char path[STR_128_LEN] = {0};
	char dir_name[64] = {0};	
	RECORD_INFO RecordInfo;	
	int sch_ret;
	struct timeval timenow;
	ALARM_PARAM *pAlarmParam = get_alarm_param();
	RAM_PARAM *pRamParam = get_ram_param(); 
	
	sch_ret = CheckSch_Alarm(pAlarmParam);
	if (sch_ret != SDK_OK)
	{
		pTaskVideo->mGetRecFlag = 0;
		return NULL;
	}

	pFileInfo = (FILE_INFO *)calloc(1, sizeof(FILE_INFO));		
	pFileInfo->file_list = (char **)calloc(1, sizeof(char *));
	pFileInfo->cnt = 1;

	remove_sd(FILE_TYPE_RECORD, MIN_LIMIT_FOR_RECORD, pRamParam->recpath);
	get_time_str(NULL, day_str, start_time_str, &RecordInfo.wday);
	strcpy(end_time_str, start_time_str);
	sprintf(path, "/mnt/sdcard/record/alarm/%s", day_str);	

	pFileInfo->file_list[0] = get_rec(ALARM_RECORD_PEROID, path, &pTaskVideo->mpAlarmParam->rec_to_sd);
	if (pTaskVideo->mpAlarmParam->rec_to_ftp == ON)
	{	
		CreateTask(&ftp_thread, ftp_thread_proc, pFileInfo);	
	}
	//if (pTaskVideo->mpAlarmParam->rec_to_sd == ON)
	/*if ((!pAlarmParam->rec_to_sd) && (sd_exist() == 0))
	{
		get_time_str(NULL, day_str, end_time_str, NULL);
		sprintf(RecordInfo.EndTime, "%s%s", day_str, end_time_str);
		write_record_index_endtime(RecordInfo.EndTime, NORAL_RECORD_INFO_FILE);
	}*/
	//else
		//unlink(pFileInfo->file_list[0]);
	if (ftp_thread > 0)
		pthread_join(ftp_thread, NULL);			
	/* 删除文件 */
	free(pFileInfo->file_list[0]);	
	free(pFileInfo->file_list);
	free(pFileInfo);	
	pTaskVideo->mGetRecFlag = 0;	

	return NULL;
}

#endif
#if 0
void *audio_thread_proc(void *pArg)
{
	CTaskVideo *pTaskVideo = (CTaskVideo *)pArg;
	WAV_HEAD Header;	
	FILE *file;
	char buf[164] = {0x00,0x01,0x50,0x00};

	file = fopen(AUDIO_ALARM_EN, "rb");
	if (file)
	{
		fread(&Header, 1, sizeof(WAV_HEAD), file);
		while (!feof(file))
		{
			fread(buf + 4, 1, sizeof(buf) - 4, file);		// Reading data in chunks of BUFSIZE
			hi_write_audio(buf, sizeof(buf));
		}
	}
	else
	{
		LOG_ERROR("open file[%s] fail\n", AUDIO_ALARM_EN);
		return NULL;
	}
	fclose(file);
	pTaskVideo->mPlayAudioFlag = 0;

	return NULL;
}


#define		BLOCK_WIDTH		(32)
#define		BLOCK_HEIGHT	(32)
#define		X_UNIT	(MAX_WIDTH_SIZE / BLOCK_WIDTH)		/* 一行包含多少块 */
#define		Y_UNIT	(MAX_HEIGHT_SIZE / BLOCK_HEIGHT)	/* 一列包含多少块 */
void md_result_cb(SAMPLE_RECT_ARRAY_S *pstRect, void *pUserData)
{
	CTaskVideo *pTaskVideo = (CTaskVideo *)pUserData;
	unsigned int i, j, k;
	unsigned int x1, x2, y1, y2;
	ALARM_PARAM *pAlarmParam;
	pthread_t pic_thread, rec_thread, audio_thread;
	MESSAGE msg;
	static struct timespec old_time = {0}, tm_now = {0};
	EVENT_CONNECT_SESSION *pSession, *n_pSession;

	pAlarmParam = get_alarm_param();
	if (pAlarmParam->enable[0] == OFF)
		return;
	
	clock_gettime(CLOCK_MONOTONIC, &tm_now);	
	for (i = 0; i < pstRect->u16Num; i++)
	{
		if (pstRect->astRect[i].astPoint[0].s32X < pstRect->astRect[i].astPoint[1].s32X)
		{
			x1 = pstRect->astRect[i].astPoint[0].s32X;
			x2 = pstRect->astRect[i].astPoint[1].s32X;
		}
		else
		{
			x2 = pstRect->astRect[i].astPoint[0].s32X;
			x1 = pstRect->astRect[i].astPoint[1].s32X;
		}
		if (pstRect->astRect[i].astPoint[0].s32Y < pstRect->astRect[i].astPoint[2].s32Y)
		{
			y1 = pstRect->astRect[i].astPoint[0].s32Y;
			y2 = pstRect->astRect[i].astPoint[2].s32Y;
		}
		else
		{
			y2 = pstRect->astRect[i].astPoint[0].s32Y;
			y1 = pstRect->astRect[i].astPoint[2].s32Y;
		}
		x1 = ROUND_DOWN(x1, X_UNIT);
		x2 = ROUND_UP(x2, X_UNIT);	
		y1 = ROUND_DOWN(y1, Y_UNIT);
		y2 = ROUND_UP(y2, Y_UNIT);	
//		LOG_DEBUG("pos: %d_%d %d_%d\n", x1, x2, y1, y2);
		for (j = y1; j <= y2; j++)	/* 纵向 */
			for (k = x1; k <= x2; k++)
			{
				if ((pAlarmParam->alarm_bit[(j * BLOCK_WIDTH + k) / 8] & BIT((j * BLOCK_WIDTH + k) % 8)) && 	/* 检查区域 */
					(get_motor_status() == 0) && 						/* 检查云台 */
					(tm_now.tv_sec - old_time.tv_sec) > 2)				/* 检查报警间隔 */
				{
					memcpy(&old_time, &tm_now, sizeof(struct timespec));
					goto DO_ACTION;
				}					
			}
	}

	return;
	
DO_ACTION:	
	LOG_DEBUG("motion detect happen[%d_%d_%d]\n", pTaskVideo->mGetRecFlag, pAlarmParam->rec_to_sd, pAlarmParam->rec_to_ftp);
	list_for_each_entry_safe(pSession, n_pSession, &pTaskVideo->mEventSessionList, list)
	{
		int alarm_type = MD_EVENT;
		int ret;

		ret = tcp_noblock_send(pSession->sockfd, (char *)&alarm_type, sizeof(int));
		if (ret < 0)
		{
			LOG_WARN("tcp_noblock_send fail\n");
			list_del(&pSession->list);
			close_socket(&pSession->sockfd);
			free(pSession);
		}
		/* 异常处理 */	
	}
	if (!pTaskVideo->mGetPicFlag && (pAlarmParam->pic_to_sd || pAlarmParam->ftp_pic || pAlarmParam->email_pic))		/* 图片抓拍报警 */
	{
		pTaskVideo->mGetPicFlag = 1;
		CreateDetachedTask(&pic_thread, pic_thread_proc, pTaskVideo);
	}
	if (!pTaskVideo->mGetRecFlag && (pAlarmParam->rec_to_sd || pAlarmParam->rec_to_ftp) && (sd_exist() == 0))		/* 录像报警 */
	{
		pTaskVideo->mGetRecFlag = 1;	
		CreateDetachedTask(&rec_thread, rec_thread_proc, pTaskVideo);		/* 声音报警 */
	}
	if (!pTaskVideo->mPlayAudioFlag && (pAlarmParam->sound_enb == ON))
	{
		pTaskVideo->mPlayAudioFlag = 1;
		CreateDetachedTask(&audio_thread, audio_thread_proc, pTaskVideo);
	}
}
#endif

#if CNT_FPS
void CTaskVideo::Process(void)
{
	while(mRunning)
	{
		LOG_DEBUG("main: %d, sub1: %d, sub2: %d, sub3: %d\n", 
					mMainCnt, mSub1Cnt, mSub2Cnt, mSub3Cnt);
		mMainCnt = mSub1Cnt = mSub2Cnt = mSub3Cnt = 0;
		sleep(1);
	}
}
#endif

void *EventProc(void *pArg)
{
	int new_fd, listen_fd, addr_len;
	struct sockaddr_in remode_addr;	
	EVENT_CONNECT_SESSION *pSession = NULL;
	CTaskVideo *pTaskVideo = (CTaskVideo *)pArg;

	listen_fd = tcp_create_and_listen(EVENT_PORT);
	if (listen_fd < 0)
	{
		LOG_ERROR("tcp_create_and_listen fail\n");
		return NULL;
	}
	
	while (pTaskVideo->mRunning)
	{	
		new_fd = tcp_block_accept(listen_fd, (struct sockaddr *)&remode_addr, &addr_len);
		if (new_fd >= 0)
		{
			set_sock_noblock(new_fd);
			pSession = (EVENT_CONNECT_SESSION *)calloc(1, sizeof(EVENT_CONNECT_SESSION));
			memset(pSession, 0, sizeof(EVENT_CONNECT_SESSION));
			pSession->sockfd = new_fd;	
			list_add_tail(&pSession->list, &pTaskVideo->mEventSessionList);
		}
		else
		{
			LOG_ERROR("eventproc listen error\n");
			continue;
		}
	}
	
	return NULL;
}

PIC_SIZE_E ResolutionChange(int resolution)
{
	PIC_SIZE_E emPicSize = PIC_BUTT;
	
	switch (resolution)
	{
		case P1080:
			emPicSize = PIC_HD1080;
			break;
		case P720:
			emPicSize = PIC_HD720;
			break;
		case VGA:
			emPicSize = PIC_VGA;
			break;
		case QVGA:
			emPicSize = PIC_QVGA;			
			break;
	}
	return emPicSize;
}

int CTaskVideo::Init(void)
{	
//	RAM_PARAM *pRamParam = get_ram_param();
//	MISC_PARAM *pMiscParam = get_misc_param();	
//	PIC_SIZE_E enVencSize[2] = {PIC_HD720, PIC_VGA};	

//
//	mpRamParam   = get_ram_param();	
//	mVideoParam  = get_video_param();  	

	LOG_INFO("Ready into SAMPLE_VENC_720P_CLASSIC\n");
	SAMPLE_VENC_720P_CLASSIC();
	//mpRamParam->mpp_init_flag = 1;

	//enVencSize[0] = ResolutionChange(mVideoParam->stream[0].resolution);
	//enVencSize[1] = ResolutionChange(mVideoParam->stream[1].resolution);	
	//hi_set_venc_resolution(enVencSize, sizeof(enVencSize) / sizeof(enVencSize[0]));
	//hi_osd_showname("nihao");/*app solve */
	//LOG_INFO("Ready into hi_set_stream_cb\n");
	hi_set_stream_cb(put_stream_cb, this);

	//INIT_LIST_HEAD(&mEventSessionList);
	//CreateDetachedTask(&mEventId, EventProc, this);
	//md表示move detect 移动侦测
	//hi_md_init();					
	//hi_SetResultCB(md_result_cb, this);
#if 0
#if 0
	hi_set_bitrate(0, mVideoParam->stream[0].bitrate);
	hi_set_bitrate(1, mVideoParam->stream[1].bitrate);	
#else
//	hi_set_bitrate(0, 1024);
#endif
#if 0	
	hi_set_fps(0, mVideoParam->stream[0].fps);
	hi_set_fps(1, mVideoParam->stream[1].fps);		
#else
//	hi_set_fps(0, 15);
#endif	
#endif
	//set_csc_attr(ATTR_BRIGHTNESS, mVideoParam->brightness);
	//set_csc_attr(ATTR_CONTRAST, mVideoParam->contrast);
	//set_csc_attr(ATTR_HUE, mVideoParam->hue);
	//set_csc_attr(ATTR_SATUATURE, mVideoParam->saturation);	
	//set_rotate(mVideoParam->flip);
	//if (pMiscParam->pwr_freq == F50HZ)
	//	hi_set_pwrfreq(50);
	//else if (pMiscParam->pwr_freq == F60HZ)
	//	hi_set_pwrfreq(60);

	return SDK_OK;
}

void CTaskVideo::Uninit(void)
{
	pthread_join(mEventId, NULL);
	mpRamParam->mpp_init_flag = 0;
}


int video_task_create(void)
{
	return CTaskVideo::getInstance()->Create();
}

void video_task_destory(void)
{
	return CTaskVideo::getInstance()->Destory();
}

