#include "param.h"
#include "check_param.h"

int check_ftp_param(FTP_PARAM *pFtpParam)
{
    if (((pFtpParam->port < MIN_PORT) || (pFtpParam->port > MAX_PORT)) ||
        ((pFtpParam->mode != PASV_MODE) && (pFtpParam->mode != PORT_MODE)))
        return SDK_ERROR;
    
    return SDK_OK;
}

int check_email_param(EMAIL_PARAM *pEmailParam)
{
    if (((pEmailParam->port < MIN_PORT) || (pEmailParam->port > MAX_PORT)) ||
        ((pEmailParam->mode != EMAIL_TLS) && (pEmailParam->mode != EMAIL_SSL)))
        return SDK_ERROR;
    
    return SDK_OK;
}

int check_alarm_param(ALARM_PARAM *pAlarmParam)
{
    if (((pAlarmParam->enable[0] != ON) && (pAlarmParam->enable[0] != OFF)) ||
        ((pAlarmParam->enable[1] != ON) && (pAlarmParam->enable[1] != OFF)) ||
        ((pAlarmParam->enable[2] != ON) && (pAlarmParam->enable[2] != OFF)) ||
        ((pAlarmParam->enable[3] != ON) && (pAlarmParam->enable[3] != OFF)) ||
        ((pAlarmParam->sensitivity[0] <= 0) || (pAlarmParam->sensitivity[0] > 100)) ||
        ((pAlarmParam->sensitivity[1] <= 0) || (pAlarmParam->sensitivity[1] > 100)) ||
        ((pAlarmParam->sensitivity[2] <= 0) || (pAlarmParam->sensitivity[2] > 100)) ||
        ((pAlarmParam->sensitivity[3] <= 0) || (pAlarmParam->sensitivity[3] > 100)) ||  
        ((pAlarmParam->alarm_pos < MIN_POS) || (pAlarmParam->alarm_pos > MAX_POS)) ||
        ((pAlarmParam->ptz_enb != ON) && (pAlarmParam->ptz_enb != OFF)) ||
        ((pAlarmParam->sound_enb != ON) && (pAlarmParam->sound_enb != OFF)) ||
        ((pAlarmParam->ftp_pic != ON) && (pAlarmParam->ftp_pic != OFF)) ||
        ((pAlarmParam->email_pic != ON) && (pAlarmParam->email_pic != OFF)) ||        
        ((pAlarmParam->pic_to_sd != ON) && (pAlarmParam->pic_to_sd != OFF)) ||  
        ((pAlarmParam->rec_to_sd != ON) && (pAlarmParam->rec_to_sd != OFF)) ||  
        ((pAlarmParam->rec_to_ftp != ON) && (pAlarmParam->rec_to_ftp != OFF)) ||          
        ((pAlarmParam->pic_cnt <= 0) || (pAlarmParam->rec_to_ftp > 5)) ||         
        ((pAlarmParam->sch_enb != ON) && (pAlarmParam->sch_enb != OFF)))
        return SDK_ERROR;
    
    return SDK_OK;
}

int check_stream_param(STREAM_PARAM *pStreamParam)
{
    if (((pStreamParam->resolution < QVGA) || (pStreamParam->resolution > MAX_RESOLUTION)) ||
        ((pStreamParam->fps < 1) || (pStreamParam->fps > MAX_FPS)) || 
        ((pStreamParam->idr < 15) || (pStreamParam->idr > 50)) ||         
        ((pStreamParam->bitrate < 128) || (pStreamParam->idr > 1024 * 8)))
        return SDK_ERROR;

    return SDK_OK;
}

int check_video_param(VIDEO_PARAM *pVideoParam)
{
    if ((check_stream_param(&pVideoParam->stream[0]) == SDK_ERROR) || 
        (check_stream_param(&pVideoParam->stream[1]) == SDK_ERROR) ||
        (check_stream_param(&pVideoParam->stream[2]) == SDK_ERROR) ||
        ((pVideoParam->flip < FLIP_NORMAL) || (pVideoParam->flip > FLIP_HV)) ||
        ((pVideoParam->wideDynamic != ON) && (pVideoParam->wideDynamic != OFF)) ||        
        ((pVideoParam->osd_name_enb != ON) && (pVideoParam->osd_name_enb != OFF)) ||         
        ((pVideoParam->osd_time_enb != ON) && (pVideoParam->osd_time_enb != OFF)) ||          
        ((pVideoParam->brightness < 1) || (pVideoParam->brightness > 100)) ||        
        ((pVideoParam->contrast < 1) || (pVideoParam->contrast > 100)) ||        
        ((pVideoParam->hue < 1) || (pVideoParam->hue > 100)) ||        
        ((pVideoParam->saturation < 1) || (pVideoParam->saturation > 100)))
        return SDK_ERROR;
    
    return SDK_OK;
}

int check_audio_param(AUDIO_PARAM *pAudioParam)
{
    if (((pAudioParam->type != AUDIO_TYPE_G711) && (pAudioParam->type != AUDIO_TYPE_G726)) ||
        ((pAudioParam->in_vol < MIN_VOL) || (pAudioParam->in_vol > MAX_VOL)) || 
        ((pAudioParam->out_vol < MIN_VOL) || (pAudioParam->out_vol > MAX_VOL)) ||         
        ((pAudioParam->mic_enable != ON) && (pAudioParam->mic_enable != OFF)) ||         
        ((pAudioParam->samplerate != 8000) && (pAudioParam->samplerate != 16000)) ||            
        (pAudioParam->bitwidth != 16) ||          
        ((pAudioParam->bitrate < 0) || (pAudioParam->bitrate > 2 * 1024)) ||
        (pAudioParam->channel != 1))   
        return SDK_ERROR;
    
    return SDK_OK;
}

int check_net_param(NET_PARAM *pNetParam)
{
    if (((pNetParam->dhcp != ON) && (pNetParam->dhcp != OFF)) ||
        ((pNetParam->used_inf != USE_ETH) && (pNetParam->used_inf != USE_WLAN)))
        return SDK_ERROR;        
    
    return SDK_OK;
}

int check_wifi_param(WIFI_PARAM *pWifiParam)
{
    if (((pWifiParam->enable != ON) && (pWifiParam->enable != OFF)) ||
        ((pWifiParam->auth < AUTH_WEP) || (pWifiParam->auth > AUTH_OPEN)) ||
        ((pWifiParam->enc < ENC_NONE) || (pWifiParam->enc > ENC_TKIP))) 
        return SDK_ERROR;         
    
    return SDK_OK;
}

int check_thr_ddns_param(DDNS_PARAM *pDdnsParam)
{
    if (((pDdnsParam->enable != ON) && (pDdnsParam->enable != OFF)) ||
        ((pDdnsParam->type < DDNS_9299) && (pDdnsParam->type > DDNS_3322)))
        return SDK_ERROR;                 
    
    return SDK_OK;
}

int check_time_param(TIME_PARAM *pTimeParam)
{
    if (((pTimeParam->sync_type < NORMAL_SYNC) || (pTimeParam->sync_type < NTP_SYNC)) ||
        ((pTimeParam->sync_interval < 1) || (pTimeParam->sync_interval > 100)) ||
        ((pTimeParam->saving_time < 15) || (pTimeParam->saving_time > 60 * 8)) ||        
        ((pTimeParam->enb_savingtime != ON) && (pTimeParam->enb_savingtime != OFF)) ||         
        ((pTimeParam->timezone < GMT12) || (pTimeParam->enb_savingtime > GMT_12)))  
        return SDK_ERROR;          
    
    return SDK_OK;
}

int check_user_param(USER_PARAM *pUserParam)
{
    if ((pUserParam->level < USER_ADMIN) || (pUserParam->level < USER_GUEST))
        return SDK_ERROR;                  
    
    return SDK_OK;
}

int check_multi_param(MULTI_PARAM *pMultiParam)
{
    if ((pMultiParam->port > MIN_PORT) || (pMultiParam->port > MAX_PORT))
        return SDK_ERROR;         
    
    return SDK_OK;
}

int check_snap_param(SNAP_PARAM *pSnapParam)
{
    if (((pSnapParam->enable != ON) && (pSnapParam->enable != OFF)) ||
        ((pSnapParam->sd_enable != ON) && (pSnapParam->sd_enable != OFF)) ||
        ((pSnapParam->sd_peroid < 1) && (pSnapParam->sd_enable > 1000)) ||   
        ((pSnapParam->email_enable != ON) && (pSnapParam->email_enable != OFF)) ||
        ((pSnapParam->email_peroid < 1) && (pSnapParam->email_peroid > 1000)) ||  
        ((pSnapParam->ftp_enable != ON) && (pSnapParam->ftp_enable != OFF)) ||
        ((pSnapParam->ftp_peroid < 1) && (pSnapParam->ftp_peroid > 1000)))  
        return SDK_ERROR;          
    
    return SDK_OK;
}

int check_record_param(RECORD_PARAM *pRecordParam)
{
    if (((pRecordParam->enable != ON) && (pRecordParam->enable != OFF)) || 
        ((pRecordParam->stream_ch < MAIN_STREAM_CH) || (pRecordParam->stream_ch > SUB3_STREAM_CH)) ||
        ((pRecordParam->peroid < 1) || (pRecordParam->peroid > 10 * 60)))
        return SDK_ERROR;          
    
    return SDK_OK;
}

int check_ptz_param(PTZ_PARAM *pPtzParam)
{
    if (((pPtzParam->speed < SPEED_LOW) || (pPtzParam->speed > SPEED_HIGH)) ||
        ((pPtzParam->start_pos < 0) || (pPtzParam->start_pos > 8)) ||
        ((pPtzParam->circle_cnt < 1) || (pPtzParam->circle_cnt > 50)))   
        return SDK_ERROR;               
    
    return SDK_OK;
}

int check_misc_param(MISC_PARAM *pMiscParam)
{
    if (((pMiscParam->stream_index < MAIN_STREAM_CH) || (pMiscParam->stream_index > SUB3_STREAM_CH)) ||
        ((pMiscParam->pwr_freq < F50HZ) || (pMiscParam->pwr_freq > F60HZ)) ||
        ((pMiscParam->ir_led < IRLED_AUTO) || (pMiscParam->ir_led > IRLED_OFF)) ||        
        ((pMiscParam->http_port < MIN_PORT) || (pMiscParam->http_port > MAX_PORT)) ||         
        ((pMiscParam->rtsp_port < MIN_PORT) || (pMiscParam->rtsp_port > MAX_PORT)) ||   
        ((pMiscParam->onvif_port < MIN_PORT) || (pMiscParam->onvif_port > MAX_PORT)) ||           
        ((pMiscParam->p2p_enable != ON) && (pMiscParam->p2p_enable != OFF)) ||                 
        ((pMiscParam->upnp_enable != ON) && (pMiscParam->upnp_enable != OFF)))  
        return SDK_ERROR;           
    
    return SDK_OK;
}

