#ifndef      __COMMON_H__
#define     __COMMON_H__

#ifdef	__cplusplus
extern "C" { 
#endif

#ifdef	__SDK__	        /* linux sdk使用 */
#define     SENSOR_OV9712
#define     MCU_HI3518EV100
#endif

#ifdef	WIN32           /* WIN32 sdk使用 */
#ifndef     SENSOR_OV9712
#define     SENSOR_OV9712
#endif

#ifndef     MCU_HI3518EV100 
#define     MCU_HI3518EV100
#endif
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef		void *(*THREAD_ENTER)(void *);
typedef     int (*SET_STRING_CB)(const char *key_word, const char *value);
typedef     int (*SET_INT_CB)(const char *key_word, int value);
typedef     int (*SET_STRING_CB_EXT)(const char *key_word, const char *value, char *out_str);
typedef     int (*SET_INT_CB_EXT)(const char *key_word, int value, char *out_str);
typedef     int (*CMD_PROC_CB)(const char *key_word, const char *arg, char *out_str);

/* cgi_server 外部接口 */
typedef int (*SET_PARAM_STR_FUNC)(const char *key_word, const char *value);
typedef int (*SET_PARAM_INT_FUNC)(const char *key_word, int value);
typedef int (*GET_PARAM_FUNC)(const char *key_word, char *value);
typedef int (*CMD_PROC_FUNC)(const char *key_word, const char *arg, char *out_str);

typedef enum PARAM_TYPE_T
{
    TYPE_STR,
    TYPE_INT,
    TYPE_INT_STR,
    TYPE_CMD,
}PARAM_TYPE;

typedef enum STR_LEN_T
{
    STR_16_LEN = 16,
    STR_32_LEN = 32,
    STR_64_LEN = 64,
    STR_128_LEN = 128,
    STR_256_LEN = 256,    
    STR_512_LEN = 512,
    STR_1024_LEN = 1024,     
}STR_LEN;

typedef enum SDK_RESULT_CODE_T
{
    ERROR_BASE                  = ((int)(0xF0000000)),
	NO_AVAILABLE_CONNECTION     = (ERROR_BASE | 0x01),       /* 连接数超出范围 */    
    NO_MEMORY                   = (ERROR_BASE | 0x02),
    BAD_MARK                    = (ERROR_BASE | 0x03),
    NET_ERROR                   = (ERROR_BASE | 0x04),
    VALUE_OUT_OF_RANGE          = (ERROR_BASE | 0x05),       /* 参数超出范围 */
    BAD_KEY_WORD                = (ERROR_BASE | 0x06),       /* 找不到关键词 */
    BAD_PARAM                   = (ERROR_BASE | 0x07),       /* 输入参数错误 */
    SDK_ERROR                   = (ERROR_BASE | 0x08),
    UNINIT                      = (ERROR_BASE | 0x09),       /* 还没有初始化 */
    PARAM_ERROR                 = (ERROR_BASE | 0x0a),       /* 参数错误 */
    AUTH_ERR                    = (ERROR_BASE | 0x0b),       /* 认证失败 */
    CONNECT_SERVER_ERR          = (ERROR_BASE | 0x0c),       /* 连接设备失败(ip或端口错误) */
    PERMISSION_DENIED           = (ERROR_BASE | 0x0d),       /* 没有权限 */    
    RECV_CTRL_TIMEOUT           = (ERROR_BASE | 0x0e),       /* 接受命令相应超时 */
	BAD_HANDLE                  = (ERROR_BASE | 0x0f),       /* 非法handle */
	BAD_AUDIO_TYPE              = (ERROR_BASE | 0x10),	    /* 音频格式不对 */
	STREAM_OPENED               = (ERROR_BASE | 0x11),      /* 流已经被打开 */
	STREAM_CLOSED               = (ERROR_BASE | 0x12),	    /* 流已经被关闭 */	
	TALK_HAS_OPENED             = (ERROR_BASE | 0x13),      /* 对讲已经被打开 */
	BAD_PARAM_NAME              = (ERROR_BASE | 0x14),      /* 参数名字错误 */
	STREAM_CNT_OVERFLOW         = (ERROR_BASE | 0x15),      /* 流通道太多(一个session最多ONE_SESSION_STREAM_CH个流通道) */	
	REC_NOT_EXIT                = (ERROR_BASE | 0x16),	    /* 录像文件不存在 */
	MALLOC_FAIL                 = (ERROR_BASE | 0x17),      /* malloc调用失败 */
	FILE_AT_END                 = (ERROR_BASE | 0x18),      /* 录像到达文件末尾 */
	FILE_HEAD                   = (ERROR_BASE | 0x19),      /* 回放到达文件开头 */
	SEND_DATA_FAIL              = (ERROR_BASE | 0x1a),	    /* 发送数据失败 */	
	MD5_CHECKSUM_FAIL           = (ERROR_BASE | 0x1b),      /* MD5校验失败 */
	SMTP_CONN_SVR_ERR           = (ERROR_BASE | 0x1c),
	SMTP_AUTH_ERR               = (ERROR_BASE | 0x1d),
	SMTP_SEND_HEAD_ERR          = (ERROR_BASE | 0x1e),
	SMTP_SEND_BODY_ERR          = (ERROR_BASE | 0x1f),	
	FTP_CONN_ERR                = (ERROR_BASE | 0x20),
	FTP_LOGIN_PARAM_ERR         = (ERROR_BASE | 0x21),
	FTP_USER_ERR                = (ERROR_BASE | 0x22),
	FTP_PWD_ERR                 = (ERROR_BASE | 0x23),
	FTP_UL_TYPE_ERR             = (ERROR_BASE | 0x24),
	FTP_UL_FILE_SIZE_ERR        = (ERROR_BASE | 0x25),
	FTP_UL_OPEN_FILE_ERR        = (ERROR_BASE | 0x26),
	FTP_UL_CHK_REMOTE_DIR_ERR   = (ERROR_BASE | 0x27),
	FTP_UL_CREATE_DATA_CHN_ERR  = (ERROR_BASE | 0x28),
	FTP_UL_SEND_DATA_CHN_ERR    = (ERROR_BASE | 0x29),
	FTP_DL_TYPE_ERR             = (ERROR_BASE | 0x2a),
	FTP_DL_FILE_SIZE_ERR        = (ERROR_BASE | 0x2b),
	FTP_DL_OPEN_FILE_ERR        = (ERROR_BASE | 0x2c),
	FTP_DL_CREATE_DATA_CHN_ERR  = (ERROR_BASE | 0x2d),	
	FTP_DL_RECV_CONFIRM_ERR     = (ERROR_BASE | 0x2e),
	FTP_DL_RECV_DATA_ERR        = (ERROR_BASE | 0x2f),
	FTP_ERROR                   = (ERROR_BASE | 0x30),	
    LOAD_CONFIG_FILE_ERROR      = (ERROR_BASE | 0x31),      /* 读取配置文件失败(在远程调用了本地SDK) */	
    SDK_UNINIT                  = (ERROR_BASE | 0x32),
    
/* 内部使用 */
    INNER_ERROR_BASE            = ((int)(0x80000000)),    
	HASH_ERROR                  = (INNER_ERROR_BASE | 0x01),		/* 哈希表冲突 */
	BAD_STREAM_ID               = (INNER_ERROR_BASE | 0x02),	    /* 流ID错误 */
	BAD_SESSION_ID              = (INNER_ERROR_BASE | 0x03),	    /* 回话ID错误 */
	CTRL_BUSY                   = (INNER_ERROR_BASE | 0x04), 
	WIFI_TEST_FAIL              = (INNER_ERROR_BASE | 0x05),
	RUN_CALLBACK_FAIL           = (INNER_ERROR_BASE | 0x06),	
    
    SDK_OK                      = (0),
    CHECKSUM_OK                 = (1),
    PARAM_SAME_VALUE            = (2),
    RESP_OK                     = (3),
	AUTH_OK                     = (4),  
	SET_OK                      = (5),  
	WIFI_TEST_OK                = (6),	
}SDK_RESULT_CODE;

#define		STRING_NONE		        ""
#define     LOCAL_HOST              "127.0.0.1"
#define     GET_FLAG                "get_"
#define		PARAM_FLAG			    "param_"
#define		CTRL_FLAG		        "ctrl_id"
#define     TWS_MARK     	        "TNVS"
#define		SEARCH_RESULT_PATH		"/tmp/search"
#define		CHECKFILE_PATH			"/tmp/checkfile"
#define		FLASH_CONFIG_FILE		"/mnt/config/flash_config.ini"
#define		FACTORY_CONFIG_FILE	"/mnt/config/factory_config.ini"
#define     VGA_SNAPSHOT_PATH       "/tmp/VGA.jpg"
#define     CGI_UPLOAD_PATH         "/tmp/upload"
#define     G711_AUDIO_FIFO         "/tmp/g711"
#define		TIMEZONE_FILE		    "/etc/TZ"
#define	SET_TIMEZONE_FILE	"/etc/profile"
#define     WEB_PATH                "/www"
#define     SDCARD_DEV              "/dev/mmcblk0p1"
#define 	TIMEZONE_CITY_NAME			"People's Republic of China"

#if 0
#define     ALARM_RECORD_INFO_FILE  "/mnt/sdcard/record/data.db"
#define     NORAL_RECORD_INFO_FILE  "/mnt/sdcard/record/data.db"

#define     ALARM_RECORD_INFO_TMP_FILE  "/mnt/sdcard/record/data_bak.db"
#define     NORAL_RECORD_INFO_TMP_FILE  "/mnt/sdcard/record/data_bak.db"
#else
#define     RECORD_INFO_FILE  		"/mnt/sdcard/record/data.db"
#define     RECORD_INFO_TMP_FILE  	"/mnt/sdcard/record/data_bak.db"
#endif

#define     DEFAULT_LAN_ADDR        "192.168.1.239"
#define     AP_CONFIG_FILE          "/etc/Wireless/RT2870AP/RT2870AP.dat"
#define     AP_SSID_HEADER          "SmartCam_"

#define     TEST_EMAIL_SUBJECT      "ip camera test email"
#define     TEST_EMAIL_CONTEXT      "this email send by ip camera"
#define     FTP_TEST_FILE           "/root/ftp_test.txt"
#define     DEFAULT_ALARM_SUBJECT   "camera alarm email"
#define     DEFAULT_ALARM_CONTEXT   "your camera alarm happened"
#define     DEFAULT_SNAP_SUBJECT    "camera snapshot email"
#define     DEFAULT_SNAP_CONTEXT    "get your camera snapshot"

#define     AUDIO_WIFI_FAIL_EN         "/etc/en/connect_failed_en.wav"
#define     AUDIO_WIFI_OK_EN           "/etc/en/connect_success_en.wav"
#define     AUDIO_ENTER_SMARK_LINK_EN  "/etc/en/wifi_configuration_en.wav"
#define     AUDIO_OUT_SMARK_LINK_EN    "/etc/en/ap_mode_en.wav"
#define     AUDIO_ALARM_EN             "/etc/en/alarm.wav"
#define     AUDIO_DEFAULT_EN           "/etc/en/reset_ok_en.wav"
#define     AUDIO_UPDATE_EN            "/etc/update_ok_en.wav"

#define     AUDIO_WIFI_FAIL_CH         "/etc/ch/connect_failed_ch.wav"
#define     AUDIO_WIFI_OK_CH           "/etc/ch/connect_success_ch.wav"
#define     AUDIO_ENTER_SMARK_LINK_CH  "/etc/ch/wifi_configuration_ch.wav"
#define     AUDIO_OUT_SMARK_LINK_CH    "/etc/ch/ap_mode_ch.wav"
#define     AUDIO_ALARM_CH             "/etc/ch/alarm.wav"
#define     AUDIO_DEFAULT_CH           "/etc/ch/reset_ok_ch.wav"
#define     AUDIO_UPDATE_CH            "/etc/update_ok_ch.wav"


#define     DEFAULT_MAC             "00:12:34:56:78:85"
#define     FACTORY_TEST_FILE       "/mnt/sdcard/factory.ini"

#define     CHECKSUM_STRING_LEN 	(strlen("checksum = ") + CHECKSUM_LEN)
#define     NALU_HEAD_LEN           (4)
#define     AAC_ADTS_HEAD_LEN       (7)
#define     CHECKSUM_LEN            (32)
#define     MAX_USER_CNT            (3)
#define		MAX_STREAM_CNT		    (4)
#define		MAX_POS_CNT		        (8)
#define		MD_ROI_CNT		        (4)
#define		MAX_CAM_CNT		        (9)
#define     MAX_CONNECT_CNT         (10)
#define     ONE_CONNECT_STREAM_CH   (4)
#define		ONE_SESSION_STREAM_CH	(4)
#define     HISI_G726_HEAD          (4)                 /* {0,1,160,0} */
#define     HASH_MAP_SIZE           (1024)
#define	    MAX_MSG_SIZE		    (1024 * 4)
#define		RETRY_DIFF_MS			(10)
#define		GET_FRAME_TIMEOUT_COUNT (15 * 10)			/* 2s 关键帧间隔会比较大*/
#define     CGI_PORT                (100)
#define     PROXY_PORT              (101)
#define     EVENT_PORT              (102)
#define     MAX_AP_CNT              (28)
#define     MIN_LIMIT_FOR_RECORD    (160) //(30)
#define     MIN_LIMIT_FOR_SNAP      (5)
#define     ALARM_RECORD_PEROID     (30)
#define 	NORMAL_RECORD_PEROID	(900)
#define     DEFAULT_HTTP_PORT       (81)
#define     ONLINE_FW_CHECK_PEROID  (7 * 24 * 3600)

#ifdef MCU_HI3518EV100
#define     MAX_FRAME_RATE          (20)
#elif defined MCU_HI3518EV200
#define     MAX_FRAME_RATE          (30)
#elif defined MCU_HI3516CV200
#define     MAX_FRAME_RATE          (30)
#endif
#ifdef SENSOR_SC1035
#undef      MAX_FRAME_RATE
#define     MAX_FRAME_RATE          (30)
#define     MAX_WIDTH_SIZE          (1280)
#define     MAX_HEIGHT_SIZE         (960)
#define     MAX_RESOLUTION          (P960)
#define     MAX_FRAME_SIZE          (700 * 1024)
#elif (defined SENSOR_OV9712 || defined SENSOR_OV9732)
#define     MAX_WIDTH_SIZE          (1280)
#define     MAX_HEIGHT_SIZE         (720)
#define     MAX_RESOLUTION          (P720)
#define     MAX_FRAME_SIZE          (500 * 1024)
#elif (defined SENSOR_AR0230 || defined SENSOR_GC2023)/*2023 add by zhang*/
#define     MAX_WIDTH_SIZE          (1920)
#define     MAX_HEIGHT_SIZE         (1080)
#define     MAX_RESOLUTION          (P1080)
#define     MAX_FRAME_SIZE          (768 * 1024) 
#endif

#define		G726_RADIO			    (8)
#define		AUDIO_G726_LEN		    (40)
#define		AUDIO_PCM_LEN		    (320)
#define     AUDIO_RAW_LEN           (G726_RADIO * AUDIO_G726_LEN)
#define		PCM_FRAME			    (32)
#define		G726_FRAME_LEN		    (40)

//ROUND_UP 向上取整函数 ROUND_DOWN向下取整函数
#define		ROUND_UP(x, unit)	    ((x)/(unit) + !!((x)%(unit)))
#define		ROUND_DOWN(x, unit)	    ((x)/(unit))
#define		MINS_PER_DAY	        (24 * 60)
#define		MINS_PER_HOUR	        (60)
#define		HOUR_TO_SEC(x)		    ((x) * 60 * 60)

#define		ARRAY_SIZE(x)		    (sizeof(x)/sizeof(x[0]))

#define     SDK_UNUSED_ARG(arg)     (void)arg

#define     KEY_SHM_PARAM           (0x2001)
#define     KEY_SHM_BUFFER          (0x2002)
#define		KEY_SEM_BUFFER			(0x2003)
#define		KEY_SEM_VGA_BUFFER		(0x2004)
#define     KEY_SEM_GPIO_CTRL      (0x2005)
#define     KEY_SEM_MPP            (0x2006)            /* 在mpp中已经使用 */
#define		KEY_RECORD_INDEX_FILE 	(0x2007)

#define     MSG_CTRL_KEY            (0x1001)
#define     MSG_PARAM_KEY    		 (0x1002)
#define     MSG_EVENT_KEY           (0x1003)
#define     MSG_UPGRADE_KEY         (0x1004)
#define     MSG_PROCESS_KEY         (0x1005)

#define     MSG_TYPE_PARAM_REQUEST  (1)
#define     MSG_TYPE_PARAM_RESPONSE (2)
#define     MSG_CMD_SET_PARAM       (1)
#define     MSG_CMD_GET_PARAM       (2)
#define		BIT(x)		            (0x1 << (x))

#define     FW_UPGRADE_TYPE         (1)
#define     JFFS2_SUPER_MAGIC       (0x72b6)
#define     MSDOS_SUPER_MAGIC       (0x4d44)

#define		FRAME_HEAD_LEN		    (sizeof(FRAME_HEAD))

#define		TIMER_UNIT			    (100 * 1000)		/* 100ms, unit:0.1s */
#define     SMART_LINK_TIMEOUT_MS   (3 * 60 *1000)  /* 5 mini */


#ifndef offsetof
#define offsetof(type, member) ((int) &((type *)0)->member)
#endif

#define container_of(ptr, type, member) ({			\
        const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
        (type *)( (char *)__mptr - offsetof(type,member) );}) 

#define PARAM_OK(arg)   \
    do      \
    {       \
        if ((int)(arg) == 0)   \
        {                   \
            LOG_ERROR("Invalid param!!!\n");    \
            return BAD_PARAM;       \
        }                           \
    }while(0)

typedef enum KEY_VALUE_T
{
    DEFAULT_KEY = (0x1 << 0),
    WIFI_KEY = (0x1 << 1),       
}KEY_VALUE;

/* 录像类型定义 */
typedef enum REC_TYPE_T
{
    ALARM_REC = 0,      /* 报警录像 */
    TIMER_REC,          /* 定时录像 */
}REC_TYPE;

typedef enum PLAYBACK_STATE_T
{
    PLAYBACK_STATE_PLAY = 0,
    PLAYBACK_STATE_PAUSE,
    PLAYBACK_STATE_STOP,
    PLAYBACK_STATE_MAX,
}PLAYBACK_STATE;

typedef enum RECORD_TYPE_T
{
    REC_MOTION_DETECT = 0,
    REC_ALARM_IN,   
    REC_NORMAL,
    REC_TOPWAY,
    REC_MAX,
}RECORD_TYPE;

typedef enum SYS_PARAM_TYPE_T
{
    FACTORY_TYPE = 0,
    FLASH_TYPE,
    RAM_TYPE,
    BAD_TYPE,
}SYS_PARAM_TYPE;

typedef enum UNLINK_FILE_TYPE_T
{
    FILE_TYPE_RECORD = 0,
    FILE_TYPE_SNAP,        
}UNLINK_FILE_TYPE_T;

typedef enum UPGRADE_MSG_CMD_T
{
    LOCAL_UPGRADE = 1,
    LOCAL_UPGRADE_UBOOT,
    LOCAL_UPGRADE_KERNEL,
    LOCAL_UPGRADE_ROOTFS,        
    LOCAL_UPGRADE_WEB, 
    ONLINE_UPGRADE,
    ONLINE_UPGRADE_UBOOT,            
    ONLINE_UPGRADE_KERNEL,        
    ONLINE_UPGRADE_ROOTFS,        
    ONLINE_UPGRADE_WEB,            
}UPGRADE_MSG_CMD;

typedef enum TIMER_MODE_T
{
    TYPE_REPEAT = 0,
    TYPE_ONCE,
    TYPE_EXIST,    
}TIMER_MODE;

typedef enum TIMER_RESULT_T
{
    TIME_OUT = 0,
    NO_TIME_OUT,        
}TIMER_RESULT;

typedef enum STREAM_TYPE_T
{
    STREAM_TYPE_VIDEO = 0,
    STREAM_TYPE_AUDIO,
}STREAM_TYPE;

typedef enum UPNP_PROTOCOL_T
{
    UPNP_TCP = 0,
    UPNP_UDP,
}UPNP_PROTOCAL;

typedef enum STREAM_CH_T
{
	MAIN_CH = (1 << 0),
	SUB1_CH = (1 << 1),
	SUB2_CH = (1 << 2),
	SUB3_CH = (1 << 3),
	AUDIO_G726 = (1 << 4),
	AUDIO_G711 = (1 << 5),
	WEB_CH = (1 << 6),      /* 浏览器专用 */
}STREAM_CH;

typedef enum FD_TYPT_T
{
	READ_FD = 0,
	WRITE_FD,
	EXCEPT_FD,
	FD_TYPE_MAX,
}FD_TYPE;

typedef enum CGI_ACTION_T
{
	SET_PARAM = 0,
	GET_PARAM,	
	CMD_PROC,
	SET_WIFI,
}CGI_ACTION;

typedef enum SELECT_RESULT_T
{
	SELECT_ERROR = -1,
	SELECT_TIMEOUT,
	SELECT_OK,
}SELECT_RESULT;

typedef enum NAL_UNIT_TYPE_T
{
	TYPE_SLICE  = 0x1,
	TYPE_IDR    = 0x5,			
	TYPE_SEI    = 0x6,		
	TYPE_SPS    = 0x7,
	TYPE_PPS    = 0x8,	
	TYPE_AUD    = 0x9,		
}NAL_UNIT_TYPE;

typedef enum CSC_ATTR_T
{
    ATTR_BRIGHTNESS = 0,
    ATTR_CONTRAST,
    ATTR_HUE,
    ATTR_SATUATURE,
}CSC_ATTR;

typedef enum TABLE_VALUE_T
{
    TAB_UNUSED = 0,
    TAB_USED,    
}TABLE_VALUE;

typedef enum FRAME_TYPE_T
{
	FRAME_TYPE_IDR          = (0x1 << 0),
	FRAME_TYPE_SUB1_IDR     = (0x1 << 1),
	FRAME_TYPE_SUB2_IDR     = (0x1 << 2),
	FRAME_TYPE_SUB3_IDR     = (0x1 << 3),
	FRAME_TYPE_I            = (0x1 << 4),
	FRAME_TYPE_SUB1_I       = (0x1 << 5),
	FRAME_TYPE_SUB2_I       = (0x1 << 6),
	FRAME_TYPE_SUB3_I       = (0x1 << 7),
	FRAME_TYPE_P            = (0x1 << 8),
	FRAME_TYPE_SUB1_P       = (0x1 << 9),
	FRAME_TYPE_SUB2_P       = (0x1 << 10),
	FRAME_TYPE_SUB3_P       = (0x1 << 11),
	FRAME_TYPE_B            = (0x1 << 12),	
	FRAME_TYPE_SUB1_B       = (0x1 << 13),	
	FRAME_TYPE_SUB2_B       = (0x1 << 14),
	FRAME_TYPE_SUB3_B       = (0x1 << 15),
	FRAME_TYPE_MJPEG        = (0x1 << 16),
	FRAME_TYPE_SUB1_MJPEG   = (0x1 << 17),
	FRAME_TYPE_SUB2_MJPEG   = (0x1 << 18),
	FRAME_TYPE_SUB3_MJPEG   = (0x1 << 19),	
	FRAME_TYPE_NEAR_ADPCM   = (0x1 << 20),
	FRAME_TYPE_FAR_ADPCM    = (0x1 << 21),	
	FRAME_TYPE_NEAR_G726    = (0x1 << 22),
	FRAME_TYPE_FAR_G726     = (0x1 << 23),
	FRAME_TYPE_NEAR_G711    = (0x1 << 24),
	FRAME_TYPE_FAR_G711     = (0x1 << 25),	
	FRAME_TYPE_NEAR_PCM     = (0x1 << 26),
	FRAME_TYPE_FAR_PCM      = (0x1 << 27),	
	FRAME_TYPE_NEAR_AAC     = (0x1 << 28),
}FRAME_TYPE;

#define		MAIN_STREAM_FRAME	(FRAME_TYPE_IDR      | FRAME_TYPE_I      | FRAME_TYPE_P      | FRAME_TYPE_B     )
#define		SUB1_STREAM_FRAME	(FRAME_TYPE_SUB1_IDR | FRAME_TYPE_SUB1_I | FRAME_TYPE_SUB1_P | FRAME_TYPE_SUB1_B)
#define		SUB2_STREAM_FRAME	(FRAME_TYPE_SUB2_IDR | FRAME_TYPE_SUB2_I | FRAME_TYPE_SUB2_P | FRAME_TYPE_SUB2_B)
#define		SUB3_STREAM_FRAME	(FRAME_TYPE_SUB3_IDR | FRAME_TYPE_SUB3_I | FRAME_TYPE_SUB3_P | FRAME_TYPE_SUB3_B)
#define		IDR_MASK	        (FRAME_TYPE_IDR | FRAME_TYPE_SUB1_IDR | FRAME_TYPE_SUB2_IDR | FRAME_TYPE_SUB3_IDR |	\
							        FRAME_TYPE_MJPEG | FRAME_TYPE_SUB1_MJPEG |FRAME_TYPE_SUB2_MJPEG | FRAME_TYPE_SUB3_MJPEG )

typedef struct CGI_INTERFACE_T
{
    SET_PARAM_STR_FUNC set_param_str_func;
    SET_PARAM_INT_FUNC set_param_int_func;
    GET_PARAM_FUNC get_param_func;
    CMD_PROC_FUNC cmd_proc_func;
}CGI_INTERFACE;

typedef struct PARAM_INFO_T
{
	int	type;					    /* 标识参数类型 (PARAM_TYPE_INT/PARAM_TYPE_STRING/PARAM_TYPE_CMD) */
	void *param;                      /* 对应的参数 */
    /* 仅在(INT/INT_STR)类型使用 */
    int min_value;              
    int max_value;
    /* 仅在(STRING)类型使用 */
    unsigned int string_len;
    
	void *callback;
}PARAM_INFO;

typedef struct CMD_INFO_T
{ 
 	CMD_PROC_CB callback;   
}CMD_INFO;

typedef struct WAV_HEAD_T
{
    char chunk_id[4];
    int chunk_size;
    char format[4];
    char subchunk1_id[4];
    int subchunk1_size;
    short int audio_format;
    short int num_channels;
    int sample_rate;			// sample_rate denotes the sampling rate.
    int byte_rate;
    short int block_align;
    short int bits_per_sample;
    char subchunk2_id[4];
    int subchunk2_size;			// subchunk2_size denotes the number of samples.
}WAV_HEAD;

/* 与下面的FRAME_HEAD合并 */
#pragma pack(1) /* h264_protocol 发送数据的时候需要 */
typedef struct AV_FRAME_T
{
    char mark[4];          /* "TNVS" */
    unsigned int pts;
    int type;               /* FRAME_TYPE_VIDEO/FRAME_TYPE_AUDIO [-1: ERROR] */
    unsigned int len; 
/* for video */    
    unsigned int fps;
    unsigned int resolution;    
    unsigned int ch;
/* for audio */
    unsigned int samplerate;
    unsigned int audio_type;   /* ADUIO_TYPE_G726/AUDIO_TYPE_ADPCM/... */
    unsigned int bitwidth;
    unsigned int channel;

	char data[MAX_FRAME_SIZE];
}AV_FRAME;
#pragma pack()

typedef struct CAMERA_INFO_T
{
    char alias[STR_32_LEN];
    char ip[STR_32_LEN];
    char uid[STR_32_LEN];
    int http_port;
}CAMERA_INFO;

typedef     void(*ON_GET_DEVICE)(CAMERA_INFO *pDevice);

typedef struct IPC_DEVINFO_T
{
    int Version;          /* [MAJOR_VERSION__MINOR_VERSION__REVISON_VERSION] */
    int Width;    
    int Height;
    int MaxFps;
    int StreamCnt;
    int PtzExist;
    int AudioExist;
    int WifiExist;    
}IPC_DEVINFO;

typedef struct RECORD_INFO_T
{
	char path[STR_64_LEN];
	char StartTime[16];
	char EndTime[16];
	int wday;
	int peroid;         /* s */
	int type;	
	int isdestroy;	//avi file is whole?
	long long lStartTime;   /* danale */
}RECORD_INFO;

typedef struct REC_FILE_INFO_T
{
	int stream_ch;
	int width;
	int height;
	int fps;
	char start_time[STR_64_LEN];
	char end_time[STR_64_LEN];
}REC_FILE_INFO;

typedef struct MESSAGE_T
{
    int mType;
    int mCommand;
    char mText[MAX_MSG_SIZE];
}MESSAGE;

/* FRAME 和 REAL_FRAME的区别 */
/* FRAME只在av_buffer中使用, data只是数据指针 */
/* REAL_FRAME在av_buffer的外部模块中使用, data包含实际数据 */
#pragma pack(1)
typedef struct FRAME_HEAD_T
{
    char mark[4];          /* "TNVS" */
    unsigned int pts;
    unsigned int type;        /* FRAME_TYPE_VIDEO/FRAME_TYPE_AUDIO */
    unsigned int len; 
/* for video */    
    unsigned int fps;
    unsigned int resolution;    
    unsigned int ch;
/* for audio */
    unsigned int samplerate;
    unsigned int audio_type;   /* ADUIO_TYPE_G726/AUDIO_TYPE_ADPCM/... */
    unsigned int bitwidth;
    unsigned int channel;
}FRAME_HEAD;
#pragma pack()

typedef struct REAL_FRAME_T
{
    int magic;
    int type;
    int len;
    char data[MAX_FRAME_SIZE];     /* read_frame使用该字段 */        
}REAL_FRAME;   

typedef struct AP_INFO_T
{
    int signal;
    int enc;      
    int auth;
    int channel;
    char ssid[STR_128_LEN];         /* STR_64_LEN: 可能会溢出 */
}AP_INFO;

typedef struct FW_HEADER_T
{
    char filename[16];
    char version[16];
    char md5sum[40];
    int filelen;
}FW_HEADER;

static inline void SET_MARK(char *str)
{
    strcpy(str, TWS_MARK);
}

static inline int CHECK_MARK(char *str)
{
    if (strncmp(str, TWS_MARK, strlen(TWS_MARK)))
        return SDK_ERROR;
    else
        return SDK_OK;
}

#define		FRAME_HEAD_LEN		(sizeof(FRAME_HEAD))

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif  /* __COMMON_H__ */
