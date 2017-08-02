/* 必须用ini文件保持系统配置,确保升级(推送升级)之后能够获取到之前的配置信息 */
#include "param.h"
#include "normal.h"
#include "debug.h"
#include "md5.h"
#include "dictionary.h"
#include "iniparser.h"
#include "boot_save_default.h"
#include "net.h"
#include "string_parser.h"
#include "config_str.h"
#include "hi_gpio.h"

#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>		
#include <sys/types.h>

#undef		TARGET_LEVEL
#define		TARGET_LEVEL		LEVEL_INFO

/* -------------------------   for param default   ------------------------- */
struct FLASH_PARAM_T default_flash_param =
{
	.magic_num = FLASH_CONFIG_MAGIC,          /* 如果magic_num不同, 升级之后会default */
	.ftp_param = 
	{
		.server = STRING_NONE,
		.port = 21,
		.user = STRING_NONE,
		.pwd = STRING_NONE,
		.upload_path = "/",
		.mode = PORT_MODE,
		.autocover = ON,
	},
	.email_param = 
	{
		.server = STRING_NONE,
		.sender = STRING_NONE,
		.user = STRING_NONE,                /* 暂不使用 */
		.pwd = STRING_NONE,
		.recv1 = STRING_NONE,
		.recv2 = STRING_NONE,	
		.recv3 = STRING_NONE,		
		.recv4 = STRING_NONE,		
		.port = 25,
		.mode = EMAIL_TLS,
		.subject = DEFAULT_ALARM_SUBJECT,
		.context = DEFAULT_ALARM_CONTEXT,
	},
	.video_param = 
	{
        .stream[MAIN_STREAM_CH - MAIN_STREAM_CH] =
        {
            .resolution = P1080,
            .fps = 15,
            .idr = 15,
            .bitrate = 1024,
        },
        .stream[SUB1_STREAM_CH - MAIN_STREAM_CH] =
        {
            .resolution = VGA,            
            .fps = 15,
            .idr = 15,
            .bitrate = 512,
        },
        .stream[SUB2_STREAM_CH - MAIN_STREAM_CH] =
        {
            .fps = 15,
            .idr = 15,
            .bitrate = 512,
        },
        .stream[SUB3_STREAM_CH - MAIN_STREAM_CH] =
        {
            .fps = 15,
            .idr = 15,
            .bitrate = 200,
        },    
		.flip = FLIP_HV,
		.wideDynamic = OFF,
		.osd_name_enb = ON,
		.osd_time_enb = ON,
		.brightness = 50,
		.contrast = 50,
		.hue = 50,
		.saturation = 50,
		.quality = 3,
	},
	.audio_param = 
	{
		.type = AUDIO_TYPE_G711,
		.in_vol = 64,
		.out_vol = 94,
		.mic_enable = ON,
		.samplerate = 8000,
		.bitwidth = 16,
		.bitrate = 64,//200,
		.channel = 1,		
	},
	.ddns_param = 
	{
		.enable = OFF,              /* 关闭第三方DDNS */
		.type = DDNS_DYNDNS,
		.domain = STRING_NONE,
		.server = STRING_NONE,
		.user = STRING_NONE,
		.pwd = STRING_NONE,	
	},
	.ptz_param = 
	{
		.speed = SPEED_LOW,
		.start_pos = 0,
		.circle_cnt = 5,                /* 0(一直巡航) */
		.motor_pos[0] = {-1, -1},
		.motor_pos[1] = {-1, -1},
		.motor_pos[2] = {-1, -1},
		.motor_pos[3] = {-1, -1},
		.motor_pos[4] = {-1, -1},
		.motor_pos[5] = {-1, -1},
		.motor_pos[6] = {-1, -1},
		.motor_pos[7] = {-1, -1},	
	},
	.time_param = 
	{
		.sync_type = NTP_SYNC,
		.ntp_server = "time.nist.gov",
		.sync_interval = 1,             /* unit: hour */
		.saving_time = 0,
		.enb_savingtime = OFF,
		.timezone = GMT8,
		.timezone_city = "People's Republic of China",
	},
	.net_param = 
	{
		.dhcp = DHCP_ALL,
		.ip = "192.168.0.128",          /* nfs时候要注意修改在同一个网段,还有修改udhcpc_all.default(udhcpc_ip.default) */
		.netmask = "255.255.255.0",
		.gateway = "192.168.0.1",
		.dns1 = "192.168.0.1",
		.dns2 = "8.8.8.8",		
		.mac = "00:12:34:56:78:85",
	},
	.record_param = 
	{
		.enable = OFF,
		.stream_ch = MAIN_STREAM_CH,
		.peroid = NORMAL_RECORD_PEROID,//60,			        /*  unit: s*/
		.sch[0] = {"111111111111111111111111111111111111111111111111"},
		.sch[1] = {"111111111111111111111111111111111111111111111111"},
		.sch[2] = {"111111111111111111111111111111111111111111111111"},
		.sch[3] = {"111111111111111111111111111111111111111111111111"},
		.sch[4] = {"111111111111111111111111111111111111111111111111"},
		.sch[5] = {"111111111111111111111111111111111111111111111111"},
		.sch[6] = {"111111111111111111111111111111111111111111111111"},
		.start_time = "00:00:00",
		.end_time = "23:59:59",
		.day[0] = OFF,
		.day[1] = OFF,	
		.day[2] = OFF,
		.day[3] = OFF,	
		.day[4] = OFF,	
		.day[5] = OFF,
		.day[6] = OFF,	
		.normal_rec_plan = 	/*normal record plan*/
		{
			.start_time = "00:00:00",
			.end_time = "23:59:59",
			.day[0] = OFF,
			.day[1] = OFF,	
			.day[2] = OFF,
			.day[3] = OFF,	
			.day[4] = OFF,	
			.day[5] = OFF,
			.day[6] = OFF,	
			.stepoverflag = 0,
		},
	},
	.snap_param = 
	{
		.enable = OFF,
		.sd_enable = OFF,
		.sd_peroid = 60,
		.email_enable = OFF,
		.email_peroid = 60,
		.ftp_enable = OFF,
		.ftp_peroid = 60,
	},	
	.user_param[0] = {USER_ADMIN, "admin", "admin"},
	.user_param[1] = {USER_OPT, "opt", "opt"},
	.user_param[2] = {USER_GUEST, "guest", "guest"},
	
	.multi_param[0] = 
	    {.alias = STRING_NONE, .ip = STRING_NONE, .user = STRING_NONE, .pwd = STRING_NONE, .port = 0},
	.multi_param[1] = 
	    {.alias = STRING_NONE, .ip = STRING_NONE, .user = STRING_NONE, .pwd = STRING_NONE, .port = 0},
	.multi_param[2] = 
	    {.alias = STRING_NONE, .ip = STRING_NONE, .user = STRING_NONE, .pwd = STRING_NONE, .port = 0},
	.multi_param[3] = 
	    {.alias = STRING_NONE, .ip = STRING_NONE, .user = STRING_NONE, .pwd = STRING_NONE, .port = 0},
	.multi_param[4] = 
	    {.alias = STRING_NONE, .ip = STRING_NONE, .user = STRING_NONE, .pwd = STRING_NONE, .port = 0},
	.multi_param[5] = 
	    {.alias = STRING_NONE, .ip = STRING_NONE, .user = STRING_NONE, .pwd = STRING_NONE, .port = 0},
	.multi_param[6] = 
	    {.alias = STRING_NONE, .ip = STRING_NONE, .user = STRING_NONE, .pwd = STRING_NONE, .port = 0},
	.multi_param[7] = 
	    {.alias = STRING_NONE, .ip = STRING_NONE, .user = STRING_NONE, .pwd = STRING_NONE, .port = 0},
	.multi_param[8] = 
	    {.alias = STRING_NONE, .ip = STRING_NONE, .user = STRING_NONE, .pwd = STRING_NONE, .port = 0},
            
	.wifi_param = 
	{
	    .enable = ON,
		.auth = AUTH_WPA2,
		.enc = ENC_TKIP,
		.ssid = STRING_NONE,
		.key = STRING_NONE,
	},
	.alarm_param = 
	{
		.roi[0] = {100, 100, 100, 100},
		.roi[1] = {100, 100, 100, 100},
		.roi[2] = {100, 100, 100, 100},
		.roi[3] = {100, 100, 100, 100},	
		.roi_str = {"0", "0", "0", "0"},
		.enable = {ON, OFF, OFF, OFF},	
#ifdef	TUTK_P2P_PLATFORM
		.sensitivity = {MD_SENSITIVITY_3, 50, 50, 50},
#elif defined DANALE_P2P_PLATFORM
		.sensitivity = {MD_NORMAL, 50, 50, 50},
#endif
		.alarm_bit = { 0xff,0xff,0xff,0xff,0xff,0xff,      \
		               0xff,0xff,0xff,0xff,0xff,0xff,      \
		               0xff,0xff,0xff,0xff,0xff,0xff,      \
		               0xff,0xff,0xff,0xff,0xff,0xff,      \
		               0xff,0xff,0xff,0xff,0xff,0xff },
		.ptz_enb = OFF,		
		.sound_enb = OFF,		
		.ftp_pic = OFF,		
		.email_pic = OFF,
		.pic_to_sd = OFF,		
		.rec_to_sd = OFF,
		.rec_to_ftp = OFF,		
		.pic_cnt = 3,			
		.sch_enb = OFF,
		.sch_day[0] = {"000000000000000000000000000000000000000000000000"},
		.sch_day[1] = {"000000000000000000000000000000000000000000000000"},
		.sch_day[2] = {"000000000000000000000000000000000000000000000000"},
		.sch_day[3] = {"000000000000000000000000000000000000000000000000"},
		.sch_day[4] = {"000000000000000000000000000000000000000000000000"},
		.sch_day[5] = {"000000000000000000000000000000000000000000000000"},
		.sch_day[6] = {"000000000000000000000000000000000000000000000000"},
		.alarm_rec_plan =	/*alarm record plan*/
		{
			.start_time = "00:00:00",
			.end_time = "23:59:59",
			.day[0] = ON,
			.day[1] = ON,	
			.day[2] = ON,
			.day[3] = ON,	
			.day[4] = ON,	
			.day[5] = ON,
			.day[6] = ON,	
			.stepoverflag = 0,
		},
		.period = ALARM_RECORD_PEROID,
	},
	.misc_param = 
	{
#if  DEBUG_MODULES       
		.soft_version = FIRMWARE_VERSION "(debug)",             /* read only */
#else
        .soft_version = FIRMWARE_VERSION,                       /* read only */
#endif
        .uboot_version = UBOOT_VERSION,
        .kernel_version = KERNEL_VERSION,        
		.git_head = STRING_NONE,                                /* read only */		
		.name = "New Camera",
		.stream_index = MAIN_STREAM_CH,
		.pwr_freq = F50HZ,		
		.ir_led = IRLED_AUTO,		
		.http_port = 81,
		.rtsp_port = 554,
		.onvif_port = 888,
		.p2p_enable = ON,
		.upnp_enable = ON,
		.ocx_rec_path = "D:/record/",
		.osd = "camera",
		.app_video_quality = 2,     /* 720P768bps */
	},
	.rtmp_param = 
    {
        .port = 1935,
        .push_enb = ON,
        .push_url = STRING_NONE,
    },
    .led_status_onoff = 
	{
	    .wifi_led_status = LED_CTL_AUTO,
	    .ir_led_status = LED_CTL_AUTO,
    },
};

/* E9JX81LYJSLL3J6PYF6J  已经使用(JACK) */
/* R42UDJ88PRPNFA9G111A  已经使用 */
/* CH4DA3PY9SBFU56PUZZ1 */

struct FACTORY_PARAM_T default_factory_param =
{
#if 0    /* 不同版本的magic_num不同,在升级时会把参数恢复到出厂数据 */
	//.magic_num = FACTORY_CONFIG_MAGIC,
#else
	#ifdef 	TUTK_P2P_PLATFORM
		.magic_num = 5,	//don't change	default:5
	#elif defined DANALE_P2P_PLATFORM
		.magic_num = 8,
	#endif
#endif

	.author = "liwei/313091745@qq.com",                     /* read only */	
    .uid = "00000000000000000000",//F3YAAN7WZ57WUH6GU1X1
	.inner_ddns_enb = ON,	
#if 1	
	.limit_x = 2900,
	.limit_y = 640,
#else   /* for tenvis */
	.limit_x = 4040,
	.limit_y = 640,
#endif
	.hard_version = HARDWARE_VERSION,      
	.model = CAMERA_MODEL, 
#ifdef 	TUTK_P2P_PLATFORM
	.p2p_type = TUTK_P2P,
#elif defined DANALE_P2P_PLATFORM
	.p2p_type = DANALE_P2P,
#endif
	.oem = OEM_INFO,
    .region = "english",
   // .version_desc = "http://10.8.17.148/down/description/241AA_20160223.ini",
   	.version_desc = "http://192.168.99.135/241AA_20160817.ini",
   //.version_desc = "http://192.168.0.54/description/241AA_20160817.ini",
#ifdef  SENSOR_OV9732     
    .sensor = "ov9732",
#elif defined   SENSOR_GC2023     
		.sensor = "gc2023",
#endif 

	.ddns_param = 
	{
        .type = DDNS_EASYN,
        .enable = ON,   
        {
            .TvsDdnsInfo = 
        	{
                .ddns_server = "mytenvis.org",
                .server_port = 80,
                .user = "bsd4567",
                .pwd = "1d3e216",        
                .acod = "AALL",
            },
        	.EasynDdnsInfo =    /* sample: user[aouru] pwd[144695] */
        	{
#if 0   /* for easyn ddns */               
                .ddns_server = "user.easyn.hk",
#else
                .ddns_server = "user.ipcam.hk",
#endif                
                .server_port = 808,
        		.user = STRING_NONE,
        		.pwd = STRING_NONE,
                .vertype = FIRMWARE_VERSION,
                .language = 0,                  /* 0: english, 1: simple */
                .dtype = 0,
                .tcpport = DEFAULT_HTTP_PORT,
                .lanip = DEFAULT_LAN_ADDR,
        	},
        },     
    },
};
/* -----------------------------  end for param default  ----------------------------- */

int check_file(char *file)
{
	int check_ret0, check_ret1;
	char back_file[128] = {0};


	sprintf(back_file, "%s_bak", file);
	check_ret0 = checksum(file);
	check_ret1 = checksum(back_file);

	if ((check_ret0 != CHECKSUM_OK) && (check_ret1 == CHECKSUM_OK))
	{
		LOG_INFO("file: %s %s checksum ok\n", file, back_file);
	}
	else if ((check_ret0 != CHECKSUM_OK) && (check_ret1 == CHECKSUM_OK))
	{	
		LOG_WARN("file: %s checksum fail\n", file);
		unlink(file);
		copy_file(back_file, file);

	}
	else if ((check_ret1 != CHECKSUM_OK) && (check_ret0 == CHECKSUM_OK))
	{
		LOG_WARN("file: %s checksum fail\n", back_file);	
		unlink(back_file);
		copy_file(file, back_file);
	}
	else if ((check_ret1 != CHECKSUM_OK) && (check_ret0 != CHECKSUM_OK))
	{
		LOG_ERROR("file: %s %s checksum fail\n", file, back_file); 
		unlink(file);
		unlink(back_file);		

		return SDK_ERROR;
	}

	return SDK_OK;
}

int read_magic(char *file)
{
	int magic;
	dictionary *Dictionary = NULL;

	PARAM_OK(file);

	Dictionary = iniparser_load(file);
	if (NULL == Dictionary)
	{
		LOG_ERROR("error at iniparser_load\n");
		return SDK_ERROR;
	}
	magic = iniparser_getint(Dictionary, (char *)"magic:num", 0);
	iniparser_freedict(Dictionary);   

	return magic;
}

void copy_to_backfile(char *file)
{
	char back_file[128] = {0};

	sprintf(back_file, "%s_bak", file);
	copy_file(file, back_file);
}

/* -------------------------  for ram param  ------------------------- */
void boot_ram_config(struct SYSTEM_PARAM_T *pSystemParam)
{
    RAM_PARAM *pRamParam = &pSystemParam->ram_param;
    FLASH_PARAM *pFlashParam = &pSystemParam->flash_param;	
    FACTORY_PARAM *pFactoryParam = &pSystemParam->factory_param;

    LOG_INFO("init: %d\n", pRamParam->init);

    memset(pRamParam, 0, sizeof(RAM_PARAM));
    pRamParam->UpgradeMsgHandle = -1;      /* BUG: 进程重启后,无法升级 */
	time_now(pRamParam->system_status.start_time);
    pRamParam->system_status.run_time = get_time_ms() / 1000;
	pRamParam->system_status.wifi_test_result = TEST_OK;
	pRamParam->system_status.ftp_test_result = TEST_OK;
	pRamParam->system_status.email_test_result = TEST_OK;		
	pRamParam->system_status.talk_alive = OFF;
	if (pFactoryParam->ddns_param.enable == ON)
		pRamParam->system_status.inner_ddns_status = DDNS_UPDATE_FAIL;		
	else
		pRamParam->system_status.inner_ddns_status = TVS_DDNS_OFF;
	
	if (pFlashParam->ddns_param.enable == ON)
		pRamParam->system_status.thr_ddns_status = DDNS_UPDATE_FAIL;		
	else
		pRamParam->system_status.thr_ddns_status = THR_DDNS_OFF;
	
	if (pFlashParam->misc_param.upnp_enable == ON)
		pRamParam->system_status.upnp_status = UPNP_FAIL;	
	else
		pRamParam->system_status.upnp_status = UPNP_OFF;	
	
	if (pFlashParam->time_param.sync_type == NTP_SYNC)
		pRamParam->system_status.ntp_status = NTP_SYNC_FAIL;	
	else
		pRamParam->system_status.ntp_status = NTP_SYNC_OFF;	

	if (pFlashParam->wifi_param.enable == ON)
		pRamParam->system_status.wifi_status = WIFI_DISCONNECT;	
	else
		pRamParam->system_status.wifi_status = OFF;	
    pRamParam->key = 0;
    pRamParam->system_status.upgrade = 0;
    pRamParam->smart_link_flag = 0; 
    pRamParam->mpp_init_flag = 0;
    pRamParam->wifi_mode = 0;
    pRamParam->real_fps[0] = pFlashParam->video_param.stream[0].fps;
    pRamParam->real_fps[1] = pFlashParam->video_param.stream[1].fps;  
    LOG_INFO("real_fps: %d_%d\n", pRamParam->real_fps[0], pRamParam->real_fps[1]);
	/* 获取网络信息 */
    pRamParam->system_status.net_inf = NET_TYPE_NULL;		
	netcfg_get_ip_addr("eth0", pRamParam->net_param.ip);
	netcfg_get_mask_addr("eth0", pRamParam->net_param.netmask);	
	netcfg_get_gw_addr("eth0", pRamParam->net_param.gateway);
	LOG_INFO("ip_addr: %s\n", pRamParam->net_param.ip);
	LOG_INFO("netmask: %s\n", pRamParam->net_param.netmask);
	LOG_INFO("gateway: %s\n", pRamParam->net_param.gateway);	
	pRamParam->net_status = NET_IDLE;
	pRamParam->airlink_flag = 0;
	
    pRamParam->init = 1;        
}
/* -------------------------  end for ram param  ------------------------- */


#define     GET_STRING(value, str)      \
    do                                                                          \
    {                                                                           \
        pStr = iniparser_getstring(Dictionary, (char *)str, NULL);           \
        if (pStr) strcpy(value, pStr);                                       \
    }while(0)

#define     GET_INT(value, str)         \
    value = iniparser_getint(Dictionary, (char *)str, 0)
#define     GET_INT_STRING(value, str)                                          \
    do                                                                          \
    {                                                                           \
        pStr = iniparser_getstring(Dictionary, (char *)str, NULL);           \
        if (pStr) value = Str2Index(pStr);                                \
    }while(0)
    
/* -------------------------  for flash param  ------------------------- */
/* 在线升级必须保存wifi数据 */
int boot_flash_config_real(char *file, struct SYSTEM_PARAM_T *system_param)
{
    int i;
	char str[STR_512_LEN] = {0};
    char *pStr = NULL;     /* pStr(GET_STRING中使用) */
	dictionary	*Dictionary = NULL;
	struct FLASH_PARAM_T *flash_param;

	PARAM_OK(file && system_param);

	flash_param = &system_param->flash_param;
	/* 新版本有新的参数，将恢复到default值 */
	memcpy(flash_param, &default_flash_param, sizeof(default_flash_param));
	Dictionary = iniparser_load(file);
	if (Dictionary == NULL)
	{
		LOG_ERROR("error at iniparser_load\n");
		return SDK_ERROR;
	}

	/* for flash param */
	GET_INT(flash_param->magic_num,                     FLASH_MAGIC_NUM);
	
	GET_STRING(flash_param->ftp_param.server,           FLASH_FTP_SERVER);
	GET_INT(flash_param->ftp_param.port,                FLASH_FTP_PORT);	
	GET_STRING(flash_param->ftp_param.user,             FLASH_FTP_USER);
	GET_STRING(flash_param->ftp_param.pwd,              FLASH_FTP_PWD);
	GET_STRING(flash_param->ftp_param.upload_path,      FLASH_FTP_UPLOAD_PATH);		
	GET_INT_STRING(flash_param->ftp_param.mode,         FLASH_FTP_MODE);
	GET_INT_STRING(flash_param->ftp_param.autocover,    FLASH_FTP_AUTOCOVER);    

	GET_STRING(flash_param->email_param.server, FLASH_EMAIL_SERVER);
	GET_STRING(flash_param->email_param.sender, FLASH_EMAIL_SENDER);
	GET_STRING(flash_param->email_param.user, FLASH_EMAIL_USER);    
	GET_STRING(flash_param->email_param.pwd, FLASH_EMAIL_PWD);
	GET_STRING(flash_param->email_param.recv1, FLASH_EMAIL_RECV1);
	GET_STRING(flash_param->email_param.recv2, FLASH_EMAIL_RECV2);
	GET_STRING(flash_param->email_param.recv3, FLASH_EMAIL_RECV3);
	GET_STRING(flash_param->email_param.recv4, FLASH_EMAIL_RECV4);    
	GET_INT(flash_param->email_param.port, FLASH_EMAIL_PORT);	
	GET_INT_STRING(flash_param->email_param.mode, FLASH_EMAIL_MODE);	
	GET_STRING(flash_param->email_param.subject, FLASH_EMAIL_SUBJECT);	
	GET_STRING(flash_param->email_param.context, FLASH_EMAIL_CONTEXT);

	GET_INT_STRING(flash_param->video_param.stream[0].resolution, FLASH_VIDEO_RESOLUTION0);
	GET_INT(flash_param->video_param.stream[0].fps, FLASH_VIDEO_FPS0);
	GET_INT(flash_param->video_param.stream[0].idr, FLASH_VIDEO_IDR0);
	GET_INT(flash_param->video_param.stream[0].bitrate, FLASH_VIDEO_BITRATE0);	
	GET_INT_STRING(flash_param->video_param.stream[1].resolution, FLASH_VIDEO_RESOLUTION1);
	GET_INT(flash_param->video_param.stream[1].fps, FLASH_VIDEO_FPS1);
	GET_INT(flash_param->video_param.stream[1].idr, FLASH_VIDEO_IDR1);
	GET_INT(flash_param->video_param.stream[1].bitrate, FLASH_VIDEO_BITRATE1);
	GET_INT_STRING(flash_param->video_param.stream[2].resolution, FLASH_VIDEO_RESOLUTION2);
	GET_INT(flash_param->video_param.stream[2].fps, FLASH_VIDEO_FPS2);
	GET_INT(flash_param->video_param.stream[2].idr, FLASH_VIDEO_IDR2);
	GET_INT(flash_param->video_param.stream[2].bitrate, FLASH_VIDEO_BITRATE2);
   
	GET_INT_STRING(flash_param->video_param.flip, FLASH_VIDEO_FLIP);    
	GET_INT_STRING(flash_param->video_param.wideDynamic, FLASH_VIDEO_WIDEDYNAMIC); 
	GET_INT_STRING(flash_param->video_param.osd_name_enb, FLASH_VIDEO_OSD_NAME_ENB);
	GET_INT_STRING(flash_param->video_param.osd_time_enb, FLASH_VIDEO_OSD_TIME_ENB);   
   
	GET_INT(flash_param->video_param.brightness, FLASH_VIDEO_BRIGHTNESS);
	GET_INT(flash_param->video_param.contrast, FLASH_VIDEO_CONTRAST);
	GET_INT(flash_param->video_param.hue, FLASH_VIDEO_HUE);
	GET_INT(flash_param->video_param.saturation, FLASH_VIDEO_SATURATION);    
	GET_INT(flash_param->video_param.quality, FLASH_VIDEO_QUALITY);

	GET_INT_STRING(flash_param->audio_param.type, FLASH_AUDIO_TYPE);
	GET_INT(flash_param->audio_param.in_vol, FLASH_AUDIO_IN_VOL);
	GET_INT(flash_param->audio_param.out_vol, FLASH_AUDIO_OUT_VOL);	
	GET_INT_STRING(flash_param->audio_param.mic_enable, FLASH_AUDIO_MIC_ENABLE);
	GET_INT(flash_param->audio_param.samplerate, FLASH_AUDIO_SAMPLERATE);
	GET_INT(flash_param->audio_param.bitwidth, FLASH_AUDIO_BITWIDTH);	
	GET_INT(flash_param->audio_param.bitrate, FLASH_AUDIO_BITRATE);
	GET_INT(flash_param->audio_param.channel, FLASH_AUDIO_CHANNEL);	    

	GET_INT_STRING(flash_param->ddns_param.enable, FLASH_DDNS_ENABLE);	
	GET_INT_STRING(flash_param->ddns_param.type, FLASH_DDNS_TYPE);	
	GET_STRING(flash_param->ddns_param.domain, FLASH_DDNS_DOMAIN);	
	GET_STRING(flash_param->ddns_param.server, FLASH_DDNS_SERVER);	
	GET_STRING(flash_param->ddns_param.user, FLASH_DDNS_USER);		
	GET_STRING(flash_param->ddns_param.pwd, FLASH_DDNS_PWD); 

	GET_INT_STRING(flash_param->ptz_param.speed, FLASH_PTZ_SPEED);	
	GET_INT(flash_param->ptz_param.start_pos, FLASH_PTZ_START_POS);	
	GET_INT(flash_param->ptz_param.circle_cnt, FLASH_PTZ_CIRCLE_CNT);	    
	GET_INT(flash_param->ptz_param.motor_pos[0].x, FLASH_PTZ_POS_X1);	
	GET_INT(flash_param->ptz_param.motor_pos[0].y, FLASH_PTZ_POS_Y1);	
	GET_INT(flash_param->ptz_param.motor_pos[1].x, FLASH_PTZ_POS_X2);	
	GET_INT(flash_param->ptz_param.motor_pos[1].y, FLASH_PTZ_POS_Y2);
	GET_INT(flash_param->ptz_param.motor_pos[2].x, FLASH_PTZ_POS_X3);	
	GET_INT(flash_param->ptz_param.motor_pos[2].y, FLASH_PTZ_POS_Y3);	
	GET_INT(flash_param->ptz_param.motor_pos[3].x, FLASH_PTZ_POS_X4);	
	GET_INT(flash_param->ptz_param.motor_pos[3].y, FLASH_PTZ_POS_Y4);
	GET_INT(flash_param->ptz_param.motor_pos[4].x, FLASH_PTZ_POS_X5);	
	GET_INT(flash_param->ptz_param.motor_pos[4].y, FLASH_PTZ_POS_Y5);	
	GET_INT(flash_param->ptz_param.motor_pos[5].x, FLASH_PTZ_POS_X6);	
	GET_INT(flash_param->ptz_param.motor_pos[5].y, FLASH_PTZ_POS_Y6);
	GET_INT(flash_param->ptz_param.motor_pos[6].x, FLASH_PTZ_POS_X7);	
	GET_INT(flash_param->ptz_param.motor_pos[6].y, FLASH_PTZ_POS_Y7);	
	GET_INT(flash_param->ptz_param.motor_pos[7].x, FLASH_PTZ_POS_X8);	
	GET_INT(flash_param->ptz_param.motor_pos[7].y, FLASH_PTZ_POS_Y8);    

	GET_INT_STRING(flash_param->time_param.sync_type, FLASH_TIME_SYNC_TYPE);		
	GET_STRING(flash_param->time_param.ntp_server, FLASH_TIME_NTP_SERVER);	
	GET_INT(flash_param->time_param.sync_interval, FLASH_TIME_SYNC_INTERVAL);    
	GET_INT(flash_param->time_param.saving_time, FLASH_TIME_SAVING_TIME);	
	GET_INT_STRING(flash_param->time_param.enb_savingtime, FLASH_TIME_ENB_SAVINGTIME);		
	GET_INT_STRING(flash_param->time_param.timezone, FLASH_TIME_TIMEZONE);	    
	GET_STRING(flash_param->time_param.timezone_city, FLASH_TIME_TIMEZONE_CITY);		

	GET_INT_STRING(flash_param->net_param.dhcp, FLASH_NETWORK_DHCP);		
	GET_STRING(flash_param->net_param.ip, FLASH_NETWORK_IP);	
	GET_STRING(flash_param->net_param.netmask, FLASH_NETWORK_NETMASK);	
	GET_STRING(flash_param->net_param.gateway, FLASH_NETWORK_GATEWAY);
	GET_STRING(flash_param->net_param.dns1, FLASH_NETWORK_DNS1);	
	GET_STRING(flash_param->net_param.dns2, FLASH_NETWORK_DNS2);
	GET_STRING(flash_param->net_param.mac, FLASH_NETWORK_MAC);        

	GET_INT_STRING(flash_param->record_param.enable, FLASH_RECORD_ENABLE);		
	GET_INT_STRING(flash_param->record_param.stream_ch, FLASH_RECORD_STREAM_CH);    
	GET_INT(flash_param->record_param.peroid, FLASH_RECORD_PEROID);   
	GET_STRING(flash_param->record_param.sch[0], FLASH_RECORD_SCH1);  
	GET_STRING(flash_param->record_param.sch[1], FLASH_RECORD_SCH2);       
	GET_STRING(flash_param->record_param.sch[2], FLASH_RECORD_SCH3);       
	GET_STRING(flash_param->record_param.sch[3], FLASH_RECORD_SCH4);       
	GET_STRING(flash_param->record_param.sch[4], FLASH_RECORD_SCH5);       
	GET_STRING(flash_param->record_param.sch[5], FLASH_RECORD_SCH6);     
	GET_STRING(flash_param->record_param.sch[6], FLASH_RECORD_SCH7);   
	GET_INT_STRING(flash_param->record_param.day[0], FLASH_RECORD_DAY1);  
	GET_INT_STRING(flash_param->record_param.day[1], FLASH_RECORD_DAY2);       
	GET_INT_STRING(flash_param->record_param.day[2], FLASH_RECORD_DAY3);       
	GET_INT_STRING(flash_param->record_param.day[3], FLASH_RECORD_DAY4);       
	GET_INT_STRING(flash_param->record_param.day[4], FLASH_RECORD_DAY5);       
	GET_INT_STRING(flash_param->record_param.day[5], FLASH_RECORD_DAY6);     
	GET_INT_STRING(flash_param->record_param.day[6], FLASH_RECORD_DAY7);     
	GET_STRING(flash_param->record_param.start_time, FLASH_RECORD_STARTTIME);    
	GET_STRING(flash_param->record_param.end_time, FLASH_RECORD_ENDTIME);      

	GET_STRING(flash_param->record_param.normal_rec_plan.start_time,FLASH_RECORD1_START_TIME);          
	GET_STRING(flash_param->record_param.normal_rec_plan.end_time,FLASH_RECORD1_END_TIME);     
    	GET_INT_STRING(flash_param->record_param.normal_rec_plan.day[0],FLASH_RECORD1_DAY1);      
	GET_INT_STRING(flash_param->record_param.normal_rec_plan.day[1],FLASH_RECORD1_DAY2);      
	GET_INT_STRING(flash_param->record_param.normal_rec_plan.day[2],FLASH_RECORD1_DAY3);      
	GET_INT_STRING(flash_param->record_param.normal_rec_plan.day[3],FLASH_RECORD1_DAY4);      
	GET_INT_STRING(flash_param->record_param.normal_rec_plan.day[4],FLASH_RECORD1_DAY5);      
	GET_INT_STRING(flash_param->record_param.normal_rec_plan.day[5],FLASH_RECORD1_DAY6);          
	GET_INT_STRING(flash_param->record_param.normal_rec_plan.day[6],FLASH_RECORD1_DAY7); 
	GET_INT(flash_param->record_param.normal_rec_plan.stepoverflag,FLASH_RECORD1_STEPOVERFLAG); 

	GET_INT_STRING(flash_param->snap_param.enable, FLASH_SNAP_EANBLE);	
	GET_INT_STRING(flash_param->snap_param.sd_enable, FLASH_SNAP_SD_ENABLE);		
	GET_INT(flash_param->snap_param.sd_peroid, FLASH_SNAP_SD_PEROID);   
	GET_INT_STRING(flash_param->snap_param.email_enable, FLASH_SNAP_EMAIL_ENABLE);		
	GET_INT(flash_param->snap_param.email_peroid, FLASH_SNAP_EMAIL_PEROID);    
	GET_INT_STRING(flash_param->snap_param.ftp_enable, FLASH_SNAP_FTP_ENABLE);		
	GET_INT(flash_param->snap_param.ftp_peroid, FLASH_SNAP_FTP_PEROID);       

	GET_INT_STRING(flash_param->user_param[0].level, FLASH_USER_LEVEL1);
	GET_STRING(flash_param->user_param[0].user, FLASH_USER_USER1);	
	GET_STRING(flash_param->user_param[0].pwd, FLASH_USER_PWD1);
	GET_INT_STRING(flash_param->user_param[1].level, FLASH_USER_LEVEL2);
	GET_STRING(flash_param->user_param[1].user, FLASH_USER_USER2);	
	GET_STRING(flash_param->user_param[1].pwd, FLASH_USER_PWD2);
	GET_INT_STRING(flash_param->user_param[2].level, FLASH_USER_LEVEL3);
	GET_STRING(flash_param->user_param[2].user, FLASH_USER_USER3);	
	GET_STRING(flash_param->user_param[2].pwd, FLASH_USER_PWD3);    

	GET_STRING(flash_param->multi_param[0].alias, FLASH_MULTI_ALIAS1);
	GET_STRING(flash_param->multi_param[0].ip, FLASH_MULTI_IP1);
	GET_STRING(flash_param->multi_param[0].user, FLASH_MULTI_USER1);	
	GET_STRING(flash_param->multi_param[0].pwd, FLASH_MULTI_PWD1);
	GET_INT(flash_param->multi_param[0].port, FLASH_MULTI_PORT1); 
	GET_STRING(flash_param->multi_param[1].alias, FLASH_MULTI_ALIAS2);
	GET_STRING(flash_param->multi_param[1].ip, FLASH_MULTI_IP2);
	GET_STRING(flash_param->multi_param[1].user, FLASH_MULTI_USER2);	
	GET_STRING(flash_param->multi_param[1].pwd, FLASH_MULTI_PWD2);
	GET_INT(flash_param->multi_param[1].port, FLASH_MULTI_PORT2);      
	GET_STRING(flash_param->multi_param[2].alias, FLASH_MULTI_ALIAS3);    
	GET_STRING(flash_param->multi_param[2].ip, FLASH_MULTI_IP3);
	GET_STRING(flash_param->multi_param[2].user, FLASH_MULTI_USER3);	
	GET_STRING(flash_param->multi_param[2].pwd, FLASH_MULTI_PWD3);
	GET_INT(flash_param->multi_param[2].port, FLASH_MULTI_PORT3);      
	GET_STRING(flash_param->multi_param[3].alias, FLASH_MULTI_ALIAS4);    
	GET_STRING(flash_param->multi_param[3].ip, FLASH_MULTI_IP4);
	GET_STRING(flash_param->multi_param[3].user, FLASH_MULTI_USER4);	
	GET_STRING(flash_param->multi_param[3].pwd, FLASH_MULTI_PWD4);
	GET_INT(flash_param->multi_param[3].port, FLASH_MULTI_PORT4);  
	GET_STRING(flash_param->multi_param[4].alias, FLASH_MULTI_ALIAS5);    
	GET_STRING(flash_param->multi_param[4].ip, FLASH_MULTI_IP5);
	GET_STRING(flash_param->multi_param[4].user, FLASH_MULTI_USER5);	
	GET_STRING(flash_param->multi_param[4].pwd, FLASH_MULTI_PWD5);
	GET_INT(flash_param->multi_param[4].port, FLASH_MULTI_PORT5);      
	GET_STRING(flash_param->multi_param[5].alias, FLASH_MULTI_ALIAS6);    
	GET_STRING(flash_param->multi_param[5].ip, FLASH_MULTI_IP6);
	GET_STRING(flash_param->multi_param[5].user, FLASH_MULTI_USER6);	
	GET_STRING(flash_param->multi_param[5].pwd, FLASH_MULTI_PWD6);
	GET_INT(flash_param->multi_param[5].port, FLASH_MULTI_PORT6); 
    
	GET_STRING(flash_param->multi_param[6].alias, FLASH_MULTI_ALIAS7);    
	GET_STRING(flash_param->multi_param[6].ip, FLASH_MULTI_IP7);
	GET_STRING(flash_param->multi_param[6].user, FLASH_MULTI_USER7);	
	GET_STRING(flash_param->multi_param[6].pwd, FLASH_MULTI_PWD7);
	GET_INT(flash_param->multi_param[6].port, FLASH_MULTI_PORT7);      
	GET_STRING(flash_param->multi_param[7].alias, FLASH_MULTI_ALIAS8);    
	GET_STRING(flash_param->multi_param[7].ip, FLASH_MULTI_IP8);
	GET_STRING(flash_param->multi_param[7].user, FLASH_MULTI_USER8);	
	GET_STRING(flash_param->multi_param[7].pwd, FLASH_MULTI_PWD8);
	GET_INT(flash_param->multi_param[7].port, FLASH_MULTI_PORT8);      
	GET_STRING(flash_param->multi_param[8].alias, FLASH_MULTI_ALIAS9);    
	GET_STRING(flash_param->multi_param[8].ip, FLASH_MULTI_IP9);
	GET_STRING(flash_param->multi_param[8].user, FLASH_MULTI_USER9);	
	GET_STRING(flash_param->multi_param[8].pwd, FLASH_MULTI_PWD9);	
	GET_INT(flash_param->multi_param[8].port, FLASH_MULTI_PORT9);      

	GET_STRING(flash_param->alarm_param.roi_str[0], FLASH_ALARM_ROI_STR);
	GET_INT_STRING(flash_param->alarm_param.enable[0], FLASH_ALARM_ENABLE);	
	GET_INT(flash_param->alarm_param.sensitivity[0], FLASH_ALARM_SENSITIVITY);	
	GET_INT(flash_param->alarm_param.alarm_pos, FLASH_ALARM_POS);
#if 0    
//	GET_STRING(flash_param->alarm_param.alarm_bit, FLASH_ALARM_BIT);	
#else
    for (i = 0; i < 30; i++)
    {
        sprintf(str, FLASH_ALARM_BIT "%d", i);
	    GET_INT(flash_param->alarm_param.alarm_bit[i], str);        
    }
#endif
	GET_INT_STRING(flash_param->alarm_param.ptz_enb, FLASH_ALARM_PTZ_ENB);	
	GET_INT_STRING(flash_param->alarm_param.sound_enb, FLASH_ALARM_SOUND_ENB);	
	GET_INT_STRING(flash_param->alarm_param.ftp_pic, FLASH_ALARM_FTP_PIC);	
	GET_INT_STRING(flash_param->alarm_param.email_pic, FLASH_ALARM_EMAIL_PIC);	
	GET_INT_STRING(flash_param->alarm_param.pic_to_sd, FLASH_ALARM_PIC_TO_SD);	
	GET_INT_STRING(flash_param->alarm_param.rec_to_sd, FLASH_ALARM_REC_TO_SD);		
	GET_INT(flash_param->alarm_param.pic_cnt, FLASH_ALARM_PIC_CNT);	
	GET_INT_STRING(flash_param->alarm_param.sch_enb, FLASH_ALARM_SCH_ENB);		
	GET_STRING(flash_param->alarm_param.sch_day[0], FLASH_ALARM_SCH_DAY1);	
	GET_STRING(flash_param->alarm_param.sch_day[1], FLASH_ALARM_SCH_DAY2);	
	GET_STRING(flash_param->alarm_param.sch_day[2], FLASH_ALARM_SCH_DAY3);	
	GET_STRING(flash_param->alarm_param.sch_day[3], FLASH_ALARM_SCH_DAY4);	
	GET_STRING(flash_param->alarm_param.sch_day[4], FLASH_ALARM_SCH_DAY5);	
	GET_STRING(flash_param->alarm_param.sch_day[5], FLASH_ALARM_SCH_DAY6);	
	GET_STRING(flash_param->alarm_param.sch_day[6], FLASH_ALARM_SCH_DAY7);	    

	GET_STRING(flash_param->alarm_param.alarm_rec_plan.start_time,FLASH_ALARM1_START_TIME);          
	GET_STRING(flash_param->alarm_param.alarm_rec_plan.end_time,FLASH_ALARM1_END_TIME);     
    	GET_INT_STRING(flash_param->alarm_param.alarm_rec_plan.day[0],FLASH_ALARM1_DAY1);      
	GET_INT_STRING(flash_param->alarm_param.alarm_rec_plan.day[1],FLASH_ALARM1_DAY2);      
	GET_INT_STRING(flash_param->alarm_param.alarm_rec_plan.day[2],FLASH_ALARM1_DAY3);      
	GET_INT_STRING(flash_param->alarm_param.alarm_rec_plan.day[3],FLASH_ALARM1_DAY4);      
	GET_INT_STRING(flash_param->alarm_param.alarm_rec_plan.day[4],FLASH_ALARM1_DAY5);      
	GET_INT_STRING(flash_param->alarm_param.alarm_rec_plan.day[5],FLASH_ALARM1_DAY6);          
	GET_INT_STRING(flash_param->alarm_param.alarm_rec_plan.day[6],FLASH_ALARM1_DAY7); 
	GET_INT(flash_param->alarm_param.alarm_rec_plan.stepoverflag,FLASH_ALARM1_STEPOVERFLAG); 

	GET_INT_STRING(flash_param->wifi_param.enable, FLASH_WIFI_ENABLE);
	GET_INT_STRING(flash_param->wifi_param.auth, FLASH_WIFI_AUTH);
	GET_INT_STRING(flash_param->wifi_param.enc, FLASH_WIFI_ENC);
	GET_STRING(flash_param->wifi_param.ssid, FLASH_WIFI_SSID);	
	GET_STRING(flash_param->wifi_param.key, FLASH_WIFI_KEY);	    

#if 0   /* 软件版本不能改变 */
	GET_STRING(flash_param->misc_param.soft_version, FLASH_MISC_SOFT_VERSION);
#endif    
	GET_STRING(flash_param->misc_param.name, FLASH_MISC_NAME);
	GET_INT_STRING(flash_param->misc_param.stream_index, FLASH_MISC_STREAM_INDEX);
	GET_INT_STRING(flash_param->misc_param.pwr_freq, FLASH_MISC_PWR_FREQ);
	GET_INT_STRING(flash_param->misc_param.ir_led, FLASH_MISC_IR_LED);
	GET_STRING(flash_param->misc_param.git_head, FLASH_MISC_GIT_HEAD);
	GET_INT(flash_param->misc_param.http_port, FLASH_MISC_HTTP_PORT);
	GET_INT(flash_param->misc_param.rtsp_port, FLASH_MISC_RTSP_PORT);
	GET_INT(flash_param->misc_param.onvif_port, FLASH_MISC_ONVIF_PORT);	
	GET_INT_STRING(flash_param->misc_param.p2p_enable, FLASH_MISC_P2P_ENABLE);		   
	GET_INT_STRING(flash_param->misc_param.upnp_enable, FLASH_MISC_UPNP_ENABLE);
	GET_STRING(flash_param->misc_param.ocx_rec_path, FLASH_MISC_OCX_REC_PATH);
	GET_STRING(flash_param->misc_param.osd, FLASH_MISC_OSD);   
	GET_INT(flash_param->misc_param.app_video_quality, FLASH_MISC_VIDEO_QUALITY);	

	GET_INT_STRING(flash_param->led_status_onoff.wifi_led_status, FLASH_WIFI_STATUS_LED);
    	GET_INT_STRING(flash_param->led_status_onoff.ir_led_status, FLASH_IR_STATUS_LED);
  	
	iniparser_freedict(Dictionary);

    if (path_exist("/mnt/sdcard/main_enter_test") == 0)
    {
    	Dictionary = iniparser_load(FACTORY_TEST_FILE);

        GET_STRING(flash_param->wifi_param.ssid, FLASH_WIFI_SSID);
        GET_STRING(flash_param->wifi_param.key, FLASH_WIFI_KEY);
        GET_INT_STRING(flash_param->net_param.dhcp, FLASH_NETWORK_DHCP);
        GET_STRING(flash_param->net_param.ip, FLASH_NETWORK_IP);
        GET_STRING(flash_param->net_param.netmask, FLASH_NETWORK_NETMASK);    
        GET_STRING(flash_param->net_param.gateway, FLASH_NETWORK_GATEWAY); 
    	iniparser_freedict(Dictionary);    
    }

	return 0;
}


int boot_flash_config(char *file, struct SYSTEM_PARAM_T *pSystemParam)
{
	int magic;

	if (check_file(file) == SDK_ERROR)
	{        
		default_flash_config(file, pSystemParam);
		LOG_WARN("checksum fail, default flash config\n");

		return SDK_OK;
	}

	/* magic is not match, default config */	
	magic = read_magic(file);
	if (magic != default_flash_param.magic_num)
	{
		default_flash_config(file, pSystemParam);
		LOG_WARN("magic[%d:%d] not match, maybe version is not match, default flash config\n",
                    magic, default_flash_param.magic_num);

		return SDK_OK;
	}

	if (boot_flash_config_real(file, pSystemParam) == SDK_ERROR)
	{
		default_flash_config(file, pSystemParam);
        LOG_INFO("boot_flash_config_real fail, default flash config\n");
		
		return SDK_OK;
	}

	return SDK_OK;
}

/* be careful */        
static char g_str[128] = {0};
static char *int_to_str(int value)
{
	sprintf(g_str, "%d", value);
	
	return g_str;
}
/* end */

#define     SET_STRING(str, value)      \
	iniparser_set(Dictionary, (char *)str, (char *)value);
#define     SET_INT(str, value)         \
    iniparser_set(Dictionary, (char *)str, int_to_str(value)); 
#define     SET_INT_STRING(str, value)                              \
    do                                                              \
    {                                                               \
        char *pStr = Index2Str(value);                                  \
                                                                        \
        if (pStr) strcpy(int_str, pStr);                                \
        else strcpy(int_str, "");                                       \
        iniparser_set(Dictionary, (char *)str, int_str);  \
    }while(0)

int save_flash_config(char *file, struct SYSTEM_PARAM_T *pSystemParam)
{
    int i;
	FILE *fp = NULL;
	dictionary	*Dictionary = NULL;
	struct FLASH_PARAM_T *flash_param;
	char int_str[STR_64_LEN] = {0}, str[STR_64_LEN] = {0};

	PARAM_OK(file && pSystemParam);

	flash_param = &pSystemParam->flash_param;

	/* create file before iniparser_load */
	Dictionary = dictionary_new(0);
	if (Dictionary == NULL)
	{
		LOG_ERROR("error at iniparser_load\n");
		return SDK_ERROR;
	}

	fp = fopen(file, "w+");
	if (fp == NULL)
	{
		LOG_ERROR("error at fopen file\n");
		return SDK_ERROR;
	}

	/* for MISC config */
	SET_STRING(FLASH_MAGIC, NULL);
	SET_INT(FLASH_MAGIC_NUM, flash_param->magic_num);

	SET_STRING(FLASH_FTP, NULL);	
	SET_STRING(FLASH_FTP_SERVER, flash_param->ftp_param.server);
	SET_INT(FLASH_FTP_PORT, flash_param->ftp_param.port);
	SET_STRING(FLASH_FTP_USER, flash_param->ftp_param.user);
	SET_STRING(FLASH_FTP_PWD, flash_param->ftp_param.pwd);
	SET_STRING(FLASH_FTP_UPLOAD_PATH, flash_param->ftp_param.upload_path);
	SET_INT_STRING(FLASH_FTP_MODE, flash_param->ftp_param.mode);	    /* ? error */
	SET_INT_STRING(FLASH_FTP_AUTOCOVER, flash_param->ftp_param.autocover);    

	SET_STRING(FLASH_EMAIL, NULL);	
	SET_STRING(FLASH_EMAIL_SERVER, flash_param->email_param.server);
	SET_STRING(FLASH_EMAIL_SENDER, flash_param->email_param.sender);
	SET_STRING(FLASH_EMAIL_USER, flash_param->email_param.user);    
	SET_STRING(FLASH_EMAIL_PWD, flash_param->email_param.pwd);	
	SET_STRING(FLASH_EMAIL_RECV1, flash_param->email_param.recv1);
	SET_STRING(FLASH_EMAIL_RECV2, flash_param->email_param.recv2);
	SET_STRING(FLASH_EMAIL_RECV3, flash_param->email_param.recv3);
	SET_STRING(FLASH_EMAIL_RECV4, flash_param->email_param.recv4);    
	SET_INT(FLASH_EMAIL_PORT, flash_param->email_param.port);
	SET_INT_STRING(FLASH_EMAIL_MODE, flash_param->email_param.mode);
	SET_STRING(FLASH_EMAIL_SUBJECT, flash_param->email_param.subject);
	SET_STRING(FLASH_EMAIL_CONTEXT, flash_param->email_param.context);    

	SET_STRING(FLASH_VIDEO, NULL);	
	SET_INT_STRING(FLASH_VIDEO_RESOLUTION0, flash_param->video_param.stream[0].resolution);    
	SET_INT(FLASH_VIDEO_FPS0, flash_param->video_param.stream[0].fps);
	SET_INT(FLASH_VIDEO_IDR0, flash_param->video_param.stream[0].idr);
	SET_INT(FLASH_VIDEO_BITRATE0, flash_param->video_param.stream[0].bitrate);
	SET_INT_STRING(FLASH_VIDEO_RESOLUTION1, flash_param->video_param.stream[1].resolution);        
	SET_INT(FLASH_VIDEO_FPS1, flash_param->video_param.stream[1].fps);
	SET_INT(FLASH_VIDEO_IDR1, flash_param->video_param.stream[1].idr);
	SET_INT(FLASH_VIDEO_BITRATE1, flash_param->video_param.stream[1].bitrate);
	SET_INT_STRING(FLASH_VIDEO_RESOLUTION2, flash_param->video_param.stream[2].resolution);        
	SET_INT(FLASH_VIDEO_FPS2, flash_param->video_param.stream[2].fps);
	SET_INT(FLASH_VIDEO_IDR2, flash_param->video_param.stream[2].idr);
	SET_INT(FLASH_VIDEO_BITRATE2, flash_param->video_param.stream[2].bitrate);
	SET_INT_STRING(FLASH_VIDEO_FLIP, flash_param->video_param.flip);    
	SET_INT_STRING(FLASH_VIDEO_WIDEDYNAMIC, flash_param->video_param.wideDynamic);       
	SET_INT_STRING(FLASH_VIDEO_OSD_NAME_ENB, flash_param->video_param.osd_name_enb);
	SET_INT_STRING(FLASH_VIDEO_OSD_TIME_ENB, flash_param->video_param.osd_time_enb);	
	SET_INT(FLASH_VIDEO_BRIGHTNESS, flash_param->video_param.brightness);	    
	SET_INT(FLASH_VIDEO_CONTRAST, flash_param->video_param.contrast);
	SET_INT(FLASH_VIDEO_HUE, flash_param->video_param.hue);
	SET_INT(FLASH_VIDEO_SATURATION, flash_param->video_param.saturation);    
	SET_INT(FLASH_VIDEO_QUALITY, flash_param->video_param.quality);

	SET_STRING(FLASH_AUDIO, NULL);	
	SET_INT_STRING(FLASH_AUDIO_TYPE, flash_param->audio_param.type);
	SET_INT(FLASH_AUDIO_IN_VOL, flash_param->audio_param.in_vol);
	SET_INT(FLASH_AUDIO_OUT_VOL, flash_param->audio_param.out_vol);
	SET_INT_STRING(FLASH_AUDIO_MIC_ENABLE, flash_param->audio_param.mic_enable);
	SET_INT(FLASH_AUDIO_SAMPLERATE, flash_param->audio_param.samplerate);
	SET_INT(FLASH_AUDIO_BITRATE, flash_param->audio_param.bitrate);    
	SET_INT(FLASH_AUDIO_CHANNEL, flash_param->audio_param.channel);       

	SET_STRING(FLASH_DDNS, NULL);	
	SET_INT_STRING(FLASH_DDNS_ENABLE, flash_param->ddns_param.enable);    
	SET_INT_STRING(FLASH_DDNS_TYPE, flash_param->ddns_param.type);
	SET_STRING(FLASH_DDNS_DOMAIN, flash_param->ddns_param.domain);
	SET_STRING(FLASH_DDNS_SERVER, flash_param->ddns_param.server);
	SET_STRING(FLASH_DDNS_USER, flash_param->ddns_param.user);
	SET_STRING(FLASH_DDNS_PWD, flash_param->ddns_param.pwd);

	SET_STRING(FLASH_PTZ, NULL);	
	SET_INT_STRING(FLASH_PTZ_SPEED, flash_param->ptz_param.speed);
	SET_INT(FLASH_PTZ_START_POS, flash_param->ptz_param.start_pos);
	SET_INT(FLASH_PTZ_CIRCLE_CNT, flash_param->ptz_param.circle_cnt);    
	SET_INT(FLASH_PTZ_POS_X1, flash_param->ptz_param.motor_pos[0].x);
	SET_INT(FLASH_PTZ_POS_Y1, flash_param->ptz_param.motor_pos[0].y);	
	SET_INT(FLASH_PTZ_POS_X2, flash_param->ptz_param.motor_pos[1].x);
	SET_INT(FLASH_PTZ_POS_Y2, flash_param->ptz_param.motor_pos[1].y);	
	SET_INT(FLASH_PTZ_POS_X3, flash_param->ptz_param.motor_pos[2].x);
	SET_INT(FLASH_PTZ_POS_Y3, flash_param->ptz_param.motor_pos[2].y);	
	SET_INT(FLASH_PTZ_POS_X4, flash_param->ptz_param.motor_pos[3].x);
	SET_INT(FLASH_PTZ_POS_Y4, flash_param->ptz_param.motor_pos[3].y);	
	SET_INT(FLASH_PTZ_POS_X5, flash_param->ptz_param.motor_pos[4].x);
	SET_INT(FLASH_PTZ_POS_Y5, flash_param->ptz_param.motor_pos[4].y);	
	SET_INT(FLASH_PTZ_POS_X6, flash_param->ptz_param.motor_pos[5].x);
	SET_INT(FLASH_PTZ_POS_Y6, flash_param->ptz_param.motor_pos[5].y);	
	SET_INT(FLASH_PTZ_POS_X7, flash_param->ptz_param.motor_pos[6].x);
	SET_INT(FLASH_PTZ_POS_Y7, flash_param->ptz_param.motor_pos[6].y);	
	SET_INT(FLASH_PTZ_POS_X8, flash_param->ptz_param.motor_pos[7].x);
	SET_INT(FLASH_PTZ_POS_Y8, flash_param->ptz_param.motor_pos[7].y);	

	SET_STRING(FLASH_TIME, NULL);	
	SET_INT_STRING(FLASH_TIME_SYNC_TYPE, flash_param->time_param.sync_type);    
	SET_STRING(FLASH_TIME_NTP_SERVER, flash_param->time_param.ntp_server);
	SET_INT(FLASH_TIME_SYNC_INTERVAL, flash_param->time_param.sync_interval);	    
	SET_INT(FLASH_TIME_SAVING_TIME, flash_param->time_param.saving_time);	
	SET_INT_STRING(FLASH_TIME_ENB_SAVINGTIME, flash_param->time_param.enb_savingtime);    
	SET_INT_STRING(FLASH_TIME_TIMEZONE, flash_param->time_param.timezone);
	SET_STRING(FLASH_TIME_TIMEZONE_CITY, flash_param->time_param.timezone_city);	

	SET_STRING(FLASH_NETWORK, NULL);	
	SET_INT_STRING(FLASH_NETWORK_DHCP, flash_param->net_param.dhcp);    
	SET_STRING(FLASH_NETWORK_IP, flash_param->net_param.ip);
	SET_STRING(FLASH_NETWORK_NETMASK, flash_param->net_param.netmask);	
	SET_STRING(FLASH_NETWORK_GATEWAY, flash_param->net_param.gateway);
	SET_STRING(FLASH_NETWORK_DNS1, flash_param->net_param.dns1);	
	SET_STRING(FLASH_NETWORK_DNS2, flash_param->net_param.dns2);	
	SET_STRING(FLASH_NETWORK_MAC, flash_param->net_param.mac);    
	SET_STRING(FLASH_NETWORK_USED_INF, flash_param->net_param.used_inf);  

	SET_STRING(FLASH_RECORD, NULL);	
	SET_INT_STRING(FLASH_RECORD_ENABLE, flash_param->record_param.enable);        
	SET_INT_STRING(FLASH_RECORD_STREAM_CH, flash_param->record_param.stream_ch);      
	SET_INT(FLASH_RECORD_PEROID, flash_param->record_param.peroid);      
	SET_STRING(FLASH_RECORD_SCH1, flash_param->record_param.sch[0]);      
	SET_STRING(FLASH_RECORD_SCH2, flash_param->record_param.sch[1]);      
	SET_STRING(FLASH_RECORD_SCH3, flash_param->record_param.sch[2]);      
	SET_STRING(FLASH_RECORD_SCH4, flash_param->record_param.sch[3]);      
	SET_STRING(FLASH_RECORD_SCH5, flash_param->record_param.sch[4]);      
	SET_STRING(FLASH_RECORD_SCH6, flash_param->record_param.sch[5]);          
	SET_STRING(FLASH_RECORD_SCH7, flash_param->record_param.sch[6]); 
	SET_INT_STRING(FLASH_RECORD_DAY1, flash_param->record_param.day[0]);      
	SET_INT_STRING(FLASH_RECORD_DAY2, flash_param->record_param.day[1]);      
	SET_INT_STRING(FLASH_RECORD_DAY3, flash_param->record_param.day[2]);      
	SET_INT_STRING(FLASH_RECORD_DAY4, flash_param->record_param.day[3]);      
	SET_INT_STRING(FLASH_RECORD_DAY5, flash_param->record_param.day[4]);      
	SET_INT_STRING(FLASH_RECORD_DAY6, flash_param->record_param.day[5]);          
	SET_INT_STRING(FLASH_RECORD_DAY7, flash_param->record_param.day[6]); 
	SET_STRING(FLASH_RECORD_STARTTIME, flash_param->record_param.start_time);          
	SET_STRING(FLASH_RECORD_ENDTIME, flash_param->record_param.end_time);     

	SET_STRING(FLASH_RECORD1_START_TIME, flash_param->record_param.normal_rec_plan.start_time);          
	SET_STRING(FLASH_RECORD1_END_TIME, flash_param->record_param.normal_rec_plan.end_time);     
    	SET_INT_STRING(FLASH_RECORD1_DAY1, flash_param->record_param.normal_rec_plan.day[0]);      
	SET_INT_STRING(FLASH_RECORD1_DAY2, flash_param->record_param.normal_rec_plan.day[1]);      
	SET_INT_STRING(FLASH_RECORD1_DAY3, flash_param->record_param.normal_rec_plan.day[2]);      
	SET_INT_STRING(FLASH_RECORD1_DAY4, flash_param->record_param.normal_rec_plan.day[3]);      
	SET_INT_STRING(FLASH_RECORD1_DAY5, flash_param->record_param.normal_rec_plan.day[4]);      
	SET_INT_STRING(FLASH_RECORD1_DAY6, flash_param->record_param.normal_rec_plan.day[5]);          
	SET_INT_STRING(FLASH_RECORD1_DAY7, flash_param->record_param.normal_rec_plan.day[6]); 
	SET_INT(FLASH_RECORD1_STEPOVERFLAG,flash_param->record_param.normal_rec_plan.stepoverflag); 

	SET_STRING(FLASH_SNAP, NULL);	
	SET_INT_STRING(FLASH_SNAP_EANBLE, flash_param->snap_param.enable);        
	SET_INT_STRING(FLASH_SNAP_SD_ENABLE, flash_param->snap_param.sd_enable);   
	SET_INT(FLASH_SNAP_SD_PEROID, flash_param->snap_param.sd_peroid);   
	SET_INT_STRING(FLASH_SNAP_EMAIL_ENABLE, flash_param->snap_param.email_enable);       
	SET_INT(FLASH_SNAP_EMAIL_PEROID, flash_param->snap_param.email_peroid);     
	SET_INT_STRING(FLASH_SNAP_FTP_ENABLE, flash_param->snap_param.ftp_enable);       
	SET_INT(FLASH_SNAP_FTP_PEROID, flash_param->snap_param.ftp_peroid);   

	SET_STRING(FLASH_USER, NULL);	
	SET_INT_STRING(FLASH_USER_LEVEL1, flash_param->user_param[0].level);
	SET_STRING(FLASH_USER_USER1, flash_param->user_param[0].user);	
	SET_STRING(FLASH_USER_PWD1, flash_param->user_param[0].pwd);
	SET_INT_STRING(FLASH_USER_LEVEL2, flash_param->user_param[1].level);
	SET_STRING(FLASH_USER_USER2, flash_param->user_param[1].user);	
	SET_STRING(FLASH_USER_PWD2, flash_param->user_param[1].pwd);	
	SET_INT_STRING(FLASH_USER_LEVEL3, flash_param->user_param[2].level);
	SET_STRING(FLASH_USER_USER3, flash_param->user_param[2].user);	
	SET_STRING(FLASH_USER_PWD3, flash_param->user_param[2].pwd);		

	SET_STRING(FLASH_MULTI, NULL);	
	SET_STRING(FLASH_MULTI_ALIAS1, flash_param->multi_param[0].alias);    
	SET_STRING(FLASH_MULTI_IP1, flash_param->multi_param[0].ip);
	SET_STRING(FLASH_MULTI_USER1, flash_param->multi_param[0].user);	
	SET_STRING(FLASH_MULTI_PWD1, flash_param->multi_param[0].pwd);
	SET_INT(FLASH_MULTI_PORT1, flash_param->multi_param[0].port);    
	SET_STRING(FLASH_MULTI_ALIAS2, flash_param->multi_param[1].alias);        
	SET_STRING(FLASH_MULTI_IP2, flash_param->multi_param[1].ip);
	SET_STRING(FLASH_MULTI_USER2, flash_param->multi_param[1].user);	
	SET_STRING(FLASH_MULTI_PWD2, flash_param->multi_param[1].pwd);	
	SET_INT(FLASH_MULTI_PORT2, flash_param->multi_param[1].port);    
	SET_STRING(FLASH_MULTI_ALIAS3, flash_param->multi_param[2].alias);     
	SET_STRING(FLASH_MULTI_IP3, flash_param->multi_param[2].ip);
	SET_STRING(FLASH_MULTI_USER3, flash_param->multi_param[2].user);	
	SET_STRING(FLASH_MULTI_PWD3, flash_param->multi_param[2].pwd);	
	SET_INT(FLASH_MULTI_PORT3, flash_param->multi_param[2].port);    
	SET_STRING(FLASH_MULTI_ALIAS4, flash_param->multi_param[3].alias);      
	SET_STRING(FLASH_MULTI_IP4, flash_param->multi_param[3].ip);
	SET_STRING(FLASH_MULTI_USER4, flash_param->multi_param[3].user);	
	SET_STRING(FLASH_MULTI_PWD4, flash_param->multi_param[3].pwd);
	SET_INT(FLASH_MULTI_PORT4, flash_param->multi_param[3].port);    
	SET_STRING(FLASH_MULTI_ALIAS5, flash_param->multi_param[4].alias);     
	SET_STRING(FLASH_MULTI_IP5, flash_param->multi_param[4].ip);
	SET_STRING(FLASH_MULTI_USER5, flash_param->multi_param[4].user);	
	SET_STRING(FLASH_MULTI_PWD5, flash_param->multi_param[4].pwd);
	SET_INT(FLASH_MULTI_PORT5, flash_param->multi_param[4].port);    
	SET_STRING(FLASH_MULTI_ALIAS6, flash_param->multi_param[5].alias);     
	SET_STRING(FLASH_MULTI_IP6, flash_param->multi_param[5].ip);
	SET_STRING(FLASH_MULTI_USER6, flash_param->multi_param[5].user);	
	SET_STRING(FLASH_MULTI_PWD6, flash_param->multi_param[5].pwd);	
	SET_INT(FLASH_MULTI_PORT6, flash_param->multi_param[5].port);    
	SET_STRING(FLASH_MULTI_ALIAS7, flash_param->multi_param[6].alias);     
	SET_STRING(FLASH_MULTI_IP7, flash_param->multi_param[6].ip);
	SET_STRING(FLASH_MULTI_USER7, flash_param->multi_param[6].user);	
	SET_STRING(FLASH_MULTI_PWD7, flash_param->multi_param[6].pwd);
	SET_INT(FLASH_MULTI_PORT7, flash_param->multi_param[6].port);    
	SET_STRING(FLASH_MULTI_ALIAS8, flash_param->multi_param[7].alias);     
	SET_STRING(FLASH_MULTI_IP8, flash_param->multi_param[7].ip);
	SET_STRING(FLASH_MULTI_USER8, flash_param->multi_param[7].user);	
	SET_STRING(FLASH_MULTI_PWD8, flash_param->multi_param[7].pwd);	
	SET_INT(FLASH_MULTI_PORT8, flash_param->multi_param[7].port);    
	SET_STRING(FLASH_MULTI_ALIAS9, flash_param->multi_param[8].alias);     
	SET_STRING(FLASH_MULTI_IP9, flash_param->multi_param[8].ip);
	SET_STRING(FLASH_MULTI_USER9, flash_param->multi_param[8].user);	
	SET_STRING(FLASH_MULTI_PWD9, flash_param->multi_param[8].pwd);	
	SET_INT(FLASH_MULTI_PORT9, flash_param->multi_param[8].port);   

	SET_STRING(FLASH_WIFI, NULL);	
	SET_INT_STRING(FLASH_WIFI_ENABLE, flash_param->wifi_param.enable);    
	SET_INT_STRING(FLASH_WIFI_AUTH, flash_param->wifi_param.auth);

	SET_INT_STRING(FLASH_WIFI_ENC, flash_param->wifi_param.enc);
	SET_STRING(FLASH_WIFI_SSID, flash_param->wifi_param.ssid);	
	SET_STRING(FLASH_WIFI_KEY, flash_param->wifi_param.key);	

	SET_STRING(FLASH_ALARM, NULL);	  
	SET_STRING(FLASH_ALARM_ROI_STR, flash_param->alarm_param.roi_str[0]);
	SET_INT_STRING(FLASH_ALARM_ENABLE, flash_param->alarm_param.enable[0]);
	SET_INT(FLASH_ALARM_SENSITIVITY, flash_param->alarm_param.sensitivity[0]);	
	SET_INT(FLASH_ALARM_POS, flash_param->alarm_param.alarm_pos);	
	for (i = 0; i < 30; i++)
	{
		sprintf(str, FLASH_ALARM_BIT "%d", i);
		SET_INT(str, flash_param->alarm_param.alarm_bit[i]);            
	}
	    
	SET_INT_STRING(FLASH_ALARM_PTZ_ENB, flash_param->alarm_param.ptz_enb);
	SET_INT_STRING(FLASH_ALARM_SOUND_ENB, flash_param->alarm_param.sound_enb);	
	SET_INT_STRING(FLASH_ALARM_FTP_PIC, flash_param->alarm_param.ftp_pic);	
	SET_INT_STRING(FLASH_ALARM_EMAIL_PIC, flash_param->alarm_param.email_pic);	
	SET_INT_STRING(FLASH_ALARM_PIC_TO_SD, flash_param->alarm_param.pic_to_sd);	
	SET_INT_STRING(FLASH_ALARM_REC_TO_SD, flash_param->alarm_param.rec_to_sd);	
	SET_INT(FLASH_ALARM_PIC_CNT, flash_param->alarm_param.pic_cnt);	
	SET_INT_STRING(FLASH_ALARM_SCH_ENB, flash_param->alarm_param.sch_enb);	
	SET_STRING(FLASH_ALARM_SCH_DAY1, flash_param->alarm_param.sch_day[0]);	
	SET_STRING(FLASH_ALARM_SCH_DAY2, flash_param->alarm_param.sch_day[1]);	
	SET_STRING(FLASH_ALARM_SCH_DAY3, flash_param->alarm_param.sch_day[2]);	
	SET_STRING(FLASH_ALARM_SCH_DAY4, flash_param->alarm_param.sch_day[3]);	
	SET_STRING(FLASH_ALARM_SCH_DAY5, flash_param->alarm_param.sch_day[4]);	
	SET_STRING(FLASH_ALARM_SCH_DAY6, flash_param->alarm_param.sch_day[5]);	
	SET_STRING(FLASH_ALARM_SCH_DAY7, flash_param->alarm_param.sch_day[6]);	    

	SET_STRING(FLASH_ALARM1_START_TIME, flash_param->alarm_param.alarm_rec_plan.start_time);          
	SET_STRING(FLASH_ALARM1_END_TIME, flash_param->alarm_param.alarm_rec_plan.end_time);     
    	SET_INT_STRING(FLASH_ALARM1_DAY1, flash_param->alarm_param.alarm_rec_plan.day[0]);      
	SET_INT_STRING(FLASH_ALARM1_DAY2, flash_param->alarm_param.alarm_rec_plan.day[1]);      
	SET_INT_STRING(FLASH_ALARM1_DAY3, flash_param->alarm_param.alarm_rec_plan.day[2]);      
	SET_INT_STRING(FLASH_ALARM1_DAY4, flash_param->alarm_param.alarm_rec_plan.day[3]);      
	SET_INT_STRING(FLASH_ALARM1_DAY5, flash_param->alarm_param.alarm_rec_plan.day[4]);      
	SET_INT_STRING(FLASH_ALARM1_DAY6, flash_param->alarm_param.alarm_rec_plan.day[5]);          
	SET_INT_STRING(FLASH_ALARM1_DAY7, flash_param->alarm_param.alarm_rec_plan.day[6]); 
	SET_INT(FLASH_ALARM1_STEPOVERFLAG, flash_param->alarm_param.alarm_rec_plan.stepoverflag);
	
	SET_STRING(FLASH_MISC, NULL);	
#if 0      /* 软件版本不能改变 */ 
	SET_STRING(FLASH_MISC_SOFT_VERSION, flash_param->misc_param.soft_version);	
#endif    
	SET_STRING(FLASH_MISC_NAME, flash_param->misc_param.name);		
	SET_INT_STRING(FLASH_MISC_STREAM_INDEX, flash_param->misc_param.stream_index);
	SET_INT_STRING(FLASH_MISC_PWR_FREQ, flash_param->misc_param.pwr_freq);	
	SET_INT_STRING(FLASH_MISC_IR_LED, flash_param->misc_param.ir_led);
	SET_STRING(FLASH_MISC_GIT_HEAD, flash_param->misc_param.git_head);		
	SET_INT(FLASH_MISC_HTTP_PORT, flash_param->misc_param.http_port);	
	SET_INT(FLASH_MISC_RTSP_PORT, flash_param->misc_param.rtsp_port);
	SET_INT(FLASH_MISC_ONVIF_PORT, flash_param->misc_param.onvif_port);	
	SET_INT_STRING(FLASH_MISC_P2P_ENABLE, flash_param->misc_param.p2p_enable);   
	SET_INT_STRING(FLASH_MISC_UPNP_ENABLE, flash_param->misc_param.upnp_enable);	
	SET_STRING(FLASH_MISC_OCX_REC_PATH, flash_param->misc_param.ocx_rec_path);	     
	SET_STRING(FLASH_MISC_OSD, flash_param->misc_param.osd);    
	SET_INT(FLASH_MISC_VIDEO_QUALITY, flash_param->misc_param.app_video_quality);	

	SET_STRING(FLASH_STATUS_LED,NULL);
	SET_INT_STRING(FLASH_WIFI_STATUS_LED, flash_param->led_status_onoff.wifi_led_status);
	SET_INT_STRING(FLASH_IR_STATUS_LED, flash_param->led_status_onoff.ir_led_status);

	iniparser_dump_ini(Dictionary, fp);
	dictionary_del(Dictionary);
    
	fclose(fp);

	add_checksum(file);
	copy_to_backfile(file);

	return 0;
}

int default_flash_config(char *file, struct SYSTEM_PARAM_T *pSystemParam)
{
	struct FLASH_PARAM_T *flash_param;    

    PARAM_OK(pSystemParam);
    
	flash_param = &pSystemParam->flash_param;
	memcpy(flash_param, &default_flash_param, sizeof(FLASH_PARAM));
	unlink(FLASH_CONFIG_FILE);	
	save_flash_config(FLASH_CONFIG_FILE, pSystemParam);

	return SDK_OK;

}
/* -------------------------  end for flash param  ------------------------- */

/* -------------------------  for factory param  ------------------------- */
int boot_factory_config_real(char *file, struct SYSTEM_PARAM_T *system_param)
{
	dictionary	*Dictionary = NULL;
    char *pStr = NULL;
	struct FACTORY_PARAM_T *pFactoryParam;

	PARAM_OK(file && system_param);

	pFactoryParam = &system_param->factory_param;	
	
	memcpy(pFactoryParam, &default_factory_param, sizeof(default_factory_param));
	Dictionary = iniparser_load(file);
	if (Dictionary == NULL)
	{
		LOG_ERROR("error at iniparser_load\n");
		return SDK_ERROR;
	}

	GET_INT(pFactoryParam->magic_num, FACTORY_MAGIC_NUM);
	GET_STRING(pFactoryParam->model, FACTORY_MISC_MODEL);   
	GET_STRING(pFactoryParam->hard_version, FACTORY_MISC_HARD_VERSION);    
	GET_INT_STRING(pFactoryParam->p2p_type, FACTORY_MISC_P2P_TYPE);  
	GET_STRING(pFactoryParam->oem, FACTORY_MISC_OEM);    
	GET_STRING(pFactoryParam->uid, FACTORY_MISC_UID);
	GET_STRING(pFactoryParam->sn, FACTORY_MISC_SN);    
	GET_INT(pFactoryParam->limit_x, FACTORY_MISC_X_LIMIT);
	GET_INT(pFactoryParam->limit_y, FACTORY_MISC_Y_LIMIT);   
	GET_STRING(pFactoryParam->region, FACTORY_MISC_REGION);    
	GET_STRING(pFactoryParam->version_desc, FACTORY_MISC_VERSION_DES);    

	GET_INT_STRING(pFactoryParam->ddns_param.type, FACTORY_DDNS_TYPE);	
    if (pFactoryParam->ddns_param.type == DDNS_TWS)
    {
    	GET_STRING(pFactoryParam->ddns_param.TvsDdnsInfo.ddns_server, FACTORY_TWS_DDNS_SERVER);	
    	GET_STRING(pFactoryParam->ddns_param.TvsDdnsInfo.user, FACTORY_TWS_DDNS_USER);	
    	GET_STRING(pFactoryParam->ddns_param.TvsDdnsInfo.pwd, FACTORY_TWS_DDNS_PWD);			
    	GET_STRING(pFactoryParam->ddns_param.TvsDdnsInfo.acod, FACTORY_TWS_DDNS_ACOD);			    
    }
    else if (pFactoryParam->ddns_param.type == DDNS_EASYN)
    {
    	GET_STRING(pFactoryParam->ddns_param.EasynDdnsInfo.ddns_server, FACTORY_EASYN_DDNS_SERVER);	
    	GET_STRING(pFactoryParam->ddns_param.EasynDdnsInfo.user, FACTORY_EASYN_DDNS_USER);	
    	GET_STRING(pFactoryParam->ddns_param.EasynDdnsInfo.pwd, FACTORY_EASYN_DDNS_PWD);			
    }

	iniparser_freedict(Dictionary);

	return 0;
}


int boot_factory_config(char *file, struct SYSTEM_PARAM_T *pSystemParam)
{
	int magic;
	LOG_INFO("ready into check_file FACTORY_CONFIG_FILE\n");

	if (check_file(FACTORY_CONFIG_FILE) == SDK_ERROR)
	{
		LOG_WARN("check config file[%s] error, default factory config\n", FACTORY_CONFIG_FILE);        
		default_factory_config(file, pSystemParam);
		return SDK_OK;
	}

	/* magic is not match, default config */
	magic = read_magic(FACTORY_CONFIG_FILE);
	if (magic != default_factory_param.magic_num)
	{
		LOG_WARN("magic not match, maybe version is not match, default factory config\n");        
		default_factory_config(file, pSystemParam);

		return SDK_OK;
	}

	/* param is not match, default config */
	if (boot_factory_config_real(FACTORY_CONFIG_FILE, pSystemParam) == SDK_ERROR)
	{
		LOG_WARN("Param not match, maybe version is not match, default factory config\n");        
		default_factory_config(file, pSystemParam);
		
		return SDK_OK;
	}

	return SDK_OK;	
}

int save_factory_config(char *file, struct SYSTEM_PARAM_T *pSystemParam)
{
    char int_str[STR_64_LEN] = {0};
	FILE *fp = NULL;
	dictionary	*Dictionary = NULL;
	struct FACTORY_PARAM_T *pFactory_param;

	PARAM_OK(file && pSystemParam);

	pFactory_param = &pSystemParam->factory_param;

	/* create file before iniparser_load */
	Dictionary = dictionary_new(0);
	if (Dictionary == NULL)
	{
		LOG_ERROR("error at iniparser_load\n");
		return SDK_ERROR;
	}

	fp = fopen(file, "w+");
	if (fp == NULL)
	{
		LOG_ERROR("error at fopen file\n");
		return SDK_ERROR;
	}

	SET_STRING(FACTORY_MAGIC, NULL);	
	SET_INT(FACTORY_MAGIC_NUM, pFactory_param->magic_num);
	SET_STRING(FACTORY_MISC, NULL); 
	SET_STRING(FACTORY_MISC_MODEL, pFactory_param->model);    
	SET_STRING(FACTORY_MISC_HARD_VERSION, pFactory_param->hard_version);	 
	SET_INT_STRING(FACTORY_MISC_P2P_TYPE, pFactory_param->p2p_type);  
	SET_STRING(FACTORY_MISC_OEM, pFactory_param->oem);       
	SET_STRING(FACTORY_MISC_UID, pFactory_param->uid);
    SET_STRING(FACTORY_MISC_SN, pFactory_param->sn);
	SET_INT(FACTORY_MISC_X_LIMIT, pFactory_param->limit_x);
	SET_INT(FACTORY_MISC_Y_LIMIT, pFactory_param->limit_y); 
	SET_STRING(FACTORY_MISC_REGION, pFactory_param->region);       
	SET_STRING(FACTORY_MISC_VERSION_DES, pFactory_param->version_desc);    
	SET_STRING(FACTORY_DDNS, NULL);	

	SET_INT_STRING(FACTORY_DDNS_TYPE, pFactory_param->ddns_param.type);
    if (pFactory_param->ddns_param.type == DDNS_TWS)
    {	
    	SET_STRING(FACTORY_TWS_DDNS, NULL);	        
    	SET_STRING(FACTORY_TWS_DDNS_SERVER, pFactory_param->ddns_param.TvsDdnsInfo.ddns_server);    
    	SET_STRING(FACTORY_TWS_DDNS_USER, pFactory_param->ddns_param.TvsDdnsInfo.user);		
    	SET_STRING(FACTORY_TWS_DDNS_PWD, pFactory_param->ddns_param.TvsDdnsInfo.pwd);	  
    	SET_STRING(FACTORY_TWS_DDNS_ACOD, pFactory_param->ddns_param.TvsDdnsInfo.acod);	    
    }
    else if (pFactory_param->ddns_param.type == DDNS_EASYN)
    {
    	SET_STRING(FACTORY_EASYN_DDNS, NULL);	        
    	SET_STRING(FACTORY_EASYN_DDNS_SERVER, pFactory_param->ddns_param.EasynDdnsInfo.ddns_server);    
    	SET_STRING(FACTORY_EASYN_DDNS_USER, pFactory_param->ddns_param.EasynDdnsInfo.user);		
    	SET_STRING(FACTORY_EASYN_DDNS_PWD, pFactory_param->ddns_param.EasynDdnsInfo.pwd);	  	
    }

	iniparser_dump_ini(Dictionary, fp);
	dictionary_del(Dictionary);
	fclose(fp);

	add_checksum(file);
	copy_to_backfile(file);

	return 0;

}

int default_factory_config(char *file, struct SYSTEM_PARAM_T *pSystemParam)
{
	struct FACTORY_PARAM_T *pFactoryParam;

	PARAM_OK(pSystemParam);	

	pFactoryParam = &pSystemParam->factory_param;
	memcpy(pFactoryParam, &default_factory_param, sizeof(FACTORY_PARAM));
	unlink(file);	
	save_factory_config(file, pSystemParam);

	return SDK_OK;
}

/* -------------------------  end for factory param  ------------------------- */

int sys_boot_config(void)
{
    SYSTEM_PARAM *pSystemParam;
   
    pSystemParam = (struct SYSTEM_PARAM_T *)system_param_map(0);
    LOG_INFO("Done system_param_map\n"); 
    if (pSystemParam == NULL) 
    {
	LOG_ERROR("error at system_param_map\n");
	return -1;
    }
  
    /* 三种参数的加载顺序不能调换, 因为ram参数会根据flash参数变化 */
    boot_factory_config(FACTORY_CONFIG_FILE, pSystemParam);
    LOG_INFO("Done boot_factory_config\n");
    boot_flash_config(FLASH_CONFIG_FILE, pSystemParam);
    LOG_INFO("Done boot_flash_config\n");
    boot_ram_config(pSystemParam);
    LOG_INFO("Done boot_ram_config\n");

	return SDK_OK;
}

int sys_default_config(void)
{
    SYSTEM_PARAM *pSystemParam;
    
    pSystemParam = (struct SYSTEM_PARAM_T *)system_param_map(0);
	if (pSystemParam == NULL)
	{
		LOG_ERROR("error at system_param_map\n");
		return -1;
	} 
    default_flash_config(FLASH_CONFIG_FILE, pSystemParam);

    return SDK_OK;
}

int sys_save_flash_config(void)
{
    SYSTEM_PARAM *pSystemParam;

    LOG_INFO("===  sys_save_flash_config  ===\n");
    pSystemParam = (struct SYSTEM_PARAM_T *)system_param_map(0);
	if (pSystemParam == NULL)
	{
		LOG_ERROR("error at system_param_map\n");
		return -1;
	} 
    save_flash_config(FLASH_CONFIG_FILE, pSystemParam);

    return SDK_OK;
}

int sys_save_factory_config(void)
{
    SYSTEM_PARAM *pSystemParam;

    LOG_INFO("===  sys_save_factory_config  ===\n");
    pSystemParam = (struct SYSTEM_PARAM_T *)system_param_map(0);
	if (pSystemParam == NULL)
	{
		LOG_ERROR("error at system_param_map\n");
		return -1;
	} 
    save_factory_config(FACTORY_CONFIG_FILE, pSystemParam);

    return SDK_OK;
}

