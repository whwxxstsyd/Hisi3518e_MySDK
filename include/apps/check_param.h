#ifndef     __CHECK_PARAM_H__
#define     __CHECK_PARAM_H__

#include "param.h"

#ifdef		__cplusplus
extern "C" {
#endif


int check_ftp_param(FTP_PARAM *pFtpParam);
int check_email_param(EMAIL_PARAM *pEmailParam);
int check_alarm_param(ALARM_PARAM *pAlarmParam);
int check_video_param(VIDEO_PARAM *pVideoParam);
int check_audio_param(AUDIO_PARAM *pAudioParam);
int check_net_param(NET_PARAM *pNetParam);
int check_wifi_param(WIFI_PARAM *pWifiParam);
int check_thr_ddns_param(DDNS_PARAM *pDdnsParam);
int check_time_param(TIME_PARAM *pTimeParam);
int check_user_param(USER_PARAM *pUserParam);
int check_multi_param(MULTI_PARAM *pMultiParam);
int check_snap_param(SNAP_PARAM *pSnapParam);
int check_record_param(RECORD_PARAM *pRecordParam);
int check_ptz_param(PTZ_PARAM *pPtzParam);
int check_misc_param(MISC_PARAM *pMiscParam);


#ifdef		__cplusplus
}
#endif

#endif
