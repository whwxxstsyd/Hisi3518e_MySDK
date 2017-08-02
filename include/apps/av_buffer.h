#ifndef	__AV_BUFFER_H__
#define	__AV_BUFFER_H__

#ifdef	__cplusplus
extern "C" {
#endif	

#include "common.h"

#include <stdlib.h>

/* AV_BUFFER_FRAME只在av_buffer内部使用 */
typedef struct DATA_FRAME_T
{
    int magic_num;
    int type;
    int len;
    char *data;     /* read_frame使用该字段 */
    int pos;        
}DATA_FRAME;

/* 1秒内sample个数 */
#define     I_FRAME_PER_SEC         (2)
#define     P_FRAME_PER_SEC         (30)    /* 改成20 onvif无法输出视频 */
#define     AUDIO_FRAME_PER_SEC     (100)    /* PCM and G726 */
#define     BACK_FRAME_CNT           (1)    /* 备用帧缓冲空间 */

typedef enum FRAME_INFO_VALUE_T
{
/* 实际分配的内存要大于缓冲中的帧数,确保帧队列中的帧,都有内存 */    
	I_FRAME_COUNT = (I_FRAME_PER_SEC*12/10),
	P_FRAME_COUNT = (P_FRAME_PER_SEC*12/10),
	AUDIO_FRAME_COUNT = (AUDIO_FRAME_PER_SEC*12/10),          
	
	FRAME_COUNT = (I_FRAME_PER_SEC + P_FRAME_PER_SEC + AUDIO_FRAME_PER_SEC),    /* 大于1S(留有余量) */
	I_FRAME_SIZE = (MAX_FRAME_SIZE * 6 / 10),   /* 60% 最大帧 */
	P_FRAME_SIZE = (60 * 1024),
	AUDIO_FRAME_SIZE = 1024,
	VGA_JPG_SIZE = (200 * 1024),
}FRAME_INFO_VALUE;

#define     FIRST_FRAME       \
	((mBufferParam->last_frame != 0) ? (mBufferParam->last_frame - 1) : (FRAME_COUNT - 1))
#define		LAST_FRAME		(mBufferParam->last_frame)	

/* 每个队列多申请一个内存块,避免内存越界 */
#define		BUFFER_SIZE			\
					((I_FRAME_SIZE * (I_FRAME_COUNT + BACK_FRAME_CNT)) + 	        \
					(P_FRAME_SIZE * (P_FRAME_COUNT + BACK_FRAME_CNT)) + 	            \
					(AUDIO_FRAME_SIZE * (AUDIO_FRAME_COUNT + BACK_FRAME_CNT)) +      \
					VGA_JPG_SIZE)
#define		I_BUFFER_OFFSET		\
					(sizeof(BUFFER_PARAM))
#define		P_BUFFER_OFFSET		\
					(I_BUFFER_OFFSET + (I_FRAME_SIZE * (I_FRAME_COUNT + 1)))
#define		AUDIO_BUFFER_OFFSET		\
					(P_BUFFER_OFFSET + (P_FRAME_SIZE * (P_FRAME_COUNT + 1)))	
#define     VGA_JPG_BUFFER_OFFSET   \
                    (AUDIO_BUFFER_OFFSET + (AUDIO_FRAME_SIZE * (AUDIO_FRAME_COUNT + 1)))

typedef struct BUFFER_PARAM_T
{
    int magic;
    int last_frame;
    DATA_FRAME frame[FRAME_COUNT];
	int last_i;
    char *i_frame_data;
	int last_p;	
    char *p_frame_data;
	int last_audio;	
    char *audio_frame_data; 
    int init_flag;
}BUFFER_PARAM;

#ifdef	__cplusplus
class CAvBuffer     /* 单例模式不能调用new创建新对象,线程安全 */
{
    public:     
        static CAvBuffer& getInstance()
        {
            static CAvBuffer mInstance;

            return mInstance;
        }
		CAvBuffer();		
		~CAvBuffer();
        int init_ipc(void);
        int init_param(void);        
        int Init(void);
        void Uninit(void); 
        DATA_FRAME *change_data(DATA_FRAME *pDataFrame);       
        DATA_FRAME *GetFreeFrame(void);
        void GetBuffer(DATA_FRAME *pDataFrame, int unit);          
		DATA_FRAME *GetFreeIFrameBuffer(int unit);
		DATA_FRAME *GetFreePFrameBuffer(int unit);
		DATA_FRAME *GetFreeAudioFrameBuffer(int unit);
        void write_frame(int type, char *data, int len);
        int read_frame(int type, REAL_FRAME *real_frame); 
        int read_frame2(int type, REAL_FRAME *real_frame, int key_frame); 
        int read_frame(int type, REAL_FRAME *real_frame, int key_frame);       
        int write_vga_pic(char *data, int len);
        int read_vga_pic(char *data);    
        void dump_buffer(void);
        void clear_sem(void);        

	private:
        struct BUFFER_PARAM_T *mBufferParam;
		char *mIFrameData;
		char *mPFrameData;
		char *mAudioFrameData;
        char *mVgaJpgData;
    	int  mShmID;        
		int  mSemID;
		int  mVgaSemID;  
        int  mInit;
        pthread_t mFrameCntId;

    public:
        int FrameCntRun;        
        int mICnt;
        int mPCnt;
        int mAudioCnt;   
};
#endif

int av_buffer_init(void);
void av_buffer_uninit(void);
void av_write_frame(int type, char *data, int len);
int av_read_frame(int type, REAL_FRAME *real_frame);
int av_read_frame2(int type, REAL_FRAME *real_frame, int key_frame);
int av_read_frame_ext(int type, REAL_FRAME *real_frame, int key_frame);
int av_write_jpg(char *data, int len);
int av_read_jpg(char *data);
void av_dump_buffer(void);
void av_clear_sem(void);
int audio_playback(unsigned char *data, int len);
int play_audio_file(const char *pPath);

#ifdef	__cplusplus
}
#endif	

#endif
