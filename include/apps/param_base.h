#ifndef      __PARAM_BASE_H__
#define     __PARAM_BASE_H__

#ifdef	__cplusplus
extern "C" { 
#endif

#include "param.h"

FTP_PARAM *get_ftp_param(void);
ALARM_PARAM *get_alarm_param(void);
EMAIL_PARAM *get_email_param(void);
VIDEO_PARAM *get_video_param(void);
AUDIO_PARAM *get_audio_param(void);
PTZ_PARAM *get_ptz_param(void);
RECORD_PARAM *get_record_param(void);
SNAP_PARAM *get_snap_param(void);
WIFI_PARAM *get_wifi_param(void);
NET_PARAM *get_net_param(void);
NET_PARAM *get_real_net_param(void);
INNER_DDNS_PARAM *get_ddns_param(void);
DDNS_PARAM *get_thr_ddns_param(void);
TIME_PARAM *get_time_param(void);
USER_PARAM *get_user_param(void);
MULTI_PARAM *get_multi_param(void);
MISC_PARAM *get_misc_param(void);
RTMP_PARAM *get_rtmp_param(void);
LED_STATUS_ONOFF *get_led_status_onoff_param(void);
RAM_PARAM *get_ram_param(void);
FACTORY_PARAM *get_factory_param(void);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif  /* __COMMON_H__ */
