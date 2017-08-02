#include "debug.h"
#include "param.h"
#include "av_buffer.h"
#include "normal.h"
#include "sem.h"

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#undef		TARGET_LEVEL
#define		TARGET_LEVEL		LEVEL_INFO

#define		ENABLE_BUFFER_CNT		0

CAvBuffer::CAvBuffer()
{
	mBufferParam = NULL;
	mIFrameData = NULL;
	mPFrameData = NULL;
	mAudioFrameData = NULL;
	mSemID = -1;
	mVgaSemID = -1;  
    mICnt = 0;
    mPCnt = 0;
    mAudioCnt = 0; 
	mInit = 0;
}

CAvBuffer::~CAvBuffer()
{

}

void *cnt_proc(void *arg)
{
	CAvBuffer *pAvBuffer = (CAvBuffer *)arg;

	pAvBuffer->FrameCntRun = 1;
	while (pAvBuffer->FrameCntRun)
	{
		LOG_INFO("i_p_audio: [%d_%d_%d]\n", pAvBuffer->mICnt, pAvBuffer->mPCnt, pAvBuffer->mAudioCnt);
		pAvBuffer->mICnt = 0;
		pAvBuffer->mPCnt = 0;
		pAvBuffer->mAudioCnt = 0;		
		sleep(1);
	}

	return NULL;
}

int CAvBuffer::init_param(void) /* 整个系统中只初始化一次 */
{	
	GetSem(mSemID);				/* 互斥访问,避免重新初始化 */
	
	if (mBufferParam->init_flag == 0)
	{
		mBufferParam->magic = 0;
		mBufferParam->last_frame = -1;		/* 设为-1, 避免第0帧为空帧 */
		mBufferParam->last_i = 0;
		mBufferParam->last_p = 0;	
		mBufferParam->last_audio = 0;
		memset(mBufferParam->frame, 0, sizeof(mBufferParam->frame));
#if ENABLE_BUFFER_CNT		
		CreateDetachedTask(&mFrameCntId, cnt_proc, this);
#endif	
		mBufferParam->init_flag = 1;		

		LOG_INFO("AV buffer init ok\n");
	}
	else
		LOG_INFO("AV buffer has init\n");

	PostSem(mSemID);	

	return SDK_OK;
}

int CAvBuffer::init_ipc(void)	/* 在每个进程中初始化一次 */
{
	/* for share memory (在申请共享内存是已经被初始化为0了) */
	mShmID = shmget(KEY_SHM_BUFFER, sizeof(BUFFER_PARAM) + BUFFER_SIZE, IPC_CREAT | 0666);
    if (mShmID < 0)
    {
        LOG_ERROR("shmget error\n");
		return SDK_ERROR;
    }
	LOG_INFO("create share memory size: %d\n", BUFFER_SIZE);
	
    mBufferParam = (BUFFER_PARAM *)shmat(mShmID, (void *)0, 0);
    if (mBufferParam == NULL)
    {
        LOG_ERROR("shmat error\n");
		return SDK_ERROR;		
	}	
	mIFrameData = (char *)mBufferParam + I_BUFFER_OFFSET;
	mPFrameData = (char *)mBufferParam + P_BUFFER_OFFSET;
	mAudioFrameData = (char *)mBufferParam + AUDIO_BUFFER_OFFSET;
	mVgaJpgData = (char *)mBufferParam + VGA_JPG_BUFFER_OFFSET;	

	/* for share memory sem */
	mSemID = CreateSem(KEY_SEM_BUFFER);
    if (mSemID < 0)
    {
        LOG_ERROR("CreateSem error\n");
		return SDK_ERROR;		
    }
	mVgaSemID = CreateSem(KEY_SEM_VGA_BUFFER);
    if (mVgaSemID < 0)
    {
        LOG_ERROR("CreateSem error\n");
		return SDK_ERROR;		
    }

	return SDK_OK;
}

int CAvBuffer::Init(void)
{
	if (mInit)
		return 0;
	
	init_ipc();
	init_param();

	mInit = 1;

	return SDK_OK;
}

void CAvBuffer::Uninit(void)
{
	if (!mInit)
		return;
	
#if ENABLE_BUFFER_CNT
	FrameCntRun = 0;
	pthread_join(mFrameCntId, NULL);
#endif
	shmdt(mBufferParam);		/* 分离共享内存 */
	//shmctl(mShmID, NULL, 0);		/* 释放共享内存 */
	mBufferParam = NULL;
	UninitSem(mSemID);
	UninitSem(mVgaSemID);
	mBufferParam->init_flag = 0;	
	mBufferParam->magic = 0;

	mInit = 0;
}

DATA_FRAME *CAvBuffer::change_data(DATA_FRAME *pDataFrame)
{
	switch (pDataFrame->type)
	{
		case FRAME_TYPE_IDR:
		case FRAME_TYPE_SUB1_IDR:
		case FRAME_TYPE_SUB2_IDR:
		case FRAME_TYPE_SUB3_IDR:
		case FRAME_TYPE_I:
		case FRAME_TYPE_SUB1_I:
		case FRAME_TYPE_SUB2_I:
		case FRAME_TYPE_SUB3_I:	
		case FRAME_TYPE_MJPEG:
		case FRAME_TYPE_SUB1_MJPEG:
		case FRAME_TYPE_SUB2_MJPEG:
		case FRAME_TYPE_SUB3_MJPEG:
			//存放I 帧的地址:包含了数据头部
			pDataFrame->data = mIFrameData + pDataFrame->pos;
			break;
		case FRAME_TYPE_P:
	    case FRAME_TYPE_SUB1_P:
	    case FRAME_TYPE_SUB2_P:
	    case FRAME_TYPE_SUB3_P:
		case FRAME_TYPE_B:
	    case FRAME_TYPE_SUB1_B:
	    case FRAME_TYPE_SUB2_B:
	    case FRAME_TYPE_SUB3_B:			
			//存放P 帧的地址:包含了数据头部
			pDataFrame->data = mPFrameData + pDataFrame->pos;		
			break;
		case FRAME_TYPE_NEAR_G726:							
		case FRAME_TYPE_FAR_G726:				
		case FRAME_TYPE_NEAR_G711:			
		case FRAME_TYPE_FAR_G711:
		case FRAME_TYPE_NEAR_ADPCM:	
		case FRAME_TYPE_FAR_ADPCM:	
		case FRAME_TYPE_NEAR_AAC:				
		case FRAME_TYPE_NEAR_PCM:				
			pDataFrame->data = mAudioFrameData + pDataFrame->pos;		
			break;
	}

	return pDataFrame;
}

/* 读操作太快,直接遍历到最后一帧 */
/* 对操作太慢,找不到原始帧,取最新的IDR */
int CAvBuffer::read_frame(int type, REAL_FRAME *real_frame)
{
    int i, magic_diff;
	int cnt = GET_FRAME_TIMEOUT_COUNT;		/* 2s 超时 */
	if (mBufferParam->init_flag == 0)
	{
		LOG_INFO("av_buffer uninit\n");
		return -1;
	}
	while (1)
	{
		if (GetSem(mSemID) < 0)
			return -1;
	    magic_diff = mBufferParam->magic - real_frame->magic;
	    if (magic_diff > FRAME_COUNT)						/* 找不到旧帧(传第一帧 或者 读取帧率太慢) */
	    {			
	        i = mBufferParam->last_frame;
	        for ( ; i >= 0; i--)
	            if (mBufferParam->frame[i].type & type)
					goto GET_FRAME;					
		    for (i = (FRAME_COUNT - 1); i > mBufferParam->last_frame; i--)
		        if (mBufferParam->frame[i].type & type)
					goto GET_FRAME;
	    }
	    else if (magic_diff > LAST_FRAME)  					/* 旧帧在最大帧右边 */
	    {
	  		i = FRAME_COUNT - magic_diff + LAST_FRAME + 1; 	/* 旧帧位置 */
	        for ( ; i < FRAME_COUNT; i++)
	            if (mBufferParam->frame[i].type & type)
					goto GET_FRAME;
		    for (i = 0; i <= LAST_FRAME; i++)
		        if (mBufferParam->frame[i].type & type)
					goto GET_FRAME;
	    }
		else if (magic_diff <= LAST_FRAME)						/* 旧帧在最大帧左边 */
		{
			i = LAST_FRAME - magic_diff + 1;						/* 旧帧位置 */	
			for ( ; i <= LAST_FRAME; i++)
		        if (mBufferParam->frame[i].type & type)
					goto GET_FRAME;
		}
		PostSem(mSemID);
		usleep(RETRY_DIFF_MS * 1000);							/* 读太快,遍历到列表尾部 */
		if (--cnt == 0)
			return -1;											/* video src loss, 视频源丢失 */
	}

GET_FRAME:
	change_data(&(mBufferParam->frame[i])); 		   
	real_frame->magic = mBufferParam->frame[i].magic_num;	
	real_frame->type = mBufferParam->frame[i].type;
	real_frame->len = mBufferParam->frame[i].len;
	if (real_frame->type == 0x10)
		LOG_WARN("magic: %d, len: %d, type: 0x%x\n", real_frame->magic, real_frame->len, real_frame->type);
	memcpy(real_frame->data, mBufferParam->frame[i].data, real_frame->len);
	PostSem(mSemID);
	LOG_DEBUG("read_frame magic: %d\n", real_frame->magic);
	
	return real_frame->len;
}

int CAvBuffer::read_frame2(int type, REAL_FRAME *real_frame, int key_frame)
{
	int i, magic_diff;
	int cnt = GET_FRAME_TIMEOUT_COUNT;		/* 2s 超时 */
	int firsttype = FRAME_TYPE_IDR;

	int last_frame = mBufferParam->last_frame;
	
	if (mBufferParam->init_flag == 0)
	{
		LOG_INFO("av_buffer uninit\n");
		return -1;
	}

	while (1)
	{
		if (GetSem(mSemID) < 0)
		return -1;
		magic_diff = mBufferParam->magic - real_frame->magic;
		if ((magic_diff > FRAME_COUNT) || (key_frame))						/* 找不到旧帧(传第一帧 或者 读取帧率太慢) */
		{
			//找到BUF中已存的第一个IDR帧
			i = mBufferParam->last_frame;			
			for ( ; i >= 0; i--)
				if (mBufferParam->frame[i].type & firsttype)
					goto GET_FRAME;	
			for (i = (FRAME_COUNT - 1); i > mBufferParam->last_frame; i--)
				if (mBufferParam->frame[i].type & firsttype)
					goto GET_FRAME;
		}
		else if (magic_diff > last_frame)  					/* 旧帧在最大帧右边 */
		{
			i = FRAME_COUNT - magic_diff + last_frame + 1; 	/* 旧帧位置 */
			for ( ; i < FRAME_COUNT; i++)
				if (mBufferParam->frame[i].type & type)
					goto GET_FRAME;
			for (i = 0; i <= last_frame; i++)
				if (mBufferParam->frame[i].type & type)
					goto GET_FRAME;
		}
		else if (magic_diff <= last_frame)						/* 旧帧在最大帧左边 */
		{
			i = last_frame - magic_diff + 1;						/* 旧帧位置 */	
			for ( ; i <= last_frame; i++)
			if (mBufferParam->frame[i].type & type)
			goto GET_FRAME;
		}
		PostSem(mSemID);
		usleep(RETRY_DIFF_MS * 1000);							/* 读太快,遍历到列表尾部 */
		if (--cnt == 0)
		return -1;											/* video src loss, 视频源丢失 */
	}

	GET_FRAME:
	change_data(&(mBufferParam->frame[i])); 		   
	real_frame->magic = mBufferParam->frame[i].magic_num;	
	real_frame->type = mBufferParam->frame[i].type;
	real_frame->len = mBufferParam->frame[i].len;
	if (real_frame->type == 0x10)
		LOG_WARN("magic: %d, len: %d, type: 0x%x\n", real_frame->magic, real_frame->len, real_frame->type);
	memcpy(real_frame->data, mBufferParam->frame[i].data, real_frame->len);
	PostSem(mSemID);
	LOG_DEBUG("read_frame magic: %d\n", real_frame->magic);
	return real_frame->len;
}



//FILE *pFile1 = fopen("read_frame.h264", "wb");

/* 读操作太快,直接遍历到最后一帧 */
/* 对操作太慢,找不到原始帧,取最新的IDR */
int CAvBuffer::read_frame(int type, REAL_FRAME *real_frame, int key_frame)
{
    int i, magic_diff;
	int cnt = GET_FRAME_TIMEOUT_COUNT;		/* 2s 超时 */
	
	if (mBufferParam->init_flag == 0)
	{
		LOG_INFO("av_buffer uninit\n");
		return -1;
	}
	while (1)
	{
		if (GetSem(mSemID) < 0)
			return -1;
	    magic_diff = mBufferParam->magic - real_frame->magic;
	    if (magic_diff > FRAME_COUNT)						/* 找不到旧帧(传第一帧 或者 读取帧率太慢) */
	    {			
	    	if (key_frame && (type & IDR_MASK))					/* 优先取IDR */
				type = (type & IDR_MASK);	
	        i = mBufferParam->last_frame;
	        for ( ; i >= 0; i--)
	            if (mBufferParam->frame[i].type & type)
					goto GET_FRAME;					
		    for (i = (FRAME_COUNT - 1); i > mBufferParam->last_frame; i--)
		        if (mBufferParam->frame[i].type & type)
					goto GET_FRAME;
	    }
	    else if (magic_diff > LAST_FRAME)  					/* 旧帧在最大帧右边 */
	    {
	  		i = FRAME_COUNT - magic_diff + LAST_FRAME + 1; 	/* 旧帧位置 */
	        for ( ; i < FRAME_COUNT; i++)
	            if (mBufferParam->frame[i].type & type)
					goto GET_FRAME;
		    for (i = 0; i <= LAST_FRAME; i++)
		        if (mBufferParam->frame[i].type & type)
					goto GET_FRAME;
	    }
		else if (magic_diff <= LAST_FRAME)						/* 旧帧在最大帧左边 */
		{
			i = LAST_FRAME - magic_diff + 1;						/* 旧帧位置 */	
			for ( ; i <= LAST_FRAME; i++)
		        if (mBufferParam->frame[i].type & type)
					goto GET_FRAME;
		}

		PostSem(mSemID);
		usleep(RETRY_DIFF_MS * 1000);							/* 读太快,遍历到列表尾部 */
		cnt--; 
		if (cnt == 0)
			return -1;											/* video src loss, 视频源丢失 */
	}

GET_FRAME:
	change_data(&(mBufferParam->frame[i]));
	//fwrite(mBufferParam->frame[i].data,mBufferParam->frame[i].len,1,pFile1);
	real_frame->magic = mBufferParam->frame[i].magic_num;	
	real_frame->type = mBufferParam->frame[i].type;
	real_frame->len = mBufferParam->frame[i].len;	
	
	memcpy(real_frame->data, mBufferParam->frame[i].data, real_frame->len);
	
	PostSem(mSemID);
	return real_frame->len;
}

/* 从FrameList中找出Frame，此时还没分配内存 */
DATA_FRAME *CAvBuffer::GetFreeFrame(void)	
{
	DATA_FRAME *pDataFrame;

	mBufferParam->last_frame++;
	mBufferParam->last_frame = mBufferParam->last_frame % FRAME_COUNT;	
	pDataFrame = &(mBufferParam->frame[mBufferParam->last_frame]);

    return pDataFrame;
}

void CAvBuffer::GetBuffer(DATA_FRAME *pDataFrame, int unit)	/* unit:获取几个单位的空间(只支持1-2) */
{
	switch (pDataFrame->type)
	{
		case FRAME_TYPE_IDR:
		case FRAME_TYPE_SUB1_IDR:
		case FRAME_TYPE_SUB2_IDR:
		case FRAME_TYPE_SUB3_IDR:
		case FRAME_TYPE_I:
		case FRAME_TYPE_SUB1_I:
		case FRAME_TYPE_SUB2_I:
		case FRAME_TYPE_SUB3_I:	
		case FRAME_TYPE_MJPEG:
		case FRAME_TYPE_SUB1_MJPEG:
		case FRAME_TYPE_SUB2_MJPEG:
		case FRAME_TYPE_SUB3_MJPEG:			
			pDataFrame->pos = I_FRAME_SIZE * mBufferParam->last_i;		
			if ((mBufferParam->last_i + unit) >= I_FRAME_COUNT)
				mBufferParam->last_i = 0;
			else
				mBufferParam->last_i += unit;		
			break;
		case FRAME_TYPE_P:
	    case FRAME_TYPE_SUB1_P:
	    case FRAME_TYPE_SUB2_P:
	    case FRAME_TYPE_SUB3_P:
		case FRAME_TYPE_B:
	    case FRAME_TYPE_SUB1_B:
	    case FRAME_TYPE_SUB2_B:
	    case FRAME_TYPE_SUB3_B:			
			pDataFrame->pos = P_FRAME_SIZE * mBufferParam->last_p;		
			if ((mBufferParam->last_p + unit) >= P_FRAME_COUNT)
				mBufferParam->last_p = 0;
			else
				mBufferParam->last_p += unit;
			break;
		case FRAME_TYPE_NEAR_G726:							
		case FRAME_TYPE_FAR_G726:				
		case FRAME_TYPE_NEAR_G711:			
		case FRAME_TYPE_FAR_G711:
		case FRAME_TYPE_NEAR_ADPCM:	
		case FRAME_TYPE_FAR_ADPCM:	
		case FRAME_TYPE_NEAR_AAC:	
		case FRAME_TYPE_NEAR_PCM:				
			pDataFrame->pos = AUDIO_FRAME_SIZE * mBufferParam->last_audio;
			if ((mBufferParam->last_audio + unit) >= AUDIO_FRAME_COUNT)
				mBufferParam->last_audio = 0;
			else
				mBufferParam->last_audio += unit;			
			break;
	}
}

DATA_FRAME *CAvBuffer::GetFreeIFrameBuffer(int unit)
{
	DATA_FRAME *pDataFrame;

	pDataFrame = GetFreeFrame();			/* 从数组中获取帧 */
	pDataFrame->type = FRAME_TYPE_I;
	GetBuffer(pDataFrame, unit);
	
    return pDataFrame; 
}

DATA_FRAME *CAvBuffer::GetFreePFrameBuffer(int unit)
{
	DATA_FRAME *pDataFrame;

	pDataFrame = GetFreeFrame();//保存了P帧全局的地址
	pDataFrame->type = FRAME_TYPE_P;	
	GetBuffer(pDataFrame, unit);
	//pDataFrame->pos = I_FRAME_SIZE * mBufferParam->last_i;
	
    return pDataFrame; 
}

DATA_FRAME *CAvBuffer::GetFreeAudioFrameBuffer(int unit)
{
	DATA_FRAME *pDataFrame;

	mAudioCnt++;
	pDataFrame = GetFreeFrame();
	pDataFrame->type = FRAME_TYPE_NEAR_ADPCM;		
	GetBuffer(pDataFrame, unit);
	
    return pDataFrame; 
}

/* 帧头在获取帧的位置添加 */
void CAvBuffer::write_frame(int type, char *data, int len)
{
	DATA_FRAME *pDataFrame = NULL;
	FRAME_HEAD *head = (FRAME_HEAD *)data;
	int mem_unit = -1;		/* 存储单元的个数 */	

	if (mBufferParam->init_flag == 0)
	{
		LOG_INFO("av_buffer uninit\n");
		return;
	}

	GetSem(mSemID);
	switch (type)
	{
		case FRAME_TYPE_IDR:
		case FRAME_TYPE_SUB1_IDR:
		case FRAME_TYPE_SUB2_IDR:
		case FRAME_TYPE_SUB3_IDR:
		case FRAME_TYPE_I:
		case FRAME_TYPE_SUB1_I:
		case FRAME_TYPE_SUB2_I:
		case FRAME_TYPE_SUB3_I:	
		case FRAME_TYPE_MJPEG:
		case FRAME_TYPE_SUB1_MJPEG:
		case FRAME_TYPE_SUB2_MJPEG:
		case FRAME_TYPE_SUB3_MJPEG:	
			mem_unit = ROUND_UP(len, I_FRAME_SIZE);
			if (mem_unit > (BACK_FRAME_CNT + 1))
			{
				LOG_WARN("!!! i frame size too big[%d:%d] mem_unit[%d] !!!\n", len, I_FRAME_SIZE, mem_unit);
				PostSem(mSemID);
				return;
			}
			pDataFrame = GetFreeIFrameBuffer(mem_unit);
			memcpy(mIFrameData + pDataFrame->pos , data, len);		
			break;
		case FRAME_TYPE_P:
	    case FRAME_TYPE_SUB1_P:
	    case FRAME_TYPE_SUB2_P:
	    case FRAME_TYPE_SUB3_P:
		case FRAME_TYPE_B:
	    case FRAME_TYPE_SUB1_B:
	    case FRAME_TYPE_SUB2_B:
	    case FRAME_TYPE_SUB3_B:	
			mem_unit = ROUND_UP(len, P_FRAME_SIZE);		
			if (mem_unit > (BACK_FRAME_CNT + 1))
			{
				LOG_WARN("!!! p frame size too big[%d:%d] mem_unit[%d] !!!\n", len, P_FRAME_SIZE, mem_unit);
				PostSem(mSemID);
				return;
			}
			pDataFrame = GetFreePFrameBuffer(mem_unit);
			//pDataFrame = &(mBufferParam->frame[mBufferParam->last_frame]);
			memcpy(mPFrameData + pDataFrame->pos , data, len);		
			break;
		case FRAME_TYPE_NEAR_G726: 						 
		case FRAME_TYPE_FAR_G726:				 
		case FRAME_TYPE_NEAR_G711: 		 
		case FRAME_TYPE_FAR_G711:
		case FRAME_TYPE_NEAR_ADPCM:  
		case FRAME_TYPE_FAR_ADPCM:	
		case FRAME_TYPE_NEAR_AAC:	
		case FRAME_TYPE_NEAR_PCM:	
			mem_unit = ROUND_UP(len, AUDIO_FRAME_SIZE);	
			if (mem_unit > (BACK_FRAME_CNT + 1))
			{
				LOG_WARN("!!! audio frame size too big[%d:%d] mem_unit[%d] !!!\n", len, AUDIO_FRAME_SIZE, mem_unit);
				PostSem(mSemID);
				return;
			}
			pDataFrame = GetFreeAudioFrameBuffer(mem_unit);
			memcpy(pDataFrame->pos + mAudioFrameData, data, len);			 			 
			break;
		default:
			break;
	}
	mBufferParam->magic++;
	if (pDataFrame)
	{
		pDataFrame->magic_num = mBufferParam->magic;	
		pDataFrame->type = type;
		pDataFrame->len = len;
	}
    //LOG_INFO("magic = %d,mICnt = %d,mPCnt= %d\n",mBufferParam->magic,mICnt,mPCnt);
	PostSem(mSemID);
}

/* 添加四个字节的头 */
int CAvBuffer::write_vga_pic(char *data, int len)
{	
	GetSem(mVgaSemID);
	
	if (len < (int)VGA_JPG_SIZE)
	{
		memcpy(mVgaJpgData, data, len);
		LOG_DEBUG("vga pic len[%d]\n", len);
	}
	else
		LOG_WARN("VGA pic to big [%d:%d]\n", len, VGA_JPG_SIZE);
	
	PostSem(mVgaSemID);

	return 0;
}

void CAvBuffer::dump_buffer(void)
{
	int i;
	
	GetSem(mVgaSemID);
	for (i = 0; i < FRAME_COUNT; i++)
	{
		LOG_INFO("frame[%d]: magic[%d], type[0x%x], len[%d]\n", i,
					mBufferParam->frame[i].magic_num,
					mBufferParam->frame[i].type,
					mBufferParam->frame[i].len);				
	}
	PostSem(mVgaSemID);	
}

void CAvBuffer::clear_sem(void)
{
	CleanSem(mSemID);
}

int CAvBuffer::read_vga_pic(char *data)
{
	int len = 0;
	FRAME_HEAD FrameHead;
	
	GetSem(mVgaSemID);

	memcpy(&FrameHead, mVgaJpgData, FRAME_HEAD_LEN);
	len = FrameHead.len;
	memcpy(data, mVgaJpgData, FrameHead.len + FRAME_HEAD_LEN);
	
	PostSem(mVgaSemID);

	return len;
}

int av_buffer_init(void)
{
	return CAvBuffer::getInstance().Init();
}

void av_buffer_uninit(void)
{
	return CAvBuffer::getInstance().Uninit();
}
	
void av_write_frame(int type, char *data, int len)
{
	
	return CAvBuffer::getInstance().write_frame(type, data, len);
}

int av_read_frame(int type, REAL_FRAME *real_frame)
{
	return CAvBuffer::getInstance().read_frame(type, real_frame);
}

int av_read_frame2(int type, REAL_FRAME *real_frame, int key_frame)
{
	return CAvBuffer::getInstance().read_frame2(type, real_frame, key_frame);
}

int av_read_frame_ext(int type, REAL_FRAME *real_frame, int key_frame)
{
	return CAvBuffer::getInstance().read_frame(type, real_frame, key_frame);
}

int av_write_jpg(char *data, int len)
{
	return CAvBuffer::getInstance().write_vga_pic(data, len);
}

int av_read_jpg(char *data)
{
	return CAvBuffer::getInstance().read_vga_pic(data);
}

void av_dump_buffer(void)
{
	return CAvBuffer::getInstance().dump_buffer();
}

void av_clear_sem(void)
{
	return CAvBuffer::getInstance().clear_sem();
}

int audio_playback(unsigned char *data, int len)
{
	static int fifo = -1;
	int ret;

	if (fifo == -1)
	{
		fifo = open(G711_AUDIO_FIFO, O_WRONLY | O_CREAT);
		if (fifo < 0)
		{
			LOG_ERROR("open fail\n");
			return -1;
		}
	}
	if (fifo < 0)
	{		
		LOG_ERROR("file fd error\n");
		fifo = -1;
		return -1;
	}
	ret = write(fifo, data, len);
	if (ret != len)
	{
		LOG_ERROR("write fail[%d]\n", ret);
		close(fifo);
		fifo = -1;
		return -1;
	}
//	close(fifo);		/* some bug? ignore */
	
	return 0;
}

int play_audio_file(const char *pPath)
{
	WAV_HEAD Header;	
	FILE *file = NULL;
	unsigned char buf[160] = {0};
	file = fopen(pPath, "rb");
	if (file)
	{
		fread(&Header, 1, sizeof(WAV_HEAD), file);
		while (!feof(file))
		{
			fread(buf, 1, sizeof(buf), file);		// Reading data in chunks of BUFSIZE			
			if (audio_playback(buf, sizeof(buf)) < 0)
			{
				LOG_ERROR("audio_playback fail\n");
				fclose(file);
				return -1;
			}		
		}
	}
	else
	{
		LOG_ERROR("open file[%s] fail\n", pPath);
		return -1;
	}
	fclose(file);

	return 0;
}

